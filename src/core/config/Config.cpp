#include "Config.h"
#include <assert.h>

Config::Config(const char * path) : reader_(path)
{
	if (reader_.ParseError() < 0)
	{
		printf("load path %s failed\n", path);
		//assert(false);
	}
}

bool Config::GetBool(const char * sec, const char * key, bool def)
{
	return reader_.GetBoolean(sec, key, def);
}

int64_t Config::GetInt64(const char * sec, const char * key, int64_t def)
{
	return reader_.GetInteger(sec, key, def);
}

double Config::GetDouble(const char * sec, const char * key, double def)
{
	return reader_.GetReal(sec, key, def);
}

std::string Config::GetString(const char * sec, const char * key, std::string def)
{
	return reader_.Get(sec, key, def);
}