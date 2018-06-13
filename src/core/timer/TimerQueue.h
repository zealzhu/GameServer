#ifndef _TIME_QUEUE_H
#define _TIME_QUEUE_H

#include "ITimer.h"
#include "stdint.h"
#include <vector>
#include <stdio.h>
#include <mutex>
#include <atomic>
#define REPEAT_FOREVER -1

class TimerNode : public ITimerNode
{
public:
    TimerNode() = default;
    TimerNode(const TimerNode &) = default;
    TimerNode(ITimer * timer, uint64_t delay, int32_t count, uint64_t interval);
    virtual ~TimerNode() { /* printf("release timer node\n"); */ }

    int64_t GetExpire() { return expire_; }
    void SetExpire(int64_t dalay = 0);
    void SetCount(int32_t count) { count_ = count; }
    int64_t GetCount() { return count_; }
    bool IsStop() { return stop_; }
    virtual void Stop() { stop_ = true; }
    bool Run(IContext * context);

    bool operator > (const TimerNode & node)
    {
        return expire_ > node.expire_;
    }
    bool operator < (const TimerNode & node)
    {
        return expire_ < node.expire_;
    }

private:
    ITimer * timer_;
    int32_t count_;
    int64_t interval_;
    int64_t expire_;
    std::atomic< bool > stop_;
};

class TimerQueue
{
public:
    static void StopTimer(ITimer * timer);

    void AddTimer(ITimer * timer, uint64_t delay, int32_t count, uint64_t interval);

    bool Tick(IContext * context);

    void KillAll();

    TimerQueue(int32_t size = 20);
    ~TimerQueue();

private:
    void InsertTimerNode(TimerNode * node);
    void PopTop();

    std::vector< TimerNode * > timer_heap_;
    std::mutex lock_;
};

#endif // _TIME_ENGINE_H
