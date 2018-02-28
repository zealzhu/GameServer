#ifndef _EVENT_CENTER_HPP
#define _EVENT_CENTER_HPP

#include <unordered_map>
#include <game_log.h>
#include <imessage_center.hpp>

#define RegEvent(obj, eid, cb) EventProcessor::Instance().RegistEvent(obj, eid, cb)
#define ExecEvent(buff) EventProcessor::Instance().ExecuteEvent(buff);

enum EventId
{
    kEventConnectionClose,
    kEventUserOnline,
    kEventUserOffline,
    kEventUserLostConnect,

    kEventLeaveRoomAck,
    kEventLandlordAck,
    kEventReadyAck,

    kEventGameBegin,
    kEventPutCardNtf,
    kEventCallLandlordNtf,
    kEventPutLandlordCardNtf,
    kEventPlayError,
    kEventPlaySuccess,
    kEventGameRestart,
    kEventGameOver,
};

class EventProcessor
{
public:
    static EventProcessor & Instance() {
        static EventProcessor instance;
        return instance;
    }

    void SetMessageCenter(IMessageCenter * center) {
        this->msg_center_ = center;
    }

    template< typename Obj >
    void RegistEvent(Obj* obj, const int16_t event_id, void (Obj::*func)(proto::EventBuff &)) {
        auto find = events_map_.find(event_id);

        if(find == events_map_.end()) {
            events_map_[event_id] = std::vector< std::pair< std::string, EventCB > >();
        }
        events_map_[event_id].emplace_back(
                std::make_pair< std::string, EventCB >(
                    std::move(obj->GetModuleName()),
                    std::move(std::bind(func, obj, std::placeholders::_1))));
    }

    void ExecuteEvent(proto::EventBuff * buff) {
        auto find = events_map_.find(buff->GetId());
        if(find == events_map_.end()) {
            if(buff != nullptr) {
                delete buff;
            }
            logger_warn("not found event {} module", buff->GetId());
            return;
        }

        for(auto & event : find->second) {
            msg_center_->InsertEventMessage(event.first, event.second, buff);
        }
    }

private:
    EventProcessor() : msg_center_(nullptr) {}
    virtual ~EventProcessor() {}

    std::unordered_map< int16_t, std::vector< std::pair< std::string, EventCB > > > events_map_;
    IMessageCenter * msg_center_;
};

#endif // _EVENT_CENTER_HPP
