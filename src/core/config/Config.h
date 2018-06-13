#ifndef _CONFIG_H
#define _CONFIG_H

#include "IContext.h"
#include "INIReader.h"

class Config : public IConfig
{
public:
	Config(const char * path);

	virtual bool GetBool(const char * sec, const char * key, bool def = false);
	virtual int64_t GetInt64(const char * sec, const char * key, int64_t def = 0);
	virtual double GetDouble(const char * sec, const char * key, double def = 0);
	virtual std::string GetString(const char * sec, const char * key, std::string def = "");

private:
	INIReader reader_;
};

#endif // _CONFIG_MANAGER_H
