#ifndef _IMESSAGE_CENTER_HPP
#define _IMESSAGE_CENTER_HPP
#include <protobuf.hpp>
#include <eventbuf.hpp>
#include <isender.hpp>
#include <itimer.hpp>

typedef std::function< void (proto::EventBuff &) > EventCB;

class ITimerProc
{
public:
    virtual void InsertTimer(ITimer * timer, int64_t delay, int32_t count, int64_t interval) = 0;

    virtual ~ITimerProc() {}
};

class IMessageCenter
{
public:
    virtual void InsertProtobufMessage(const std::string & module_name, ISender * sender, proto::Protobuf * buff) = 0;
    virtual void InsertEventMessage(const std::string & module_name, EventCB & cb, proto::EventBuff * buff) = 0;
    virtual ITimerProc * FindTimerProc() = 0;
};

#endif
