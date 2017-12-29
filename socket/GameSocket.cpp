/**
 * @file GameSocket.cpp
 * @brief
 * @author zhu peng cheng
 * @version 1.0
 * @date 2017-12-29
 */

#include "GameSocket.h"
#include "GameSocketException.h"

namespace GameSocketLib
{

void BaseSocket::Close()
{
    close(this->socket_);
    socket_ = -1;
}

void BaseSocket::SetBlocking(bool is_blocking)
{
    int err = 0;
    // 获取文件描述
    bool flag = fcntl(this->socket_, F_GETFL, 0);

    // 设置为非阻塞
    if(!is_blocking)
    {
        fcntl(this->socket_, F_SETFL, flag | O_NONBLOCK);
    }
    // 设置为阻塞
    else
    {
        fcntl(this->socket_, F_SETFL, flag & ~O_NONBLOCK);
    }
    // 接受错误
    err = fcntl(this->socket_, F_SETFL, flag);

    // 有错误抛异常
    if(err == -1)
    {
        throw(GameSocketException());
    }
    this->blocking_ = is_blocking;
}

BaseSocket::BaseSocket(gsocket socket)
    : socket_(socket)
{
    // socket已建立就获取信息
    if(socket != -1)
    {
        socklen_t len = sizeof(this->local_info_);
        getsockname(socket, (sockaddr *)(&this->local_info_), &len);
    }

    // 默认阻塞模式
    this->blocking_ = true;
}

ListeningSocket::ListeningSocket()
    : is_listening_(false)
{}

void ListeningSocket::Listen(unsigned int port)
{
    int err = 0;

    // 创建一个socket
    if(socket_ == -1)
    {
        socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(socket_ == -1)
        {
            throw GameSocketException();
        }
    }

    int reuse = 1;
    // 配置socket属性，让socket能够重复使用ip和端口
    err = setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, (char *)(&reuse), sizeof(reuse));
    if(err != 0)
    {
        throw GameSocketException();
    }

    // 配置socket地址信息
    local_info_.sin_family = AF_INET;
    local_info_.sin_port = htons(port);
    local_info_.sin_addr.s_addr = htonl(INADDR_ANY);    // 0.0.0.0
    memset(&(local_info_.sin_zero), 0, 8);

    // 绑定socket信息
    err = bind(socket_, (struct sockaddr *)&local_info_, sizeof(struct sockaddr));
    if(err == -1)
    {
        throw GameSocketException();
    }

    // 监听，设置监听队列为8
    err = listen(socket_, 8);
    if(err == -1)
    {
        throw GameSocketException();
    }

    this->is_listening_ = true;
}

DataSocket ListeningSocket::Accept()
{
    gsocket accept_socket;
    struct sockaddr accept_addr;

    socklen_t size = sizeof(struct sockaddr);
    accept_socket = accept(socket_, (struct sockaddr *)&accept_socket, &size);

    if(accept_socket == -1)
    {
        throw GameSocketException();
    }

    // 建立连接

    return DataSocket(accept_socket);
}

DataSocket::DataSocket(gsocket socket)
    : is_connected_(false), BaseSocket(socket)
{
    // 获取连接信息
    if(socket != -1)
    {
        socklen_t size = sizeof(this->remote_info_);
        getpeername(socket, (struct sockaddr *)&remote_info_, &size);
        is_connected_ = true;
    }
}

void DataSocket::Connect(ipaddress ip, unsigned int port)
{
    int err = 0;

    if(is_connected_)
    {
        return;
    }

    // 创建连接socket
    if(socket_ == -1)
    {
        socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if(socklen_t_ == -1)
        {
            throw GameSocketException();
        }
    }

    // 设置远程socket信息
    remote_info_.sin_port = htons(port);
    remote_info_.sin_addr.s_addr = htonl(ip);
    remote_info_.sin_family = AF_INET;
    memset(&(remote_info_.sin_zero), 0, 8);

    // 连接
    socklen_t size = sizeof(struct sockaddr);
    err = connect(socket_, (struct sockaddr *)&remote_info_, &size);
    if(err == -1)
    {
        throw GameSocketException();
    }

    is_connected_ = true;
}

}
