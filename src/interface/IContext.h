#ifndef _ICONTEXT_H
#define _ICONTEXT_H

#include <memory>
#include <ByteBuffer.hpp>
#include <Tools.hpp>
#include <ITimer.h>
#include <IDB.h>

#define TraceLog(fmt, ...) { \
	char logBuff[8192] = { 0 }; \
	sprintf(logBuff, fmt, ## __VA_ARGS__); \
	pContext->Log("trace", logBuff); \
}

#define DebugLog(fmt, ...) { \
	char logBuff[8192] = { 0 }; \
	sprintf(logBuff, fmt, ## __VA_ARGS__); \
	pContext->Log("debug", logBuff); \
}

#define InfoLog(fmt, ...) { \
	char logBuff[8192] = { 0 }; \
	sprintf(logBuff, fmt, ## __VA_ARGS__); \
	pContext->Log("info", logBuff); \
}

#define WarnLog(fmt, ...) { \
	char logBuff[8192] = { 0 }; \
	sprintf(logBuff, fmt, ## __VA_ARGS__); \
	pContext->Log("warn", logBuff); \
}

#define ErrorLog(fmt, ...) { \
	char logBuff[8192] = { 0 }; \
	sprintf(logBuff, " [%s] " fmt, __FILE__, ## __VA_ARGS__); \
	pContext->Log("error", logBuff); \
}

class IConfig;
class IContext;
class IModule;
class IMsgMgr;
class ITimer;

#define RegMessageHandler(type, handler) pContext->GetMsgMgr()->RegistMessageHandler(type, std::bind(handler, this, std::placeholders::_1, std::placeholders::_2))
#define RegEventHandler(type, handler) pContext->GetMsgMgr()->RegistEventHandler(type, handler)

#define RegTimer(context, timer, d, c, i) context->StartTimer(timer, d, c, i)
#define UnRegTimer(context, timer) context->StopTimer(timer)

typedef std::shared_ptr<IConfig> ConfigPtr;
typedef IModule * ModulePtr;

class IConfig
{
public:
	virtual bool GetBool(const char * sec, const char * key, bool def = false) = 0;
	virtual int64_t GetInt64(const char * sec, const char * key, int64_t def = 0) = 0;
	virtual double GetDouble(const char * sec, const char * key, double def = 0) = 0;
	virtual std::string GetString(const char * sec, const char * key, std::string def = "") = 0;
};

class IContext
{
public:
	virtual bool Initialize() = 0;
	virtual void Run() = 0;
	virtual void Destory() = 0;

	virtual void SetLogLevel(const char * level) = 0;
	virtual void Log(const char * level, const char * log) = 0;

	virtual ConfigPtr LoadConfig(const char * path) = 0;
	virtual ModulePtr FindModule(const char * module_name) = 0;
	virtual IMsgMgr * GetMsgMgr() = 0;
	virtual IDBHelper * GetDBHelper() = 0;

	virtual void StartTimer(ITimer * timer, uint64_t delay, int32_t count, uint64_t interval) = 0;
	virtual void StopTimer(ITimer * timer) = 0;

	virtual void Stop() = 0;
};

#endif // _ICONTEXT_H
