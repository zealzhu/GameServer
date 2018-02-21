#ifndef _GAME_SERVICD_H
#define _GAME_SERVICE_H

#include "session_factory.hpp"

class GameService
{
public:
    static GameService & Instance();

    bool Initialize();
    bool Start();
    void Stop();

private:
    GameService() = default;
    GameService & operator=(GameService &) = delete;

    void * NetEventHandler(znet::NetEvent evt, void * data);

    znet::TcpService< TcpSession, SessionFactory > service_;
};

#define GService GameService::Instance()
#endif // _GAME_SERVICE_H



