/**
 * @file GameLog.h
 * @brief 游戏日志类
 * @author zhu peng cheng
 * @version 1.0
 * @date 2018-01-03
 */
#ifndef _GAME_LOG_H
#define _GAME_LOG_H

#include <spdlog/spdlog.h>

namespace GameTools
{

/**
 * @brief 游戏日志类
 */
class GameLog
{
public:
    static GameLog & Instance()
    {
        static GameLog instance;
        return instance;
    }

    bool Initialize();

    void SetLevel(const std::string & level);

    std::shared_ptr<spdlog::logger> GetLogger();

    ~GameLog();

private:
    GameLog();
    GameLog & operator=(GameLog const &) = delete;

    std::shared_ptr<spdlog::logger> logger_;
};

} // namespace GameTools

#define GLog GameTools::GameLog::Instance()
// {文件}::{函数}::{行}#{输出信息}
// ## 用来去除空参数时前面的逗号
#define logger_trace(fmt, ...) GLog.GetLogger()->trace("{}::{}()#{}: " fmt, __FILE__ , __FUNCTION__, __LINE__, ## __VA_ARGS__)
#define logger_debug(fmt, ...) GLog.GetLogger()->debug("{}::{}()#{}: " fmt, __FILE__ , __FUNCTION__, __LINE__, ## __VA_ARGS__)
#define logger_info(fmt, ...) GLog.GetLogger()->info("{}::{}()#{}: " fmt, __FILE__ , __FUNCTION__, __LINE__, ## __VA_ARGS__)
#define logger_warn(fmt, ...) GLog.GetLogger()->warn("{}::{}()#{}: " fmt, __FILE__ , __FUNCTION__, __LINE__, ## __VA_ARGS__)
#define logger_error(fmt, ...) GLog.GetLogger()->error("{}::{}()#{}: " fmt, __FILE__ , __FUNCTION__, __LINE__, ## __VA_ARGS__)
#define logger_critical(fmt, ...) GLog.GetLogger()->critical("{}::{}()#{}: " fmt, __FILE__ , __FUNCTION__, __LINE__, ## __VA_ARGS__)
#endif

