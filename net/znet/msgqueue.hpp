#ifndef _MSG_QUEUE_H
#define _MSG_QUEUE_H

#include <mutex>
#include <condition_variable>
#include <queue>

namespace znet
{
namespace utils
{

template < class T, int MaxMessageSize = 10000 >
    class MsgQueue
    {
    public:
        MsgQueue()
        {}

        void Push(T message)
        {
            std::unique_lock< std::mutex > lck(this->lock_);
            this->pro_condition_.wait(lck, [this](){ return this->queue_.size() != MaxMessageSize; });
            //if(this->queue_.size() >= MaxMessageSize)
            //{
                //printf("push wait\n");
                //this->pro_condition_.wait(lck);
            //}
            this->queue_.emplace(message);
            printf("push\n");
            this->con_condition_.notify_all();
        }

        T Pop()
        {
            T message;
            std::unique_lock< std::mutex > lck(this->lock_);
            this->con_condition_.wait(lck, [this](){ return this->queue_.size() != 0; });
            //if(this->queue_.size() <= 0)
            //{
                //printf("pop wait\n");
                //this->con_condition_.wait(lck);
            //}
            message = this->queue_.front();
            this->queue_.pop();
            printf("pop\n");
            this->pro_condition_.notify_all();
            return message;
        }

        int Size()
        {
            int size = 0;
            {
                std::lock_guard< std::mutex > lck(this->lock_);
                size = this->queue_.size();
            }

            return size;
        }

    private:
        std::queue< T > queue_;
        std::mutex lock_;
        std::condition_variable pro_condition_;
        std::condition_variable con_condition_;
    };

} // namespace utils
} // namespace znet

#endif // _MSG_QUEUE_H
