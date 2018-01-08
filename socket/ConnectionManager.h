/**
 * @file ConnectionManager.h
 * @brief 连接管理器
 * @author zhu peng cheng
 * @version 1.0
 * @date 2018-01-05
 */
#ifndef _CONNECTION_MANAGER_H
#define _CONNECTION_MANAGER_H

#include "GameSocket.h"
#include "GameSocketSet.h"
#include "Connection.h"
#include <list>

namespace GameSocketLib
{

class ConnectionManager
{
    typedef std::list<Connection> CONNECTION_LIST;
public:
    ConnectionManager();

    /**
     * @brief 添加连接
     *
     * @param sock
     */
    void AddConnection(DataSocket & sock);

    /**
     * @brief 获取剩余连接数
     *
     * @return
     */
    inline int AvailableConnections() const
    {
        return MAX - this->connection_list_.size();
    }

    /**
     * @brief 获取当前已连接数
     *
     * @return
     */
    inline int TotalConnections() const
    {
        return this->connection_list_.size();
    }

    /**
     * @brief 对所有连接的统一管理,执行:
     * 1、接收
     * 2、发送
     * 3、关闭
     */
    void Manager();

    /**
     * @brief 发送消息到匹配到的连接缓存
     *
     * @param sock
     * @param msg
     */
    void SendMsg(gsocket sock, void * msg);

protected:
    /**
     * @brief 监听是否有数据需要接收进行接收
     */
    void Recv();

    /**
     * @brief 遍历所有连接发送所有缓存
     */
    void Send();

    /**
     * @brief 遍历所有连接判断连接的关闭标记是否为true进行关闭,逻辑上关闭
     */
    void CloseConnections();

    /**
     * @brief 物理上对链接进行关闭
     *
     * @param sock
     */
    void Close(gsocket sock);


private:
    CONNECTION_LIST connection_list_;
    GameSocketSet socket_set_;
};

}

#endif // _CONNECTION_MANAGER_H
