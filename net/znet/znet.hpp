#ifndef _Z_NET_H
#define _Z_NET_H

#include <functional>

#define MAX_THREADS 50
#define MAX_WAIT_EVENT 128

namespace znet
{
enum NetRet
{
    kRetOK = 0,
    kRetErr = -1,
    kRetErrSocket = -100,
    kRetErrConnect = -101,
    kRetErrListen = -102,
    kRetErrBind = -103,
    kRetErrBufSize = -104,
};

enum NetEvent
{
    kEvtBind,
    kEvtListen,
    kEvtListenFail,
    kEvtConnect,
    kEvtConnectFail,
    kEvtRecv,
    kEvtSend,
    kEvtDisconnect,
    kEvtSessionCreate,
    kEvtSessionRelease,
};

typedef std::function< void * (NetEvent evt, void * data) > EventHandleFunc;

template < class Session >
    class SessionFactory
    {
    public:
        struct MsgPackage
        {
            Session * session;
            typename Session::Message * message;
        };

        Session * Create()
        {
            return new Session;
        }

        void Release(Session * session_ptr)
        {
            delete session_ptr;
        }
    };

/**
 * @brief 会话句柄
 */
class IEventHandler
{
public:
    virtual void OnConnect() = 0;
    virtual void OnDisconnect() = 0;
    virtual void OnSend(const char * data, size_t len) = 0;
    virtual void OnRecv(const char * data, size_t len) = 0;

    virtual ~IEventHandler() {}
};

class IConnection
{
public:
    virtual int Send(const char * data, size_t len) = 0;
    virtual void Close() = 0;

    virtual ~IConnection() {}
};

struct ITcpService
{
public:
    virtual void Start(EventHandleFunc handle = nullptr) = 0;
    virtual void Stop() = 0;

    virtual void AddListener(const char * address, uint16_t port) = 0;
    virtual void AddConnector(const char * address, uint16_t port) = 0;

    struct Properties
    {
        Properties()
        {
            parser = false;
            reuse = true;
            app_run = false;
            threads = 1;
            backlogs = 10;
        }

        bool parser; // app_run为false时是否解析消息
        bool reuse;
        bool app_run; // 启动消息处理线程
        int threads;  // 线程数
        int backlogs; // listen队列大小
    } config;

    virtual ~ITcpService() {}
};

class IConnector
{
public:
    virtual int Start(EventHandleFunc handler = nullptr) = 0;
    virtual void Stop() = 0;
    virtual int Connect() = 0;

    virtual ~IConnector() {}
};

class IListener
{
public:
    virtual int Start(const ITcpService::Properties & config, EventHandleFunc handler) = 0;
    virtual void Stop() = 0;
    virtual void Run() = 0;
    //virtual void Listen(const char * address, unsigned int port) = 0;

    virtual ~IListener() {}
};

}

#ifdef __linux__
#include "epollnet.hpp"
#endif // __linux__

#endif // _Z_NET_H
