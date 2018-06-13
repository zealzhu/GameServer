/**
 * @file DBConnectionPool.h
 * @brief 数据库连接池
 * @author zhu peng cheng
 * @version 1.0
 * @date 2018-01-11
 */
#ifndef _DB_CONNECTION_POOL_H
#define _DB_CONNECTION_POOL_H

#include <IDB.h>
#include <mysql.h>

#include <mutex>
#include <thread>

class IContext;

class DBConnection : public IDBConnection
{
public:
	DBConnection();
	~DBConnection();

	virtual bool Connect(const char * host, const char * user, const char * password, const char * db, int16_t port);
	virtual bool ExecQuery(const std::string & sql, QueryData & query_data);
	virtual int32_t ExecNonQuery(const std::string & sql);
	virtual std::string EscapeString(const std::string & param);

	void LogError();
	uint32_t GetErrorNo() { return mysql_errno(conn_); }

	static IContext * pContext;
private:
	MYSQL * conn_;
};

class DBConnectionPool
{
    typedef std::list<DBConnectionPtr> DBConnectionList;
public:

    bool Initialize(IContext * pContext);
    void Start();
    void Stop();

    static DBConnectionPool * Instance();

	DBConnectionPtr GetConnection();

    void ReturnConnection(DBConnectionPtr & conn);

    int GetAvailableConnectionSize();

    ~DBConnectionPool();

private:

    DBConnectionPool();

    DBConnectionPool(const DBConnectionPool &);

    DBConnectionPool & operator=(const DBConnectionPool &) = default;

    void InitConnectionPool(int size);

    void DestoryConnectionPool();

    void ExpandPool(int size);

    void ReducePool(int size);

    void AddConnections(int size);

    void DestoryOneConnection();

    void ShrinkConnectNum();

    // 数据库连接列表
	DBConnectionList connection_list_;

    // 连接池最大数量
    int max_pool_size_;
    int current_pool_size_;

	int16_t port_;
	int32_t db_check_interval_;
	std::string ip_;
    std::string user_name_;
	std::string password_;
	std::string db_;

    std::mutex lock_;
    bool thread_finish_;
    std::thread check_thread_;

	static IContext * pContext;
};

#endif // _DB_CONNECTION_POOL_H
