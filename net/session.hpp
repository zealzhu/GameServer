#ifndef _TCP_SESSION_H
#define _TCP_SESSION_H

#include "znet/znet.hpp"
#include "isender.hpp"
#include <protocol.hpp>
#include <protobuf_processor.hpp>
#include <event_processor.hpp>
#include <game_log.h>

class TcpSession : public znet::EventHandler< 8192, 8192, proto::Protobuf >, public ISender
{
public:
    TcpSession() : sid_(0)
    {}
    ~TcpSession() = default;

    virtual void OnConnect() {}
    virtual void OnDisconnect() {
        auto buff = new proto::EventBuff(kEventConnectionClose);
        buff->Append(sid_);
        ExecEvent(buff);
    }

    /**
     * @brief 解析消息包
     *
     * @param pData
     * @param len
     *
     * @return
     */
    virtual proto::Protobuf * ParseMessage(const char * pData, size_t len)
    {
        proto::ByteBuffer buffer;
        buffer.Append(pData, len);
        // 消息解包
        proto::Protobuf * protobuf = new proto::Protobuf;
        proto::Decode(*protobuf, buffer, false); // 不加密
        return protobuf;
    }

    /**
     * @brief 读取包大小
     *
     * @param pData
     * @param len
     *
     * @return
     */
    virtual int ReadPacket(const char * pData, size_t len)
    {
        if(len <= sizeof(int16_t)) {
            return 0;
        }

        proto::ByteBuffer buffer;
        buffer.Append(pData, sizeof(uint16_t));

        uint16_t size = buffer.Read< uint16_t >();
        logger_debug("read package size: {}", size);
        return size + sizeof(size);
    }

    virtual void OnMessage(proto::Protobuf * pkg)
    {
        if(!pkg) {
            return;
        }
        ProtobufProcessor::Instance().ExecHandler(this, pkg);
    }

    virtual int Send(int16_t id, google::protobuf::Message & msg)
    {
        proto::Protobuf protobuf;
        proto::ByteBuffer buffer;
        protobuf.SetId(id);
        protobuf << msg;
        proto::Encode(protobuf, buffer, false); // 不加密

        auto size = EventHandler::Send((const char *)buffer.Contents(), buffer.Size());
        logger_debug("send size: {}", size);

        return size;
    }

    inline void SetId(int32_t id) { this->sid_ = id; }
    inline int32_t GetId() { return this->sid_; }

private:
    int32_t sid_;
};

#endif // _TCP_SESSION_H
