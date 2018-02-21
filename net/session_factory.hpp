#ifndef _TCP_SESSION_FACTORY_H
#define _TCP_SESSION_FACTORY_H

#include "session_manager.hpp"

class SessionFactory
{
public:
    struct MsgPackage
    {
        TcpSession * session;
        typename TcpSession::Message * message;
    };

    SessionFactory() : increase_id_(0)
    {}

    TcpSession * Create()
    {
        auto sess = new TcpSession;
        int32_t sid = ++this->increase_id_;
        sess->SetId(sid);
        SessionManager::Instance().Add(sess);
        return sess;
    }

    void Release(TcpSession * session_ptr)
    {
        SessionManager::Instance().Remove(session_ptr->GetId());
        delete session_ptr;
    }

private:
    int32_t increase_id_;
};

#endif // _TCP_SESSION_FACTORY_H
