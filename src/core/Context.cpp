#include "Context.h"
#include "log/Logger.h"
#include "module/ModuleMgr.h"
#include "net/TcpServer.h"
#include "msg/MsgMgr.h"
#include "config/ConfigMgr.h"
#include "db/DBConnectionPool.h"

bool Context::Initialize()
{
	stop_ = false;
	if (!ConfigMgr::Instance()->Initialize(this) ||
		!Logger::Instace()->Initialize(this) || 
		!TcpServer::Instance()->Initialize(this) ||
		!DBConnectionPool::Instance()->Initialize(this) ||
		!ModuleMgr::Instance()->Initialize(this)) {
		return false;
	}
	return true;
}

void Context::Run()
{
	DBConnectionPool::Instance()->Start();
	TcpServer::Instance()->Start();
	SessionMessage msg;
	
	while (!stop_) {
		bool ret = false;
		ret = MsgMgr::Instance()->GetProtoMessage(msg);
		if (ret) {
			ByteBuffer buffer(msg.msg->message_data().c_str(), msg.msg->message_data().size());
			MsgMgr::Instance()->TransforMessage(msg.sid, msg.msg->type(), buffer);
		}
		ret = timer_queue_.Tick(this);

		if (!ret){
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	}
}

void Context::Destory()
{
	ModuleMgr::Instance()->Destory(this);
	TcpServer::Instance()->Destory();
	DBConnectionPool::Instance()->Stop();
}

void Context::SetLogLevel(const char * level)
{
	Logger::Instace()->SetLevel(level);
}

void Context::Log(const char * level, const char * log)
{
	auto logger = Logger::Instace()->GetLogger();
	if (0 == strcmp(level, "trace")) {
		logger->trace(log);
	}
	else if (0 == strcmp(level, "debug")) {
		logger->debug(log);
	}
	else if (0 == strcmp(level, "info")) {
		logger->info(log);
	}
	else if (0 == strcmp(level, "warn")) {
		logger->warn(log);
	}
	else if (0 == strcmp(level, "error")) {
		logger->error(log);
	}
}

ConfigPtr Context::LoadConfig(const char * path)
{
	return ConfigPtr(new Config(path));
}

ModulePtr Context::FindModule(const char * module_name)
{
	return ModulePtr();
}

IMsgMgr * Context::GetMsgMgr()
{
	return MsgMgr::Instance();
}

void Context::StartTimer(ITimer * timer, uint64_t delay, int32_t count, uint64_t interval)
{
	timer_queue_.AddTimer(timer, delay, count, interval);
}

void Context::StopTimer(ITimer * timer)
{
	timer_queue_.StopTimer(timer);
}
