/**
 * @file GameLog.cpp
 * @brief 游戏日志类
 * @author zhu peng cheng
 * @version 1.0
 * @date 2018-01-03
 */
#include "game_log.h"
#include "config_manager.h"
#include "file_utils.hpp"

namespace GameTools
{

std::unique_ptr<GameLog> logger;

GameLog::GameLog() : logger_(nullptr)
{

}

GameLog::~GameLog()
{
    std::cout << "clean spdlog" << std::endl;
    std::string async = GConfig.GetConfigParam("log.async", "false");
    // 如果是异步的立即刷新缓冲区
    if(async == "true")
    {
        this->logger_->flush();
    }
}

bool GameLog::Initialize()
{
    // 检测日志目录
    if(!FileUtils::IsDirExist("log"))
    {
        FileUtils::CreateDir("log");
    }

    std::string file_name = GConfig.GetConfigParam("log.filename", "server.log");
    std::string size_mb = GConfig.GetConfigParam("log.sizemb", "1");
    std::string number_files = GConfig.GetConfigParam("log.numberfiles", "10");

    // 检测是否已经有日志实例说明已经初始化过了
    if(this->logger_)
        return true;

    try
    {
        std::vector<spdlog::sink_ptr> sinks;
        // 输出到文件
        sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>
                ("log/" + file_name, atoi(size_mb.c_str()) * 1024L * 1024, atoi(number_files.c_str())));
        // 输出到控制台
        if(GConfig.GetConfigParam("log.enableconsole", "true") == "true")
        {
            // 直接控制台没有颜色
            // auto stdout_sink = spdlog::sinks::stdout_sink_mt::instance();
            auto color_sink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
            sinks.push_back(color_sink);
        }

        // 创建spdlog
        auto combined_logger = std::make_shared<spdlog::logger>("server", begin(sinks), end(sinks));

        // 异步
        if(GConfig.GetConfigParam("log.async", "false") == "true")
        {
            std::string size = GConfig.GetConfigParam("log.queue_size", "4096");
            spdlog::set_async_mode(atoi(size.c_str()));
        }

        // [时间(到ms)] [线程号] [日志等级] 输出内容
        spdlog::set_pattern("[%Y-%m-%d.%e %H:%M:%S] [%t] [%l] %v");

        spdlog::register_logger(combined_logger);

        this->logger_ = combined_logger;

        // 读取设置日志等级
        SetLevel(GConfig.GetConfigParam("log.level", "trace"));

        return true;
    }
    catch(const spdlog::spdlog_ex & ex)
    {
        std::cout << "Log initialization failed: " << ex.what() << std::endl;
        return false;
    }
}

void GameLog::SetLevel(const std::string & level)
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

std::shared_ptr<spdlog::logger> GameLog::GetLogger()
{
    if(!this->logger_)
    {
        std::cout << "Not initialization spdlog" << std::endl;
        assert(0);
    }
    return this->logger_;
}

}
