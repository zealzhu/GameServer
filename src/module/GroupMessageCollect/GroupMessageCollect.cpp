#include "GroupMessageCollect.h"
#include "IMsgMgr.h"
#include "IDB.h"
#include "message.pb.h"

IContext * GroupMessageCollect::pContext = nullptr;
IDBHelper * GroupMessageCollect::pDBHelper = nullptr;

bool GroupMessageCollect::Initialize(IContext * pContext)
{
	GroupMessageCollect::pContext = pContext;
	GroupMessageCollect::pDBHelper = pContext->GetDBHelper();
	return true;
}

void GroupMessageCollect::Start(IContext * pContext)
{
	RegMessageHandler(ClientMsgId::C2S_GROUP_MESSAGE_INFO, &GroupMessageCollect::OnCollectGroupMessage);
	RegMessageHandler(ClientMsgId::C2S_GROUP_MESSAGE_FILTER, &GroupMessageCollect::OnFilterGroupMessage);
}

void GroupMessageCollect::Destory(IContext * pContext)
{
}

void GroupMessageCollect::OnCollectGroupMessage(uint32_t session_id, ByteBuffer & buffer)
{
	GroupMessageInfo info;
	buffer >> info.age;
	buffer >> info.sex;
	buffer >> info.fromGroup;
	buffer >> info.fromQQ;
	buffer >> info.sendTime;
	buffer >> info.name;
	buffer >> info.position;
	buffer >> info.message;

	tools::GBKToUTF8(info.name);
	tools::GBKToUTF8(info.position);
	tools::GBKToUTF8(info.message);

	AddMessage(info);

#ifdef WIN32
	tools::UTF8ToGBK(info.name);
	tools::UTF8ToGBK(info.position);
	tools::UTF8ToGBK(info.message);
#endif
	InfoLog("[%llu] [%s] [%s]: %s", info.fromQQ, info.name.c_str(), info.position.c_str(), info.message.c_str());
}

void GroupMessageCollect::OnFilterGroupMessage(uint32_t session_id, ByteBuffer & buffer)
{
	auto filter_sex_msg = pDBHelper->DeleteRecord("message", "qq in(select qq from user where sex=0)");
	auto filter_sex_user = pDBHelper->DeleteRecord("user", "sex=0");
	InfoLog("Filter by sex ok");
}

void GroupMessageCollect::AddMessage(GroupMessageInfo & msg) {
	if (!IsExitUser(msg.fromQQ)) {
		InsertUserToDB(msg);
	}
	InsertMessageToDB(msg);
	UpdateLastSendTime(msg);
}

bool GroupMessageCollect::IsExitUser(int64_t qq) {
	RecordData data;
	data["qq"] = MAKE_INT_VALUE(qq);
	auto result = pDBHelper->QueryRecord("user", "id", data);
	return result.size() > 0;
}

bool GroupMessageCollect::InsertUserToDB(GroupMessageInfo & msg) {
	RecordData data;
	data["qq"] = MAKE_INT_VALUE(msg.fromQQ);
	data["sex"] = MAKE_INT_VALUE(msg.sex);
	data["age"] = MAKE_INT_VALUE(msg.age);
	data["name"] = MAKE_STRING_VALUE(msg.name);
	data["position"] = MAKE_STRING_VALUE(msg.position);
	return pDBHelper->InsertRecord("user", data) > 0;
}

bool GroupMessageCollect::InsertMessageToDB(GroupMessageInfo & msg) {
	RecordData data;
	data["qq"] = MAKE_INT_VALUE(msg.fromQQ);
	data["`group`"] = MAKE_INT_VALUE(msg.fromGroup);
	data["msg"] = MAKE_STRING_VALUE(msg.message);
	data["time"] = MAKE_INT_VALUE(msg.sendTime);
	return pDBHelper->InsertRecord("message", data) > 0;
}

bool GroupMessageCollect::UpdateLastSendTime(GroupMessageInfo & msg) {
	RecordData where;
	where["qq"] = MAKE_INT_VALUE(msg.fromQQ);

	RecordData columns;
	columns["lastSend"] = MAKE_INT_VALUE(msg.sendTime / 1000);

	return pDBHelper->UpdateRecord("user", columns, where) > 0;
}