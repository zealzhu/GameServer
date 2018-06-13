#ifndef _LOGGER_H
#define _LOGGER_H

#include <spdlog.h>
#include <IContext.h>

class Logger
{
public:
	static Logger * Instace() {
		static Logger instance;
		return &instance;
	}

    bool Initialize(IContext * context);

    void SetLevel(const std::string & level);

    std::shared_ptr<spdlog::logger> GetLogger();

    ~Logger();

private:
    std::shared_ptr<spdlog::logger> logger_;
	bool async_;
};

#endif

