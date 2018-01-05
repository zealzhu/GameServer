/**
 * @file Protocol.h
 * @brief 协议类
 * @author zhu peng cheng
 * @version 1.0
 * @date 2018-01-05
 */
#ifndef _GAME_PROTOCOL_H
#define _GAME_PROTOCOL_H

#include "GameSocket.h"

class Protocol
{
public:
    // 传输
    static void Translate(const char * buffer, unsigned int size);

    // 发送到socket
    static void SendMessage(GameSocketLib::Connection conn, void * message);

    // 解码
    static void Decode();

    // 编码
    static char * Encode();

    // 读取头部大小
    static unsigned int ReadHead(const char * buff, int size);

};

#endif //_GAME_PROTOCOL_H

