#ifndef _ITIMER_HPP
#define _ITIMER_HPP

#include <atomic>

class IContext;

class ITimerNode
{
public:
    virtual ~ITimerNode() {}
    virtual void Stop() = 0;
};

class ITimer
{
public:
    ITimer() : node_(nullptr) {}
    void SetNode(ITimerNode * node) {
        node_.store(node);
    }
    ITimerNode * GetNode() {
        return node_.load();
    }
    virtual void OnTime(IContext * context) = 0;

    virtual ~ITimer() {}

private:
    std::atomic< ITimerNode * > node_;
};

#endif // _ITIMER_HPP
