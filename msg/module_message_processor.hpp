#ifndef _MESSAGE_PROCESSOR_HPP
#define _MESSAGE_PROCESSOR_HPP

#include "imessage_center.hpp"
#include <isender.hpp>
#include <znet/spscqueue.hpp>
#include <protobuf_processor.hpp>
#include <eventbuf.hpp>
#include <timer_queue.h>

class ModuleProcessor : public ITimerProc
{
public:
    typedef std::pair< ISender *, proto::Protobuf *> ProtoPair;
    typedef std::pair< EventCB, proto::EventBuff *> EventPair;

    ModuleProcessor() : curr_thrd_id_(std::this_thread::get_id()) {  }
    ~ModuleProcessor() = default;

    bool Loop()
    {
        curr_thrd_id_ = std::this_thread::get_id();
        bool ret = false;
        // exec protobuf message
        ProtoPair proto_pair;
        ret = proto_queue_.try_dequeue(proto_pair);
        if(ret) {
            ProtobufProcessor::Instance().Dispatch(proto_pair.first, *proto_pair.second);
            if(proto_pair.second) {
                delete proto_pair.second;
            }
        }

        // exec event message
        EventPair event_pair;
        ret = event_queue_.try_dequeue(event_pair);
        if(ret) {
            event_pair.first(*event_pair.second);
            if(event_pair.second) {
                delete event_pair.second;
            }
        }

        // exec timer message
        timer_queue_.Tick();

        return ret;
    }

    void Stop() {
        ProtoPair proto_pair;
        bool ret = proto_queue_.try_dequeue(proto_pair);
        while(ret)
        {
            if(proto_pair.second) {
                delete proto_pair.second;
            }
            ret = proto_queue_.try_dequeue(proto_pair);
        }

        EventPair event_pair;
        ret = event_queue_.try_dequeue(event_pair);
        while(ret)
        {
            if(event_pair.second) {
                delete event_pair.second;
            }
            ret = event_queue_.try_dequeue(event_pair);
        }
        timer_queue_.KillAll();
    }

    inline void InsertProtobufMessage(ISender * sender, proto::Protobuf * buff) {
        proto_queue_.enqueue(std::make_pair< ISender *, proto::Protobuf *>(std::move(sender), std::move(buff)));
    }

    inline void InsertEventMessage(EventCB cb, proto::EventBuff * buff) {
        event_queue_.enqueue(std::make_pair< EventCB, proto::EventBuff *>(std::move(cb), std::move(buff)));
    }

    virtual void InsertTimer(ITimer * timer, int64_t delay, int32_t count, int64_t interval) {
        timer_queue_.AddTimer(timer, delay, count, interval);
    }

    virtual void StopTimer(ITimer * timer) {
        timer_queue_.StopTimer(timer);
    }

    inline std::thread::id GetCurrentLoopThreadId() { return curr_thrd_id_; }

private:
    typedef spscqueue< ProtoPair > ProtoQueue;
    typedef spscqueue< EventPair > EventQueue;

    ProtoQueue proto_queue_;
    EventQueue event_queue_;
    TimerQueue timer_queue_;
    std::thread::id curr_thrd_id_;
};

#endif
