/**
 * @file Connection.h
 * @brief 连接
 * @author zhu peng cheng
 * @version 1.0
 * @date 2018-01-05
 */
#ifndef _CONNECTION_H
#define _CONNECTION_H

#include "GameSocket.h"
#include <string>

namespace GameSocketLib
{

const static int RECEIVE_HEADER_SIZE = sizeof(int);

class Connection : public DataSocket
{
public:
    // 发送缓存
    void SendBuffer();

    // 接收数据到临时缓存区
    void Receive();

    // 标记需要关闭
    inline void Close() { this->should_close_ = true; }

    // 是否需要关闭
    inline bool GetShouldClose() { return this->should_close_; }

    // 这才是真正关闭连接
    inline void CloseSocket() { DataSocket::Close(); }

protected:

    // 发送缓存
    std::string send_buffer_;

    // 仅仅标记是否要关闭，如果为false则会通过连接管理器进行关闭
    bool should_close_;
}
}

#endif
