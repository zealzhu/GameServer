#include "timer_queue.h"
#include "stdio.h"
#include "../tools/game_log.h"
#include <thread>

class MyTimer;
TimerQueue te;
class MyTimer : public ITimer
{
public:
    MyTimer(int id) : id(id) {}
    virtual void OnTime() {
        SetNode(nullptr);
        logger_info("task {} run", id);
        //printf("task %d run\n", id);
        //te.AddTimer(this, 0, 1, 800);
    }

private:
    int id;
};

int main()
{
    GLog.Initialize();
    bool running = true;
    MyTimer timer1(1);
    MyTimer timer2(2);
    MyTimer timer3(3);
    te.AddTimer(&timer1, 100, REPEAT_FOREVER, 10);
    //te.AddTimer(&timer2, 200, 2, 800);
    //te.AddTimer(&timer3, 300, 1, 800);

    std::thread t([&]() {
        while(running) {
            te.Tick();
        }
    });

    //getchar();
    //te.StopTimer(&timer1);
    getchar();
    te.AddTimer(&timer3, 300, 1, 800);
    getchar();
    te.KillAll();
    getchar();
    running = false;
    t.join();

    return 0;
}
