/**
 * @file ConfigManager.h
 * @brief 配置管理器
 * @author zhu peng cheng
 * @version 1.0
 * @date 2018-01-04
 */
#ifndef _CONFIG_MANAGER_H
#define _CONFIG_MANAGER_H

#include <map>

namespace GameTools
{

class ConfigManager
{
public:
    static ConfigManager & Instance();

    bool Initialize();

    std::string GetConfigParam(const std::string & key, const std::string & default_value);
private:
    ConfigManager() = default;
    ~ConfigManager() = default;

    std::map<std::string, std::string> config_map_;
};

}

#define GConfig GameTools::ConfigManager::Instance()
#endif // _CONFIG_MANAGER_H
