#include "Logger.h"
#include "config/ConfigMgr.h"

std::unique_ptr<Logger> logger;

std::shared_ptr<spdlog::logger> Logger::GetLogger()
{
	if (!this->logger_)
	{
		printf("Not initialization spdlog\n");
		assert(false);
	}
	return this->logger_;
}

Logger::~Logger()
{
    // 如果是异步的立即刷新缓冲区
    if(async_)
    {
        this->logger_->flush();
    }
}

bool Logger::Initialize(IContext * pContext)
{
	auto & config = ConfigMgr::Instance()->GetCoreConfig();

	// 检测日志目录
	if (!tools::IsDirExist("log"))
		tools::CreateDir("log");

	async_ = config.async;

    // 检测是否已经有日志实例说明已经初始化过了
    if(this->logger_)
        return true;

    try
    {
        std::vector<spdlog::sink_ptr> sinks;
        // 输出到文件
        sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>
                ("log/" + config.file_name, config.size_mb * 1024L * 1024L, config.number_files));
        // 输出到控制台
        if(config.enable_console)
        {
            // 直接控制台没有颜色
            // auto stdout_sink = spdlog::sinks::stdout_sink_mt::instance();
#ifndef WIN32
            auto color_sink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
#else
			auto color_sink = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
#endif
            sinks.push_back(color_sink);
        }

        // 创建spdlog
        auto combined_logger = std::make_shared<spdlog::logger>(config.log_name, begin(sinks), end(sinks));

        // 异步
        if(async_)
        {
            spdlog::set_async_mode(config.queue_size);
        }

        // [时间(到ms)] [线程号] [日志等级] 输出内容
        spdlog::set_pattern("[%Y-%m-%d.%e %H:%M:%S] [%t] [%l] %v");

        spdlog::register_logger(combined_logger);

        this->logger_ = combined_logger;

        // 读取设置日志等级
        SetLevel(config.level);

        return true;
    }
    catch(const spdlog::spdlog_ex & ex)
    {
		printf("Log initialization failed: %s\n", ex.what());
		assert(false);
        return false;
    }
}

void Logger::SetLevel(const std::string & level)
{
    if(!this->logger_)
        return;

    // 日志等级
    spdlog::level::level_enum spd_level = spdlog::level::level_enum::err;

    if(level == "trace")
    {
        spd_level = spdlog::level::level_enum::trace;
    }
    else if(level == "debug")
    {
        spd_level = spdlog::level::level_enum::debug;
    }
    else if(level == "info")
    {
        spd_level = spdlog::level::level_enum::info;
    }
    else if(level == "warn")
    {
        spd_level = spdlog::level::level_enum::warn;
    }
    else if(level == "error")
    {
        spd_level = spdlog::level::level_enum::err;
    }
    else if(level == "critical")
    {
        spd_level = spdlog::level::level_enum::critical;
    }
    else if(level == "off")
    {
        spd_level = spdlog::level::level_enum::off;
    }

    this->logger_->set_level(spd_level);

    this->logger_->flush_on(spd_level);
}