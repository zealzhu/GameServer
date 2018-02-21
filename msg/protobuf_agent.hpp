#ifndef _PROTOBUF_AGENT_HPP
#define _PROTOBUF_AGENT_HPP

#include "protobuf_processor.hpp"

template< class T>
class ProtobufAgent
{
public:
    static ProtobufAgent< T > & Instance() {
        static ProtobufAgent instance;
        return instance;
    }

    template<typename Obj, typename Msg>
    bool AddHandler(Obj* obj, uint16_t id, void (Obj::*func)(T * sess, Msg&)) {
        processor_.AddHandler(obj, id, func);
    }

    bool Dispatch(T * session, proto::Protobuf & protobuf) {
        processor_.Dispatch(session, protobuf);
    }

private:
    ProtobufAgent() = default;
    ProtobufProcessor< T > processor_;
};

#endif
