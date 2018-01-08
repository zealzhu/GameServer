/**
 * @file Protocol.h
 * @brief 协议类
 * @author zhu peng cheng
 * @version 1.0
 * @date 2018-01-05
 */
#ifndef _GAME_PROTOCOL_H
#define _GAME_PROTOCOL_H

#include <socket/Connection.h>
#include <google/protobuf/message.h>

namespace ProtocolLib
{

typedef std::shared_ptr<google::protobuf::Message> MessagePtr;

class Protocol
{
public:
    // 序列化数据成消息并插入消息队列
    static void Translate(const char * buffer, unsigned int size);

    // 解码
    static MessagePtr Decode();

    // 编码
    static char * Encode(const MessagePtr msg, unsigned int size);

    // 读取头部大小
    static unsigned int ReadHead(const char * buff, int size);

    static google::protobuf::Message * CreateMessage(const std::string & type_name);
};

}

#endif //_GAME_PROTOCOL_H

