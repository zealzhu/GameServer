#ifndef _CONFIG_MGR_H
#define _CONFIG_MGR_H

#include "IContext.h"
#include "Config.h"
#include <unordered_map>

struct CoreConfig
{
	// log
	std::string log_name;
	std::string file_name;
	int64_t size_mb;
	int64_t number_files;
	bool enable_console;
	bool async;
	int64_t queue_size;
	std::string level;

	// net
	uint16_t port;

	// module
	std::set<std::string> modules_name;

	// db
	int32_t max_pool_size;
	int32_t db_check_interval;
	int16_t db_port;
	std::string user;
	std::string password;
	std::string	ip;
	std::string scheme;
};

class ConfigMgr
{
public:
	static ConfigMgr * Instance();

	bool Initialize(IContext * pContext);

	inline const CoreConfig & GetCoreConfig() { return core_config_; }

private:
	CoreConfig core_config_;
};

#endif