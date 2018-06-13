#ifndef _IDB_H
#define _IDB_H

#include <map>
#include <list>
#include <vector>
#include <memory>
#include <string>

#define MAKE_INT_VALUE(v) std::make_pair<DBType, std::string>(DBType::kDBInt, std::to_string(v))
#define MAKE_FLOAT_VALUE(v) std::make_pair<DBType, std::string>(DBType::kDBFloat, std::to_string(v))
#define MAKE_STRING_VALUE(v) std::make_pair<DBType, std::string>(DBType::kDBStr, v.c_str())

// 数据类型
enum DBType
{
	kDBInt,
	kDBStr,
	kDBFloat,
};

typedef std::map< std::string, std::pair< DBType, std::string > > RecordData;
typedef std::map<std::string, std::string> RowFields;
typedef std::vector<RowFields> QueryData;

class IDBConnection
{
public:
	virtual bool Connect(const char * host, const char * user, const char * password, const char * db, int16_t port) = 0;
	virtual bool ExecQuery(const std::string & sql, QueryData & query_data) = 0;
	virtual int32_t ExecNonQuery(const std::string & sql) = 0;
	virtual std::string EscapeString(const std::string & param) = 0;
};

typedef std::shared_ptr<IDBConnection> DBConnectionPtr;

class IDBConnectionPool
{
public:
	virtual DBConnectionPtr GetConnection() = 0;

	virtual void ReturnConnection(DBConnectionPtr & conn) = 0;
};

class IDBHelper
{
public:
	virtual std::string BuildInsertSQL(DBConnectionPtr conn, const std::string & table_name, const RecordData & column_map) = 0;

	virtual size_t InsertRecord(const std::string & table_name, const RecordData & column_map) = 0;

	virtual std::string BuildUpdateSQL(const std::string & table_name, const RecordData & column_map, const RecordData & where_map) = 0;

	virtual size_t UpdateRecord(const std::string & table_name, const RecordData & column_map, const RecordData & where_map) = 0;

	virtual size_t DeleteRecord(const std::string & table_name, const std::string & where) = 0;

	virtual std::string BuildQuerySQL(const std::string & table, const std::string & field, const RecordData & where_map) = 0;

	virtual QueryData QueryRecord(const std::string & table, const std::string & field, const RecordData & where_map) = 0;

	virtual QueryData QueryRecord(const std::string & query_sql) = 0;
}; // DBHelp

#endif // _IDB_H
