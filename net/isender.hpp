#ifndef _SENDER_HPP
#define _SENDER_HPP
#include <google/protobuf/message.h>

class ISender
{
public:
    virtual int Send(int16_t id, google::protobuf::Message & msg) = 0;
};

#endif
