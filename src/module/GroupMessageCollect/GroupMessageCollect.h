#include <IModule.h>

struct GroupMessageInfo
{
	uint8_t age;
	uint8_t sex;
	uint64_t fromGroup;
	uint64_t fromQQ;
	uint64_t sendTime;
	std::string name;
	std::string position;
	std::string message;
};

class GroupMessageCollect : public IModule
{
public:
	virtual bool Initialize(IContext * pContext);
	virtual void Start(IContext * pContext);
	virtual void Destory(IContext * pContext);

	void OnCollectGroupMessage(uint32_t session_id, ByteBuffer & buffer);
	void OnFilterGroupMessage(uint32_t session_id, ByteBuffer & buffer);

private:
	static void AddMessage(GroupMessageInfo & msg);
	static bool IsExitUser(int64_t qq);
	static bool InsertUserToDB(GroupMessageInfo & msg);
	static bool InsertMessageToDB(GroupMessageInfo & msg);
	static bool UpdateLastSendTime(GroupMessageInfo & msg);

private:
	static IContext * pContext;
	static IDBHelper * pDBHelper;
};
