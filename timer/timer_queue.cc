#include "timer_queue.h"

#include <chrono>
#include <assert.h>

using namespace std;
using namespace std::chrono;

TimerNode::TimerNode(ITimer * timer, int64_t delay, int32_t count, int64_t interval)
    : timer_(timer), interval_(interval), count_(count), stop_(false)
{
    timer_ = timer;
    interval_ = interval;
    count_ = count;
    SetExpire(delay);
}

void TimerNode::SetExpire(int64_t delay)
{
    milliseconds mill_time(delay + interval_);
    nanoseconds nano_time = duration_cast< nanoseconds >(mill_time);
    //printf("mill_time: %ld nano_time: %ld\n", mill_time.count(), nano_time.count());
    // 计算过期时间
    expire_ = duration_cast< nanoseconds >(high_resolution_clock::now().time_since_epoch()).count() + nano_time.count();
    //printf("expire: %ld\n", expire_);
}

bool TimerNode::Run()
{
    if(count_ == 0) {
        return false;
    }
    timer_->OnTime();
    if(count_ != REPEAT_FOREVER) {
        count_--;
    }

    if(count_ == 0) {
        return false;
    }
    return true;
}

TimerQueue::TimerQueue(int32_t size)
{
    timer_heap_.reserve(size);
}

TimerQueue::~TimerQueue()
{
    KillAll();
}

void TimerQueue::AddTimer(ITimer * timer, int64_t delay, int32_t count, int64_t interval)
{
    assert(timer != nullptr);
    assert(timer->GetNode() == nullptr);

    TimerNode * node = new TimerNode(timer, delay, count, interval);
    timer->SetNode(node);
    InsertTimerNode(node);
}

void TimerQueue::StopTimer(ITimer * timer)
{
    assert(timer != nullptr);
    auto timer_node = timer->GetNode();
    assert(timer_node != nullptr);
    timer_node->Stop();
    timer->SetNode(nullptr);
}

bool TimerQueue::Tick()
{
    std::unique_lock< std::mutex > lck(lock_);
    if(timer_heap_.size() == 0) {
        return false;
    }

    int64_t current = duration_cast< nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
    auto top = timer_heap_[0];

    // not tick
    if(!top || current < top->GetExpire()) {
        return false;
    }
    lck.unlock();
    // tick
    if(!top->IsStop()) {
        bool again = top->Run();
        PopTop();
        if(again) {
            // reset expire
            top->SetExpire();
            InsertTimerNode(top);
        }
        else {
            delete top;
        }
    }
    else {
        PopTop();
        delete top;
    }
    return true;
}

void TimerQueue::KillAll()
{
    std::lock_guard< std::mutex > lck(lock_);
    for(auto node : timer_heap_) {
        delete node;
    }
    timer_heap_.clear();
    std::vector< TimerNode * > temp(20);
    timer_heap_.swap(temp);
}

void TimerQueue::InsertTimerNode(TimerNode * node)
{
    std::lock_guard< std::mutex > lck(lock_);
    int i = timer_heap_.size();
    timer_heap_.push_back(node);
    // 从最后一个节点开始向上插入, expire较小的在上面
    while(i != 0 && *node < *timer_heap_[i / 2])
    {
        timer_heap_[i] = timer_heap_[i / 2];
        i /= 2; // 移到父节点
    }
    timer_heap_[i] = node;
}

void TimerQueue::PopTop()
{
    std::lock_guard< std::mutex > lck(lock_);
    if(timer_heap_.size() == 0) {
        return;
    }

    auto pop_node = timer_heap_[0];
    auto last_node = timer_heap_.back();
    timer_heap_.pop_back();
    timer_heap_[0] = last_node;

    // i->root ci->left child
    int i = 0, ci = 1;
    int current_size = timer_heap_.size();
    while(ci < current_size) {
        // get the min child
        if((ci + 1) < current_size && *timer_heap_[ci] > *timer_heap_[ci + 1]) {
            // right child < left child
            ci++;
        }

        // if child node < root node
        if(*timer_heap_[ci] < *last_node) {
            timer_heap_[i] = timer_heap_[ci];
            timer_heap_[ci] = last_node;
            // change current root index and child index
            i = ci;
            ci *= 2;
        }
        else {
            break;
        }
    }
}
