/**
 * @file MoudleManager.h
 * @brief 模块管理器
 * @author zhu peng cheng
 * @version 1.0
 * @date 2018-01-10
 */

#ifndef _MODULE_MANAGER_H
#define _MODULE_MANAGER_H

#include "imodule.hpp"
#include <game_log.h>
#include <unordered_map>
#include <user/user_mgr.h>
#include <room/room_mgr.h>

class ModuleMgr
{
    typedef std::shared_ptr< IModule > ModulePtr;
    typedef std::unordered_map< std::string, ModulePtr > ModuleMap;
    typedef ModuleMap::iterator ModuleIter;
public:
    bool Initialize()
    {
        std::shared_ptr< UserMgr > user_mgr(new UserMgr());
        std::shared_ptr< RoomMgr > room_mgr(new RoomMgr());
        this->AddModule(room_mgr);
        this->AddModule(user_mgr);

        for(auto it : this->module_map_)
        {
            if(!it.second->Initialize())
            {
                logger_error("module {} initialize failed.", it.second->GetModuleName());
                return false;
            }
            else
            {
                logger_info("module {} initlialized.", it.second->GetModuleName());
            }
        }

        return true;
    }

    void Start()
    {
        for(auto it : this->module_map_)
        {
            it.second->Start();
        }
    }

    void Stop()
    {
        for(auto it : this->module_map_)
        {
            it.second->Stop();
        }
        module_map_.clear();
    }

    ModuleMap & GetAllModule()
    {
        return module_map_;
    }

    /**
     * @brief 获取实例
     *
     * @return
     */
    static ModuleMgr & Instance()
    {
        static ModuleMgr instance;
        return instance;
    }
private:
    bool AddModule(ModulePtr module)
    {
        auto pair = this->module_map_.insert(std::make_pair< std::string, ModulePtr >(module->GetModuleName(), std::move(module)));

        return pair.second;
    }

    void RemoveModule(ModuleIter iter)
    {
        this->module_map_.erase(iter);
    }

    void RemoveAllModule()
    {
        ModuleIter it;
        while(!this->module_map_.empty())
        {
            it = this->module_map_.begin();
            RemoveModule(it);
        }
    }

    IModule * Find(const std::string & module_name)
    {
        auto find = this->module_map_.find(module_name);
        if(find == this->module_map_.end())
        {
            return nullptr;
        }

        return find->second.get();
    }

    ModuleMgr()
    {}
    ~ModuleMgr() = default;
    ModuleMgr(const ModuleMgr &) = delete;
    ModuleMgr & operator=(const ModuleMgr &) = delete;

    ModuleMap module_map_;
}; // end ModuleMgr

#define GModules ModuleMgr::Instance()
#endif // _MODULE_MANAGER_H

