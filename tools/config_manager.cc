/**
 * @file ConfigManager.cpp
 * @brief
 * @author zhu peng cheng
 * @version 1.0
 * @date 2018-01-04
 */
#include "config_manager.h"
#include "file_utils.hpp"

namespace GameTools
{

ConfigManager & ConfigManager::Instance()
{
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::Initialize()
{
    this->config_map_ = FileUtils::GetConfigMap("config/config.ini");
    return true;
}

std::string ConfigManager::GetConfigParam(const std::string & key, const std::string & default_value)
{
    auto it = ConfigManager::config_map_.find(key);

    if(it != ConfigManager::config_map_.end()) {
        return it->second;
    }
    else {
        return default_value;
    }
}

}
