/**
 * @file ConnectionManager.cpp
 * @brief 连接管理器
 * @author zhu peng cheng
 * @version 1.0
 * @date 2018-01-05
 */
#include "ConnectionManager.h"
#include "Connection.h"
#include <tools/GameLog.h>

namespace GameSocketLib
{
ConnectionManager::ConnectionManager()
{}

void ConnectionManager::AddConnection(DataSocket & sock)
{
    Connection conn(sock);

    if(AvailableConnections() == 0)
    {
        logger_warn("连接已达到最大数量 {}, 不再接受新连接", MAX);
        // 关闭连接
        conn.CloseSocket();
        return;
    }

    // 设置为非阻塞模式
    conn.SetBlocking(false);
    this->socket_set_.AddSocket(conn.GetSocket());
    this->connection_list_.emplace_back( conn );
}

void ConnectionManager::Manager()
{
    this->Recv();
    this->Send();
    this->CloseConnections();
}

void ConnectionManager::Recv()
{
    int size = 0;
    if(TotalConnections() > 0)
    {
        // 轮询是否有数据
        size = this->socket_set_.Poll();
    }

    if(size > 0)
    {
        for(auto & conn : this->connection_list_)
        {
            if(this->socket_set_.HasActivity(conn.GetSocket()))
            {
                try
                {
                    conn.Receive();
                }
                catch(...)
                {
                    // 任何异常都进行关闭连接
                    conn.SetShouldClose();
                    this->Close(conn.GetSocket());
                }
            }
        }
    }
}

void ConnectionManager::Send()
{

}

void ConnectionManager::Close(gsocket sock)
{
    auto it = std::find_if(this->connection_list_.end(), this->connection_list_.end(),
            [sock](const Connection & conn){
                return (conn.GetSocket() == sock);
            });

    if(it == this->connection_list_.end())
        return;

    logger_info("connection {}:{} 关闭, sock {}", it->GetRemoteAddress(), it->GetRemotePort(), sock);
    it->CloseSocket();
    this->connection_list_.erase(it);
    this->socket_set_.RemoveSocket(sock);
}

void ConnectionManager::CloseConnections()
{
    auto it = this->connection_list_.begin();
    decltype(it) close_it;

    while(it != this->connection_list_.end())
    {
        close_it = it++;
        if(close_it->GetShouldClose() == true)
        {
            this->Close(close_it->GetSocket());
        }
    }
}
}
