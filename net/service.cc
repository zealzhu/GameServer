#include "service.h"
#include <config_manager.h>
#include <game_log.h>

GameService & GameService::Instance()
{
    static GameService instance;
    return instance;
}

bool GameService::Initialize()
{
    this->service_.config.app_run = GConfig.GetConfigParam("net.apprun", "true") == "true";
    this->service_.config.reuse = GConfig.GetConfigParam("net.reuse", "true") == "true";
    this->service_.config.threads = atoi(GConfig.GetConfigParam("net.threads", "1").c_str());
    this->service_.config.backlogs = atoi(GConfig.GetConfigParam("net.backlogs", "10").c_str());
    this->service_.config.parser = true;

    std::string host = GConfig.GetConfigParam("net.host", "127.0.0.1");
    uint16_t port = atoi(GConfig.GetConfigParam("net.port", "9999").c_str());
    this->service_.AddListener(host.c_str(), port);
    logger_info("listen host: {} port: {}", host, port);

    return true;
}

bool GameService::Start()
{
    logger_info("service start");
    this->service_.Start(std::bind(&GameService::NetEventHandler, this, std::placeholders::_1, std::placeholders::_2));
}

void GameService::Stop()
{
    logger_info("service stop");
    this->service_.Stop();
}

void * GameService::NetEventHandler(znet::NetEvent evt, void * data)
{
    switch(evt)
    {
    default:
        break;
    }

    return nullptr;
}
