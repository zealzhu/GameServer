#include "session.hpp"
#include <game_log.h>
#include <base.pb.h>
#include <user.pb.h>
#include <room.pb.h>
#include <protobuf_define.hpp>
#include <sstream>

using namespace user;
using namespace room;
using namespace base;
using namespace std;

TcpSession * session = nullptr;

class MessageAgent
{
public:
    MessageAgent(int32_t uid, std::string name)
    {
        uid_ = uid;
        rid_ = 0;
        name_ = name;
        RegProtoHandler(this, kS2CCreateRoom, &MessageAgent::CrtCB);
        RegProtoHandler(this, kS2CEnterRoom, &MessageAgent::EntCB);
        RegProtoHandler(this, kS2CGetRoom, &MessageAgent::GetRoomCB);
        RegProtoHandler(this, kS2CLeaveRoom, &MessageAgent::LveCB);
        RegProtoHandler(this, kS2CReady, &MessageAgent::ReadyCB);
        RegProtoHandler(this, kS2CEnterRoomNtf, &MessageAgent::EntNtfCB);
        RegProtoHandler(this, kS2CLeaveRoomNtf, &MessageAgent::LveNtfCB);
        RegProtoHandler(this, kS2CReadyNtf, &MessageAgent::ReadyNtfCB);
        RegProtoHandler(this, kS2CGameBeginNtf, &MessageAgent::GameBeginNtfCB);
        RegProtoHandler(this, kS2CPutCardNtf, &MessageAgent::PutCardNtfCB);
        RegProtoHandler(this, kS2CPutLandlordCardNtf, &MessageAgent::PutLandlordCardNtfCB);
        RegProtoHandler(this, kS2CLandlord, &MessageAgent::LandlordCB);
        RegProtoHandler(this, kS2CLandlordNtf, &MessageAgent::LandlordNtfCB);
    }

    bool Loop() {
        printf("1.get\n2.create\n3.enter\n4.leave\n5.ready\n6.call\n");
        int32_t choose = 0;
        scanf("%d", &choose);
        switch(choose) {
        case 1: SendGetRoom(); break;
        case 2: SendCreateRoom();break;
        case 3: SendEnterRoom();break;
        case 4: SendLeaveRoom();break;
        case 5: SendReady();break;
        case 6: SendCall();break;
        default:
            return false;
        }
        printf("\n");
        return true;
    }

    void SendCreateRoom()
    {
        CreateRoomReq req;
        req.set_id(uid_);
        req.set_account(name_);
        session->Send(kC2SCreateRoom, req);
    }

    void SendEnterRoom()
    {
        EnterRoomReq req;
        req.set_uid(uid_);
        printf("enter room id:");
        int32_t rid = 0;
        scanf("%d", &rid);
        req.set_rid(rid);
        req.set_account(name_);
        session->Send(kC2SEnterRoom, req);
    }

    void SendGetRoom()
    {
        GetRoomReq req;
        req.set_id(uid_);
        session->Send(kC2SGetRoom, req);
    }

    void SendLeaveRoom()
    {
        LeaveRoomReq req;
        req.set_uid(uid_);
        req.set_rid(rid_);
        session->Send(kC2SLeaveRoom, req);
    }

    void SendReady()
    {
        ReadyReq req;
        req.set_uid(uid_);
        req.set_rid(rid_);
        req.set_ready(true);
        session->Send(kC2SReady, req);
    }

    void SendCall()
    {
        LandlordReq req;
        req.set_uid(uid_);
        req.set_rid(rid_);
        printf("enter call:");
        int32_t call = 0;
        scanf("%d", &call);
        req.set_call(call);
        session->Send(kC2SLandlord, req);
    }

private:
    void CrtCB(CreateRoomResp & rsp)
    {
        if(rsp.result() != SUCCESS) {
            logger_info("create failed");
            return;
        }
        logger_info("create room id: {} index: {}", rsp.rid(), rsp.index());
        rid_ = rsp.rid();
        index_ = rsp.index();
    }
    void EntCB(EnterRoomResp & rsp)
    {
        switch(rsp.result())
        {
        case PLAYER_HAS_IN_ROOM:
            logger_info("has in room");
            break;
        case ROOM_NOT_EXIST:
            logger_info("not exist");
            break;
        case ROOM_PLAYER_FULL:
            logger_info("full");
            break;
        case ROOM_ENTER_FAIL:
            logger_info("enter failed");
            break;
        case SUCCESS:
            logger_info(" room id: {} index: {}", rsp.rid(), rsp.index());
            rid_ = rsp.rid();
            index_ = rsp.index();
            break;
        default:
            break;
        }
    }
    void GetRoomCB(GetRoomResp & rsp)
    {
        for(auto room : rsp.rooms())
        {
            logger_info(" room id: {} name: {} count: {}", room.id(), room.name(), room.count());
        }
    }
    void LveCB(LeaveRoomResp & rsp)
    {
        switch(rsp.result())
        {
        case PLAYER_NOT_IN_ROOM:
            logger_info("not in room");
            break;
        case ROOM_NOT_EXIST:
            logger_info("not exist");
            break;
        case ROOM_HAS_START:
            logger_info("start");
            break;
        case SUCCESS:
            logger_info("leave success");
            break;
        default:
            break;
        }
    }
    void ReadyCB(ReadyResp & rsp)
    {
        if(rsp.result() != SUCCESS) {
            logger_info("has ready");
            return;
        }
        logger_info("set ready: 1");
    }
    void EntNtfCB(EnterRoomNtf & ntf)
    {
        logger_info("user: {} enter room: {} in index: {}", ntf.uid(), ntf.rid(), ntf.index());
    }
    void LveNtfCB(LeaveRoomNtf & ntf)
    {
        logger_info("user: {} leave room: {} from index: {}", ntf.uid(), ntf.rid(), ntf.index());
    }
    void ReadyNtfCB(ReadyNtf & ntf)
    {
        logger_info("user: {} in room: {} in index: {} ready: {}", ntf.uid(), ntf.rid(), ntf.index(), ntf.ready());
    }
    void GameBeginNtfCB(GameBeginNtf & ntf)
    {
        logger_info("room: {} begin. first: {}", ntf.rid(), ntf.first());
    }
    void PutCardNtfCB(PutCardNtf & ntf)
    {
        ostringstream os;
        for(int i = 0; i < ntf.cid().size(); i++) {
            os << ntf.cid()[i] << " ";
        }
        logger_info("recv card: {}", os.str());
    }
    void PutLandlordCardNtfCB(PutLandlordCardNtf & ntf)
    {

    }
    void LandlordCB(LandlordResp & rsp)
    {
        if(rsp.result() != SUCCESS) {
            logger_info("request landlord failed");
            return;
        }
        logger_info("request landlord success");
    }
    void LandlordNtfCB(LandlordNtf & ntf)
    {
        logger_info("recv index: {} call landlord: {}, next index: {}", ntf.cindex(), ntf.call(), ntf.nindex());
    }

    std::string name_;
    int32_t uid_;
    int32_t rid_;
    int32_t index_;
};

int main()
{
    GameTools::GameLog::Instance().Initialize();
    znet::TcpService< TcpSession > service;
    service.config.threads = 1;
    service.config.app_run = true;
    service.AddConnector("127.0.0.1", 9999);
    service.Start([](znet::NetEvent evt, void * data) {

        switch(evt) {
        case znet::kEvtSessionCreate:
            session = (TcpSession *)data;
            break;
        case znet::kEvtConnectFail:
            printf("connect server failed.");
            exit(0);
        default:
            break;
        }
        return nullptr;
    });

    logger_info("enter you id:");
    int32_t id;
    scanf("%d", &id);
    logger_info("enter you name:");
    char name[50];
    scanf("%s", name);
    MessageAgent handler(id, name);

    while(handler.Loop());

    service.Stop();

    return 0;
}
