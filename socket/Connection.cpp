/**
 * @file Connection.cpp
 * @brief 连接
 * @author zhu peng cheng
 * @version 1.0
 * @date 2018-01-05
 */
#include "Connection.h"

namespace GameSocketLib
{

void Connection::SendBuffer()
{
    if(this->send_buffer_.size() > 0)
    {
        // 发送
        int sended = DataSocket::Send(this->send_buffer_.data(), this->send_buffer_.size());

        // 移除发送的数据
        this->send_buffer_.erase(0, sended);
    }
}

//void Connection::Receive()
//{
    //unsigned int total_received = 0;
    //unsigned int received = 0;
    //unsigned int message_size = 0;

    //{
        //char head_buffer[RECEIVE_HEADER_SIZE] = { 0 };

        //// 接收头
        //do
        //{
            //received = DataSocket::Receive(head_buffer + total_received, RECEIVE_HEADER_SIZE - total_received);
            //total_received += received;

        //} while(total_received != RECEIVE_HEADER_SIZE);
        //total_received = 0;
        //message_size = Protocol::ReadHead(head_buffer, RECEIVE_HEADER_SIZE);
    //}

    //{
        //const int size = message_size;
        //char buffer[size] = 0;

        //// 接收数据本身
        //do
        //{
            //received = DataSocket::Receive(buffer + total_received, size - total_receivedi);
            //total_received += received;
        //} while(total_received != size);

        //// 传输数据
        //Protocol::Translate(GetSocket(), buffer, size);
    //}
//}


}

