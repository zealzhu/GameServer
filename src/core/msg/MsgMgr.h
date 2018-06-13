#ifndef _MSG_MGR_H
#define _MSG_MGR_H

#include "IMsgMgr.h"
#include "MsgQueue.hpp"
#include <unordered_map>
#include <message.pb.h>

typedef std::shared_ptr<BaseMessage> MessagePtr;

struct SessionMessage {
	int64_t sid;
	MessagePtr msg;
};

class MsgMgr : public IMsgMgr
{
public:
	static MsgMgr * Instance()
	{
		static MsgMgr instance;
		return &instance;
	}

	virtual void RegistMessageHandler(int32_t type, MessageHandler handler);
	virtual void RegistEventHandler(int32_t type, EventHandler handler);

	virtual void TransforMessage(uint32_t session_id, int32_t type, ByteBuffer & buffer);

	void InsertMessage(SessionMessage & msg);
	bool GetProtoMessage(SessionMessage & msg);

private:
	MsgMgr() = default;

	MsgQueue<SessionMessage> msg_queue_;

	std::unordered_map<int32_t, MessageHandler> message_handlers_;
	std::unordered_map<int32_t, EventHandler> event_handlers_;
};

#endif // _MSG_MGR_H