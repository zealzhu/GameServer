#include "DBConnectionPool.h"
#include "config/ConfigMgr.h"

#include <Tools.hpp>
#include <sstream>

using namespace std;

IContext * DBConnectionPool::pContext = nullptr;
IContext * DBConnection::pContext = nullptr;

DBConnection::DBConnection()
	: conn_(NULL)
{
	conn_ = mysql_init(NULL);
	if (conn_) {
		mysql_set_character_set(conn_, "utf8");
	}
}

DBConnection::~DBConnection()
{
	if (conn_ != NULL) {
		mysql_close(conn_);
		conn_ = NULL;
	}
}

bool DBConnection::Connect(const char * host, const char * user, const char * password, const char * db, int16_t port)
{
	if (conn_) {
		mysql_real_connect(conn_, host, user, password, db, port, NULL, 0);
		auto err = GetErrorNo();
		if(err == 0)
			return true;
	}
	LogError();
	return false;
}

bool DBConnection::ExecQuery(const std::string & sql, QueryData & query_data)
{
	auto ret = mysql_real_query(conn_, sql.c_str(), sql.size());
	if (ret == 0) {
		auto res = mysql_store_result(conn_);
		if (!res) {
			LogError();
			return false;
		}

		std::set<std::string> all_field_name;
		MYSQL_FIELD * field;
		while ((field = mysql_fetch_field(res)))//循环遍历所有字段
		{
			all_field_name.insert(field->name);
		}
	
		MYSQL_ROW row;
		query_data.clear();

		while ((row = mysql_fetch_row(res)))
		{
			unsigned long * lengths;
			lengths = mysql_fetch_lengths(res);
			RowFields row_fields;
			unsigned int i = 0;
			for(auto it = all_field_name.begin(); it != all_field_name.end(); it++)
			{
				row_fields[*it] = row[i] ? row[i] : "NULL";
				i++;
			}
			query_data.emplace_back(std::move(row_fields));
		}
		return true;
	}
	else {
		LogError();
		return false;
	}
}

int32_t DBConnection::ExecNonQuery(const std::string & sql)
{
	int state = mysql_query(conn_, sql.c_str());
	if (state != 0)
	{
		LogError();
		return 0;
	}
	return mysql_affected_rows(conn_);
}

std::string DBConnection::EscapeString(const std::string & param)
{
	char buff[8192] = { 0 };
	mysql_real_escape_string(conn_, buff, param.c_str(), param.length());
	return buff;
}

void DBConnection::LogError()
{
	const char * err = mysql_error(conn_);
	ErrorLog("query error: %s", err);;
}

DBConnectionPool::DBConnectionPool()
    : user_name_(""), password_(""), ip_(""), port_(0), db_(""),
    max_pool_size_(0), current_pool_size_(0), thread_finish_(false)
{}

DBConnectionPool::DBConnectionPool(const DBConnectionPool&)
{}

bool DBConnectionPool::Initialize(IContext * pContext)
{
	DBConnectionPool::pContext = pContext;
	DBConnection::pContext = pContext;
	auto &config = ConfigMgr::Instance()->GetCoreConfig();

    max_pool_size_ = config.max_pool_size;
    user_name_ = config.user;
    password_ = config.password;
	ip_ = config.ip;
	port_ = config.db_port;
	db_ = config.scheme;
	db_check_interval_ = config.db_check_interval;

	InfoLog("Init db. User: %s Password: %s Address %s", user_name_.c_str(), password_.c_str(), ip_.c_str());

    // 初始化数据库连接池，只初始化一半的大小
    this->current_pool_size_ = max_pool_size_ / 2;
    InitConnectionPool(max_pool_size_ / 2);

    return true;
}

void DBConnectionPool::Start()
{
    // 开启检查线程
    this->check_thread_ = std::thread(&DBConnectionPool::ShrinkConnectNum, this);
}

void DBConnectionPool::Stop()
{
    // 等待线程退出
    this->thread_finish_ = true;
    this->check_thread_.join();
    // 释放所有连接
    this->DestoryConnectionPool();
}

DBConnectionPool * DBConnectionPool::Instance()
{
    static DBConnectionPool instance;
    return &instance;
}

DBConnectionPool::~DBConnectionPool()
{
}

void DBConnectionPool::InitConnectionPool(int size)
{
    std::lock_guard<std::mutex> lck(lock_);
    this->AddConnections(size);
}

void DBConnectionPool::AddConnections(int size)
{
    //logger_info("准备新增 {} 数据库连接", size);
    int success = 0;

    for(int i = 0; i < size; i++)
    {
        try
        {
			DBConnectionPtr conn(new DBConnection);
			bool ret = conn->Connect(ip_.c_str(),
				user_name_.c_str(),
				password_.c_str(),
				db_.c_str(),
				port_);
			if (ret) {
				this->connection_list_.emplace_back(std::move(conn));
				++success;
			}
        }
        catch(...)
        {
			ErrorLog("Get db connection failed");
            return;
        }
    }
	current_pool_size_ = success;
    TraceLog("Get success db connection count: %d", success);
}

int DBConnectionPool::GetAvailableConnectionSize()
{
    return this->connection_list_.size();
}

void DBConnectionPool::ExpandPool(int size)
{
    std::lock_guard<std::mutex> lck(lock_);
    // 最大数量扩充
    this->max_pool_size_ += size;
    // 当前数量增加
    this->current_pool_size_ += size;
    // 添加连接
    this->AddConnections(size);
    TraceLog("Add %d db connection，available: %d，current：%d", size, this->GetAvailableConnectionSize(), this->current_pool_size_);
}

void DBConnectionPool::ReducePool(int size)
{
    std::lock_guard<std::mutex> lck(lock_);

    // 要减少的数量不能大于最大数量
    if(size > this->max_pool_size_)
    {
        return;
    }

    for(int i = 0; i < size; i++)
    {
        this->DestoryOneConnection();
    }
    this->max_pool_size_ -= size;
    this->current_pool_size_ -= size;
    //logger_info("减少{}个数据库连接，可用连接数{}，当前连接池大小：{}", size, this->GetAvailableConnectionSize(), this->current_pool_size_);
}

void DBConnectionPool::DestoryConnectionPool()
{
    InfoLog("Close all db connection");
    for(auto & conn : this->connection_list_)
    {
        // 依次转移所有权，智能指针t出作用域自动释放
        DBConnectionPtr && t = std::move(conn);
    }
	this->connection_list_.clear();
}

void DBConnectionPool::DestoryOneConnection()
{
    if(this->connection_list_.size() > 0)
    {
        this->connection_list_.pop_front();
        //logger_info("关闭一个数据库连接，可用连接数：{}，当前连接池大小：{}", this->GetAvailableConnectionSize(), this->current_pool_size_);
    }
    else
    {
        WarnLog("Can't close db connection. No one connection.");
    }
}

DBConnectionPtr DBConnectionPool::GetConnection()
{
    std::unique_lock<std::mutex> lck(lock_);

    // 如果没有可用连接就新增一个
    if(this->GetAvailableConnectionSize() == 0)
    {
        // 当前连接数不大于最大连接就添加一个连接
        if(this->current_pool_size_ < this->max_pool_size_)
        {
            AddConnections(1);
            ++this->current_pool_size_;
        }
        // 等待可用连接
        else
        {
            // 重试次数
            int retry_count = 0;

            // 重试次数小于10次且没有可用连接就一直重试
            while(this->GetAvailableConnectionSize() == 0 && retry_count < 10)
            {
                ++retry_count;
                WarnLog("Wait available connection.");
                // 释放锁
                lck.unlock();
                // 等待
                std::this_thread::sleep_for(std::chrono::milliseconds(50 * retry_count));
                // 重新上锁
                lck.lock();
            }

            if(retry_count >= 10)
            {
                WarnLog("Wait db connection more then 10, stop wait.");
                //throw Exception(ErrorCode::kNoAvailiableConnection);
            }
        }
    }

    // 取第一个
    DBConnectionPtr conn = this->connection_list_.front();
    this->connection_list_.pop_front();
    //logger_info("获得一个数据库连接，可用连接数：{}，当前连接池大小：{}", this->GetAvailableConnectionSize(), this->current_pool_size_);

    return conn;
}

void DBConnectionPool::ReturnConnection(DBConnectionPtr & conn)
{
    std::lock_guard<std::mutex> lck(this->lock_);

    this->connection_list_.emplace_back(conn);
    //logger_info("归还一个数据库连接，可用连接数：{}，当前连接池大小：{}", this->GetAvailableConnectionSize(), this->current_pool_size_);
}

void DBConnectionPool::ShrinkConnectNum()
{
    using namespace std::chrono;
    DebugLog("Start db connection check thread");

    // 最后一次可用连接数
    int last_available = this->GetAvailableConnectionSize();
    int cur_available = last_available;

    // 检查时间
	int64_t pre_time = tools::GetSecTime();
    // 当前时间
	int64_t cur_time;
    // 时间差
	int64_t time_span;

    while(!thread_finish_)
    {
        cur_time = tools::GetSecTime();
        time_span = cur_time - pre_time;
        // 计算时间差是否大于时间间隔
        if(time_span >= db_check_interval_)
        {
            std::lock_guard<std::mutex> lck(this->lock_);
            cur_available = this->GetAvailableConnectionSize();
            //logger_trace("前一次可用连接：{} 当前可用连接：{}", last_available, cur_available);

            // 当可用连接大于连接池一半时且上一次与当前可用连接一样时减少一个连接
            if(cur_available > this->max_pool_size_ / 2 && last_available == cur_available)
            {
                this->DestoryOneConnection();
                --this->current_pool_size_;
            }
            last_available = this->GetAvailableConnectionSize();
            // 更新之前的时间
			pre_time = cur_time;
        }
        // 如果使用sleep等待的话主线程如果退出还要等到唤醒才结束
        std::this_thread::sleep_for(milliseconds(1000));
    }
    DebugLog("Stop db connection check thread");
}

