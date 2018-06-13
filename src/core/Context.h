#ifndef _CONTEXT_H
#define _CONTEXT_H

#include "IContext.h"
#include <unordered_map>
#include <timer/TimerQueue.h>
#include "db/DBHelper.h"
#include <atomic>

class Context : public IContext
{
public:
	virtual bool Initialize();
	virtual void Run();
	virtual void Destory();
	
	virtual void SetLogLevel(const char * level);
	virtual void Log(const char * level, const char * log);

	virtual ConfigPtr LoadConfig(const char * path);
	virtual ModulePtr FindModule(const char * module_name);
	virtual IDBHelper * GetDBHelper() { return &db_helper_; }

	virtual IMsgMgr * GetMsgMgr();

	virtual void StartTimer(ITimer * timer, uint64_t delay, int32_t count, uint64_t interval);
	virtual void StopTimer(ITimer * timer);

	virtual inline void Stop() { stop_ = true; }

private:
	TimerQueue timer_queue_;
	DBHelper db_helper_;
	std::atomic<bool> stop_;
};


#endif