/**
 * @file ListeningManager.h
 * @brief 监听管理器
 * @author zhu peng cheng
 * @version 1.0
 * @date 2018-01-04
 */
#ifndef _LISTENING_MANAGER_H
#define _LISTENING_MANAGER_H

#include "GameSocket.h"
#include "GameSocketSet.h"
#include <list>

namespace GameSocketLib
{

class ListeningManager
{
public:
    /**
     * @brief 添加监听端口
     *
     * @param port
     */
    void AddPort(unsigned int port);

    /**
     * @brief 开始监听
     */
    void Listen();

private:
    std::list<ListeningSocket> socket_list_;
    GameSocketSet socket_set_;
};

}

#endif
