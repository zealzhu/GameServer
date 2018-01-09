/**
 * @file ListeningManager.cpp
 * @brief 监听管理器
 * @author zhu peng cheng
 * @version 1.0
 * @date 2018-01-04
 */
#include "ListeningManager.h"
#include "GameSocketException.h"

namespace GameSocketLib
{

void ListeningManager::AddPort(unsigned int port)
{
    // 连接数量达到上限
    if(this->socket_list_.size() >= MAX)
    {
        throw GameSocketException(SocketError::kSocketLimitReached);
    }

    ListeningSocket sock;
    sock.Listen(port);
    sock.SetBlocking(false);
    this->socket_list_.push_back(sock);
    this->socket_set_.AddSocket(sock.GetSocket());
}

void ListeningManager::Listen()
{
    DataSocket sock;

    if(this->socket_set_.Poll())
    {
        for(auto & lsock : this->socket_list_)
        {
            if(this->socket_set_.HasActivity(lsock.GetSocket()))
            {
                sock = lsock.Accept();

                if(this->connections_manager_)
                {
                    this->connections_manager_->AddConnection(sock);
                }
            }
        }
    }
}

}
