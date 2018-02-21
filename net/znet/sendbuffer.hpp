#ifndef _SEND_BUFFER_H
#define _SEND_BUFFER_H

#include <mutex>
#include <assert.h>
#include <string.h>

namespace znet
{

template <int SendBufferSize = 8192>
    class SendBuffer
    {
        typedef std::function< int (const char *, unsigned int ) > DataHandler;
    public:
        SendBuffer()
            : data_head_(0), data_tail_(0)
        {}

        inline int Peek()
        {
            return this->data_tail_ - this->data_head_;
        }

        bool Push(const char * data, unsigned int len)
        {
            assert(len <= SendBufferSize);
            std::lock_guard< std::mutex > lck(this->lock_);

            // after tail size too small
            if(SendBufferSize - this->data_tail_ < len)
            {
                // move
                if(this->data_head_ > 0)
                {
                    int use_size = this->data_tail_ - this->data_head_;
                    if(use_size > 0)
                    {
                        memmove(this->data_buff_, this->data_buff_ + this->data_head_, use_size);
                        this->data_head_ = 0;
                        this->data_tail_ = use_size;
                    }
                    else
                    {
                        this->data_tail_ = 0;
                        this->data_head_ = 0;
                    }
                }
                // no size
                else
                {
                    return false;
                }
            }

            if(SendBufferSize - this->data_tail_ > len)
            {
                memcpy(this->data_buff_ + this->data_tail_, data, len);
                this->data_tail_ += len;
            }
            else
            {
                return false;
            }

            return true;
        }

        bool Pop(DataHandler handler)
        {
            this->lock_.lock();
            int data_len = this->data_tail_ - this->data_head_;
            this->lock_.unlock();

            int pop_len = handler(data_buff_ + this->data_head_, data_len);
            if(pop_len < 0)
                return false;

            // push some
            if(pop_len < data_len)
            {
                this->lock_.lock();
                this->data_head_ += pop_len;
                this->lock_.unlock();
            }
            // all push
            else
            {
                this->lock_.lock();
                this->data_head_ = this->data_tail_ = 0;
                this->lock_.unlock();
            }
            return true;
        }

    private:
        std::mutex lock_;
        int data_head_;
        int data_tail_;
        char data_buff_[SendBufferSize];
    };

} // namespace zneg

#endif // _SEND_BUFFER_H
