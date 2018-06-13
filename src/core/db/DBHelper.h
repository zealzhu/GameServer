/**
 * @file DBHelp.h
 * @brief 数据库帮助类
 * @author zhu peng cheng
 * @version 1.0
 * @date 2018-01-12
 */
#ifndef _DB_HELP_H
#define _DB_HELP_H

#include <IDB.h>

class DBHelper : public IDBHelper
{
public:
    virtual std::string BuildInsertSQL(DBConnectionPtr conn, const std::string & table_name, const RecordData & column_map);

	virtual size_t InsertRecord(const std::string & table_name, const RecordData & column_map);

	virtual std::string BuildUpdateSQL(const std::string & table_name, const RecordData & column_map, const RecordData & where_map);

	virtual size_t UpdateRecord(const std::string & table_name, const RecordData & column_map, const RecordData & where_map);

	virtual size_t DeleteRecord(const std::string & table_name, const std::string & where);

	virtual std::string BuildQuerySQL(const std::string & table, const std::string & field, const RecordData & where_map);

	virtual QueryData QueryRecord(const std::string & table, const std::string & field, const RecordData & where_map);

	virtual QueryData QueryRecord(const std::string & query_sql);

	DBHelper() = default;
    ~DBHelper() = default;
private:
    std::string BuildWhere(const RecordData & where_map);
}; // DBHelp

#endif // _DB_HELP_H
