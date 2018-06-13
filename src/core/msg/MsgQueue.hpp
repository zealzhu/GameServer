#ifndef _MSG_QUEUE_H
#define _MSG_QUEUE_H

#include <mutex>
#include <condition_variable>
#include <queue>

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
            this->queue_.emplace(message);
            this->con_condition_.notify_all();
        }

        T Pop()
        {
            T message;
            std::unique_lock< std::mutex > lck(this->lock_);
            this->con_condition_.wait(lck, [this](){ return this->queue_.size() != 0; });
            message = this->queue_.front();
            this->queue_.pop();
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


#endif // _MSG_QUEUE_H
