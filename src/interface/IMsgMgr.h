#ifndef _IMSG_MGR_H
#define _IMSG_MGR_H

#include <functional>
#include <ByteBuffer.hpp>

typedef std::function<void(uint32_t session_id, ByteBuffer & buffer)> MessageHandler;
typedef std::function<void(int32_t type, const char * context, int32_t size)> EventHandler;

class IMsgMgr
{
public:
	virtual void RegistMessageHandler(int32_t type, MessageHandler handler) = 0;
	virtual void RegistEventHandler(int32_t type, EventHandler handler) = 0;

	virtual void TransforMessage(uint32_t session_id, int32_t type, ByteBuffer & buffer) = 0;
};

#endif // _IMSG_MGR_H