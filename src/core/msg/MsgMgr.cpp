#include "MsgMgr.h"
#include <assert.h>

void MsgMgr::RegistMessageHandler(int32_t type, MessageHandler handler)
{
	auto find = message_handlers_.find(type);
	if (find != message_handlers_.end()) {
		printf("find handler!\n");
		assert(false);
		return;
	}
	message_handlers_.insert(std::make_pair<int32_t, MessageHandler>(std::move(type), std::move(handler)));
}

void MsgMgr::RegistEventHandler(int32_t type, EventHandler handler)
{
	auto find = event_handlers_.find(type);
	if (find != event_handlers_.end()) {
		printf("find handler!\n");
		assert(false);
		return;
	}
	event_handlers_.insert(std::make_pair<int32_t, EventHandler>(std::move(type), std::move(handler)));
}

void MsgMgr::TransforMessage(uint32_t session_id, int32_t type, ByteBuffer & buffer)
{
	auto find = message_handlers_.find(type);
	if (find != message_handlers_.end()) {
		find->second(session_id, buffer);
	}
}

void MsgMgr::InsertMessage(SessionMessage & msg)
{
	msg_queue_.Push(msg);
}

bool MsgMgr::GetProtoMessage(SessionMessage & msg)
{
	if (msg_queue_.Size() > 0) {
		msg = msg_queue_.Pop();
		return true;
	}
	else
		return false;
}
