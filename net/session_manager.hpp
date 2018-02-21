#ifndef _SESSION_MANAGER_HPP
#define _SESSION_MANAGER_HPP

#include "session.hpp"
#include <unordered_map>

class SessionManager
{
public:
    static SessionManager & Instance()
    {
        static SessionManager instance;
        return instance;
    }

    bool Add(TcpSession * sess)
    {
        std::lock_guard< std::mutex > lck(this->lock_);
        auto ret = this->map_.insert(std::pair< int32_t, TcpSession* >(sess->GetId(), sess));
        return ret.second;
    }

    void Remove(int32_t sid)
    {
        std::lock_guard< std::mutex > lck(this->lock_);
        auto find = this->map_.find(sid);
        if(find == this->map_.end()) {
            return;
        }
        this->map_.erase(sid);
    }

    TcpSession * Find(int32_t sid)
    {
        auto find = this->map_.find(sid);

        if(find == this->map_.end()) {
            return nullptr;
        }
        return find->second;
    }

private:
    SessionManager()
    {}

    // session id <=> session
    typedef std::unordered_map< int32_t, TcpSession* > SessionMap;
    SessionMap map_;
    std::mutex lock_;
};

#endif // _SESSION_MANAGER_HPP
