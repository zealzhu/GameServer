/**
 * @file DBHelp.cpp
 * @brief 数据库帮助类
 * @author zhu peng cheng
 * @version 1.0
 * @date 2018-01-12
 */
#include "DBHelper.h"
#include <db/DBConnectionPool.h>
#include <sstream>

std::string DBHelper::BuildInsertSQL(DBConnectionPtr conn, const std::string & table_name, const RecordData & column_map)
{
    using namespace std;
    ostringstream columns_name; // 列名
    ostringstream columns_values; // 列名对应的数据
    auto it_end = column_map.end();

    for(auto it = column_map.begin(); it != it_end; ++it)
    {
        // 第一条数据前面不要逗号
        if(it == column_map.begin())
        {
            // 列名
            columns_name << it->first;
            // 列名对应值
            if(it->second.first == DBType::kDBStr)
            {
                columns_values << "'" << conn->EscapeString(it->second.second) << "'";
            }
            else
            {
                columns_values << it->second.second;
            }
        }
        // 后面的都要加个逗号
        else
        {
            // 列名
            columns_name << "," << it->first;
            // 列名对应值
            if(it->second.first == DBType::kDBStr)
            {
                columns_values << ",'" << conn->EscapeString(it->second.second) << "'";
            }
            else
            {
                columns_values << "," << it->second.second;
            }
        }
    }

    ostringstream insert_sql_stream;
    insert_sql_stream << "insert into " << table_name << " (" << columns_name.str() << ") values(" << columns_values.str() << ")";

    return insert_sql_stream.str();
}

size_t DBHelper::InsertRecord(const std::string & table_name, const RecordData & column_map)
{
    // 从连接池中获取一个连接
    auto conn = DBConnectionPool::Instance()->GetConnection();
	std::string insert_sql = this->BuildInsertSQL(conn, table_name, column_map);
    // 执行插入
	int32_t affected_rows = conn->ExecNonQuery(insert_sql);
    // 返回给数据库连接池
    DBConnectionPool::Instance()->ReturnConnection(conn);

    return affected_rows;
}

std::string DBHelper::BuildUpdateSQL(const std::string & table_name, const RecordData & column_map, const RecordData & where_map)
{
    using namespace std;
    ostringstream set_values;

    // set
    for(auto it = column_map.begin(); it != column_map.end(); ++it)
    {
        // 第一条数据前面不要逗号
        if(it == column_map.begin())
        {
            // 列名
            set_values << it->first << "=";
            // 列名对应值
            if(it->second.first == DBType::kDBStr)
            {
                set_values << "'" << it->second.second << "'";
            }
            else
            {
                set_values << it->second.second;
            }
        }
        // 后面的都要加个逗号
        else
        {
            // 列名
            set_values << "," << it->first << "=";
            // 列名对应值
            if(it->second.first == DBType::kDBStr)
            {
                set_values << "'" << it->second.second << "'";
            }
            else
            {
                set_values << it->second.second;
            }
        }
    }

    ostringstream update_sql_stream;
    update_sql_stream << "update " << table_name << " set " << set_values.str() << " where " << BuildWhere(where_map);

    return update_sql_stream.str();
}

size_t DBHelper::UpdateRecord(const std::string & table_name, const RecordData & column_map, const RecordData & where_map)
{
    std::string update_sql = this->BuildUpdateSQL(table_name, column_map, where_map);

    // 从连接池中获取一个连接
    auto conn = DBConnectionPool::Instance()->GetConnection();
	// 执行更新
	int32_t affected_rows = conn->ExecNonQuery(update_sql);
    // 返回给数据库连接池
    DBConnectionPool::Instance()->ReturnConnection(conn);

    return affected_rows;
}

size_t DBHelper::DeleteRecord(const std::string & table_name, const std::string & where)
{
    std::ostringstream delete_sql_stream;
    delete_sql_stream << "delete from " << table_name << " where " << where;

    // 从连接池中获取一个连接
    auto conn = DBConnectionPool::Instance()->GetConnection();
	// 执行删除
	int32_t affected_rows = conn->ExecNonQuery(delete_sql_stream.str());
    // 返回给数据库连接池
    DBConnectionPool::Instance()->ReturnConnection(conn);

    return affected_rows;
}

std::string DBHelper::BuildQuerySQL(const std::string & table, const std::string & field, const RecordData & where_map)
{
    std::ostringstream sql;
    sql << "select " << field << " from " << table << " where " << BuildWhere(where_map);

    return sql.str();
}

QueryData DBHelper::QueryRecord(const std::string & table, const std::string & fields, const RecordData & where_map)
{
    std::string query_sql = BuildQuerySQL(table, fields, where_map);
    return QueryRecord(query_sql);
}

QueryData DBHelper::QueryRecord(const std::string & query_sql)
{
    //logger_debug("SQL 查询语句：{}", query_sql);
    QueryData query_data;

    // 从连接池中获取一个连接
    auto conn = DBConnectionPool::Instance()->GetConnection();
	bool ret = conn->ExecQuery(query_sql, query_data);
    // 返回给数据库连接池
    DBConnectionPool::Instance()->ReturnConnection(conn);

    return query_data;

}

std::string DBHelper::BuildWhere(const RecordData & where_map)
{
    std::ostringstream where;

    auto it_end = where_map.end();
    for(auto it = where_map.begin(); it != it_end; ++it)
    {
        // 第一条数据前面不要逗号
        if(it == where_map.begin())
        {
            // 列名
            where << it->first << "=";
            // 列名对应值
            if(it->second.first == DBType::kDBStr)
            {
                where << "'" << it->second.second << "'";
            }
            else
            {
                where << it->second.second;
            }
        }
        // 后面的都要加个and
        else
        {
            // 列名
            where << " and " << it->first << "=";
            // 列名对应值
            if(it->second.first == DBType::kDBStr)
            {
                where << "'" << it->second.second << "'";
            }
            else
            {
                where << it->second.second;
            }
        }
    }
    return where.str();
}
