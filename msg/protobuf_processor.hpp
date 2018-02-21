#ifndef _PROTOBUF_PROCESSOR_HPP
#define _PROTOBUF_PROCESSOR_HPP

#include <unordered_map>
#include <isender.hpp>
#include <imessage_center.hpp>

class IProtobufHandler : public std::enable_shared_from_this< IProtobufHandler > {
public:
     IProtobufHandler() = default;
     virtual ~IProtobufHandler() = default;
     virtual void Call(ISender * sender, proto::Protobuf& protobuf) = 0;
protected:
     using std::enable_shared_from_this<IProtobufHandler>::shared_from_this;
};

template<typename T>
class ProtobufHandler : public IProtobufHandler {
public:
    explicit ProtobufHandler(std::function< void (ISender *, T&) > f) : func_(f) {
    }
    virtual void Call(ISender * sender, proto::Protobuf& protobuf) {
        T message;
        protobuf >> message;

        func_(sender, message);
    }

private:
    std::function< void (ISender *, T&) > func_;
};

class ProtobufProcessor {
public:
    static ProtobufProcessor & Instance() {
        static ProtobufProcessor instance;
        return instance;
    }

    void SetMessageCenter(IMessageCenter * center) {
        this->msg_center_ = center;
    }

    template<typename Obj, typename Msg>
    bool RegistHandler(Obj* obj, uint16_t id, void (Obj::*func)(ISender * sender, Msg&)) {
        if (!msg_ids_.insert(std::make_pair(id, obj->GetModuleName())).second) {
            return false;
        }

        if (!handlers_.insert(
                std::make_pair(
                    id,
                    std::make_shared<ProtobufHandler< Msg > >(
                        std::bind(func, obj,
                            std::placeholders::_1, std::placeholders::_2)))).second) {
            return false;
        }
        return true;
    }

    void ExecHandler(ISender * sender, proto::Protobuf * protobuf) {
        auto find = msg_ids_.find(protobuf->GetId());
        if(find != msg_ids_.end()) {
            msg_center_->InsertProtobufMessage(find->second, sender, protobuf);
        }
        else if(protobuf != nullptr){
            delete protobuf;
        }
    }

    bool Dispatch(ISender * sender, proto::Protobuf & protobuf) {
        auto it = handlers_.find(protobuf.ReadId());
        if (it != handlers_.end()) {
            it->second->Call(sender, protobuf);
            return true;
        }
        return false;
    }

private:
    ProtobufProcessor() : msg_center_(nullptr) {}
    virtual ~ProtobufProcessor() = default;

    std::unordered_map<uint16_t, std::string > msg_ids_;
    std::unordered_map<uint16_t, std::shared_ptr< IProtobufHandler > > handlers_;

    IMessageCenter * msg_center_;
};

#define RegProtoHandler(obj, pid, cb) ProtobufProcessor::Instance().RegistHandler(obj, pid, cb)
#endif
