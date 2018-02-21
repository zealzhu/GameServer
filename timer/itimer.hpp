#ifndef _ITIMER_HPP
#define _ITIMER_HPP

#include <atomic>

class ITimerNode
{
public:
    virtual ~ITimerNode() {}
    virtual void Stop() = 0;
};

class ITimer
{
public:
    void SetNode(ITimerNode * node) {
        node_.store(node);
    }
    ITimerNode * GetNode() {
        return node_.load();
    }
    virtual void OnTime() = 0;

    virtual ~ITimer() {}

private:
    std::atomic< ITimerNode * > node_;
};

#endif // _ITIMER_HPP
