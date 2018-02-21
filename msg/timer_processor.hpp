#ifndef _TIMER_PROCESSOR_HPP
#define _TIMER_PROCESSOR_HPP

#include <timer_queue.h>
#include <imessage_center.hpp>
#include <game_log.h>
#define START_TIMER(t, d, c, i) TimerProcessor::Instance().StartTimer(t, d, c, i)
#define STOP_TIMER(t) TimerProcessor::Instance().StopTimer(t)

class TimerProcessor
{
public:
    static TimerProcessor & Instance() {
        static TimerProcessor instance;
        return instance;
    }

    void StartTimer(ITimer * timer, int64_t delay, int32_t count, int64_t interval) {
        msg_center_->FindTimerProc()->InsertTimer(timer, delay, count, interval);
    }

    void StopTimer(ITimer * timer) {
        TimerQueue::StopTimer(timer);
    }

    void SetMessageCenter(IMessageCenter * center) {
        this->msg_center_ = center;
    }

private:
    IMessageCenter * msg_center_;
};

#endif // _TIMER_PROCESSOR_HPP
