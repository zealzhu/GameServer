#ifndef _TCP_SESSION_H
#define _TCP_SESSION_H

#include "znet/znet.hpp"
#include "isender.hpp"
#include <protocol.hpp>
#include <protobuf_processor.hpp>
#include <event_processor.hpp>
#include <game_log.h>
#include <protobuf_define.hpp>
#include <base.pb.h>
#include <chrono>

#define CLIENT_DISCONNECT_TIME 5000

class TcpSession : public znet::EventHandler< 8192, 8192, proto::Protobuf >, public ISender
{
public:
    TcpSession() : sid_(0)
    {
        last_heart_beat_time_ = GetCurrentTime();
    }
    ~TcpSession() = default;

    virtual void OnConnect() {
    }
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

        if(protobuf->ReadId() == kC2SHeartBeat) {
            static base::HeartBeatAck ack;
            ack.set_code(base::SUCCESS);
            Send(kS2CHeartBeatAck, ack);
            last_heart_beat_time_ = GetCurrentTime();
            //logger_debug("read heart beat");
            delete protobuf;
            return nullptr;
        }

        return protobuf;
    }

    inline int64_t GetCurrentTime() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
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
        auto now = GetCurrentTime();
        if(now - last_heart_beat_time_ > CLIENT_DISCONNECT_TIME) {
            logger_warn("client lose: {} now: {} last: {}", sid_, now, last_heart_beat_time_);
            return -1;
        }

        if(len <= sizeof(int16_t)) {
            return 0;
        }

        proto::ByteBuffer buffer;
        buffer.Append(pData, sizeof(uint16_t));

        uint16_t size = buffer.Read< uint16_t >();
        //logger_debug("read package size: {}", size);
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
        //logger_debug("send size: {} id: {}", size, id);

        return size;
    }

    inline void SetId(int32_t id) { this->sid_ = id; }
    inline virtual int32_t GetId() { return this->sid_; }

private:
    int32_t sid_;
    int64_t last_heart_beat_time_;
    int64_t last_recv_time_;
};

#endif // _TCP_SESSION_H
