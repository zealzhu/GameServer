#ifndef _ROOM_MGR_H
#define _ROOM_MGR_H

#include "card_room.h"
#include <imodule.hpp>
#include <room.pb.h>

#include <map>

struct UserInfo
{
public:
    int32_t uid;
    int32_t rid;
    std::string account;
    ISender * sender;
};

class RoomMgr : public IModule
{
public:
    virtual bool Initialize();
    virtual bool Start();
    virtual void Stop();

    virtual std::string GetModuleName() { return "RoomMgr"; }

private:
    int32_t CreateRoom(int32_t uid, const std::string & room_name, ISender * sender);
    void CreateRoom(ISender * sender, room::CreateRoomReq & req);

    int32_t EnterRoom(CardRoom * pRoom, int32_t uid, const std::string & user_name, ISender * sender);
    void EnterRoom(ISender * sender, room::EnterRoomReq & req);

    void LeaveRoom(ISender * sender, room::LeaveRoomReq & req);
    void GetRoom(ISender * sender, room::GetRoomReq & req);
    void Ready(ISender * sender, room::ReadyReq & req);
    void OnLandlord(ISender * sender, room::LandlordReq & req);
    void OnPlay(ISender * sender, room::PlayReq & req);

    void NtfEnterRoom(int32_t uid, int32_t rid, int8_t index);
    void NtfLeaveRoom(int32_t uid, int32_t rid, int8_t index);
    void NtfReady(int32_t uid, int32_t rid, int8_t index, bool ready);

    void CallLandlordNtf(proto::EventBuff & buff);
    void PutLandlordCard(proto::EventBuff & buff);
    void GameBegin(proto::EventBuff & buff);
    void GameOver(proto::EventBuff & buff);
    void PlayError(proto::EventBuff & buff);
    void PlaySuccess(proto::EventBuff & buff);

    bool CheckIsInRoom(int32_t uid);
private:
    std::map< int32_t, UserInfo > user_map_;                            // uid <=> uinfo
    std::map< int32_t, CardRoom * > room_map_;                          // rid <=> room
    std::map< int32_t, std::map< int32_t, int8_t > > roomuser_index_;   // rid <=> <uid, seat_index>
};

#endif // _USER_MGR_H

