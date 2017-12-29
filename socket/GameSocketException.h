/**
 * @file GameSocketException.h
 * @brief
 * @author zhu peng cheng
 * @version 1.0
 * @date 2017-12-29
 */
#ifndef _GAME_SOCKET_EXPECTION_H
#define _GAME_SOCKET_EXPECTION_H

#include "GameSocketType.h"
#include <exception>
#include <string.h>           // strerror()

namespace GameSocketLib
{

class GameSocketException : public std::exception
{
public:

    GameSocketException() : error_code_(errno)
    {}

    GameSocketException() :

    inline int ErrorCode() const
    {
        return this->error_code_;
    }

    inline const char * PrintError()
    {
        return strerror(this->error_code_);
    }

protected:
    int error_code_;
};

}
#endif //_GAME_SOCKET_EXPECTION_H

