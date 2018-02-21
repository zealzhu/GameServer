/**
 * @file ModuleInterface.h
 * @brief 模块接口
 * @author zhu peng cheng
 * @version 1.0
 * @date 2018-01-10
 */
#ifndef _MODULE_INTERFACE_H
#define _MODULE_INTERFACE_H
#include <string>
#include <isender.hpp>
#include <eventbuf.hpp>
#include <protobuf_define.hpp>

using namespace base;

class IModule
{
public:
    virtual bool Initialize() = 0;
    virtual bool Start() = 0;

    virtual void Stop() = 0;

    virtual std::string GetModuleName() = 0;

    virtual ~IModule() {}
}; // class IModule

#endif // _MODULE_INTERFACE_H
