/**
 * @file Server.cpp
 * @brief
 * @author zhu peng cheng
 * @version 1.0
 * @date 2017-12-29
 */

#include "tools/ConfigManager.h"
#include "tools/GameLog.h"

#include <iostream>
#include <thread>

/**
 * @brief 用来处理接收的数据
 */
void ReceiveThreadTask()
{
    logger_info("启动接收线程");
}

/**
 * @brief 用来处理要发送的数据
 */
void SendThreadTask()
{
    logger_info("启动发送线程");
}

/**
 * @brief 用来接收连接
 */
void ConnectThreadTask()
{
    logger_info("启动连接线程");
}

/**
 * @brief 用来处理已连接的socket
 */
void SocketManagerThreadTask()
{
    logger_info("启动socket管理线程");
}

void CreateServerThread()
{
    // 创建四个需要用到的线程
    std::thread receive_thread(ReceiveThreadTask);
    receive_thread.join();

    std::thread send_thread(SendThreadTask);
    send_thread.join();

    std::thread connect_thread(ConnectThreadTask);
    connect_thread.join();

    std::thread socket_manager_thread(SocketManagerThreadTask);
    socket_manager_thread.join();
}

/**
 * @brief Main fucntion
 *
 * @param argc argument count
 * @param argv[] arument params
 *
 * @return exit value
 */
int main(int argc, char * argv[])
{
    // 初始化日志
    GameTools::GameLog::Init("log/server.log", 10, 10);


    // 创建服务线程
    CreateServerThread();

    // 主线程用来接收输入
    return 0;
}
