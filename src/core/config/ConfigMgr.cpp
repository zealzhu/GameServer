#include "ConfigMgr.h"

ConfigMgr * ConfigMgr::Instance()
{
	static ConfigMgr instance;
	return &instance;
}

bool ConfigMgr::Initialize(IContext * pContext)
{
	auto config = pContext->LoadConfig("config.ini");

	// log config
	core_config_.log_name = config->GetString("log", "name", "MyLogger");
	core_config_.file_name = config->GetString("log", "file_name", "test");
	core_config_.size_mb = config->GetInt64("log", "size_mb", 10);
	core_config_.number_files = config->GetInt64("log", "number_files", 10);
	core_config_.enable_console = config->GetBool("log", "enable_console", true);
	core_config_.async = config->GetBool("log", "async", false);
	core_config_.queue_size = config->GetInt64("log", "queue_size", 1024);
	core_config_.level = config->GetString("log", "level", "trace");

	// net config
	core_config_.port = config->GetInt64("net", "port", 9999);

	// module config
	std::string str = config->GetString("module", "name", "");
	auto & modules_name = core_config_.modules_name;
	size_t last_len = 0, current_len = 0;
	while (true) {
		current_len = str.find(';', last_len);
		if (current_len == std::string::npos)
			break;
		std::string name = str.substr(last_len, current_len - last_len);
		last_len = current_len + 1;
		if(modules_name.find(name) == modules_name.end())
			modules_name.insert(name);
	}

	// db config
	core_config_.ip = config->GetString("db", "ip", "127.0.0.1");
	core_config_.user = config->GetString("db", "user", "zhu");
	core_config_.password = config->GetString("db", "password", "no080740");
	core_config_.scheme = config->GetString("db", "scheme", "qmsg");
	core_config_.db_port = config->GetInt64("db", "port", 3306); 
	core_config_.max_pool_size = config->GetInt64("db", "max_pool_size", 15);
	core_config_.db_check_interval = config->GetInt64("db", "check_interval", 10);

	return true;
}
