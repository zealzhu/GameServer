#include "room_mgr.h"
#include "protobuf_processor.hpp"
#include "event_processor.hpp"
#include "timer_processor.hpp"
#include <sstream>

using namespace room;

bool RoomMgr::Initialize()
{
    RegProtoHandler(this, kC2SCreateRoom, &RoomMgr::CreateRoom);
    RegProtoHandler(this, kC2SEnterRoom, &RoomMgr::EnterRoom);
    RegProtoHandler(this, kC2SLeaveRoom, &RoomMgr::LeaveRoom);
    RegProtoHandler(this, kC2SGetRoom, &RoomMgr::GetRoom);
    RegProtoHandler(this, kC2SReady, &RoomMgr::Ready);
    RegProtoHandler(this, kC2SLandlord, &RoomMgr::OnLandlord);
    RegProtoHandler(this, kC2SPlay, &RoomMgr::OnPlay);

    RegEvent(this, kEventCallLandlordNtf, &RoomMgr::CallLandlordNtf);
    RegEvent(this, kEventGameBegin, &RoomMgr::GameBegin);
    RegEvent(this, kEventPutLandlordCardNtf, &RoomMgr::PutLandlordCard);
    RegEvent(this, kEventPlayError, &RoomMgr::PlayError);
    RegEvent(this, kEventPlaySuccess, &RoomMgr::PlaySuccess);
    RegEvent(this, kEventGameOver, &RoomMgr::GameOver);
    return true;
}

bool RoomMgr::Start()
{
    int32_t rid = CreateRoom(1, "test1", nullptr);
    auto pRoom = room_map_[rid];
    EnterRoom(pRoom, 2, "test2", nullptr);
    EnterRoom(pRoom, 3, "test3", nullptr);
    pRoom->CheckBegin();
}

void RoomMgr::Stop()
{
    for(auto & it : room_map_) {
        RoomFactory::Instance().Release(it.second);
    }
    room_map_.clear();
}

int32_t RoomMgr::CreateRoom(int32_t uid, const std::string & room_name, ISender * sender)
{
    CardRoom * pRoom = RoomFactory::Instance().Create();
    int8_t index = pRoom->SetPlayer(uid);
    pRoom->SetOwner(index);

    UserInfo user_info;
    user_info.uid = uid;
    user_info.rid = pRoom->GetId();
    user_info.account = room_name;
    user_info.sender = sender;
    user_map_[uid] = user_info;

    room_map_[pRoom->GetId()] = pRoom;
    roomuser_index_[pRoom->GetId()][uid] = index;

    return pRoom->GetId();
}
void RoomMgr::CreateRoom(ISender * sender, room::CreateRoomReq & req)
{
    CreateRoomResp rsp;

    if(CheckIsInRoom(req.id())) {
        rsp.set_result(PLAYER_HAS_IN_ROOM);
        sender->Send(kS2CCreateRoom, rsp);
        return;
    }

    int32_t rid = CreateRoom(req.id(), req.account(), sender);

    rsp.set_result(SUCCESS);
    rsp.set_rid(rid);
    rsp.set_index(roomuser_index_[rid][req.id()]);
    sender->Send(kS2CCreateRoom, rsp);
}

int32_t RoomMgr::EnterRoom(CardRoom * pRoom, int32_t uid, const std::string & user_name, ISender * sender)
{
    int8_t index = pRoom->SetPlayer(uid);
    // add user map
    UserInfo user_info;
    user_info.uid = uid;
    user_info.rid = pRoom->GetId();
    user_info.account = user_name;
    user_info.sender = sender;
    user_map_[uid] = user_info;

    roomuser_index_[pRoom->GetId()][uid] = index;
    return index;
}
void RoomMgr::EnterRoom(ISender * sender, room::EnterRoomReq & req)
{
    EnterRoomResp rsp;

    // check in room
    if(CheckIsInRoom(req.uid())) {
        rsp.set_result(PLAYER_HAS_IN_ROOM);
        sender->Send(kS2CEnterRoom, rsp);
        return;
    }

    auto find = room_map_.find(req.rid());
    // check has room
    if(find == room_map_.end()) {
        rsp.set_result(ROOM_NOT_EXIST);
        sender->Send(kS2CEnterRoom, rsp);
        return;
    }
    auto pRoom = find->second;

    // check is full
    if(pRoom->GetCount() == 3) {
        rsp.set_result(ROOM_PLAYER_FULL);
        sender->Send(kS2CEnterRoom, rsp);
        return;
    }

    // check room state
    if(pRoom->GetState() != kRoomWaiting) {
        rsp.set_result(ROOM_ENTER_FAIL);
        rsp.set_desc("room is not in waiting");
        sender->Send(kS2CEnterRoom, rsp);
        return;
    }

    int8_t index = EnterRoom(pRoom, req.uid(), req.account(), sender);

    rsp.set_result(SUCCESS);
    rsp.set_rid(pRoom->GetId());
    rsp.set_index(index);
    rsp.set_ready(pRoom->GetIsReady(index));
    sender->Send(kS2CEnterRoom, rsp);

    NtfEnterRoom(req.uid(), pRoom->GetId(), index);
    pRoom->CheckBegin();
}

void RoomMgr::LeaveRoom(ISender * sender, room::LeaveRoomReq & req)
{
    LeaveRoomResp rsp;
    int32_t uid = req.uid();
    int32_t rid = req.rid();

    if(!CheckIsInRoom(uid)) {
        rsp.set_result(PLAYER_NOT_IN_ROOM);
        sender->Send(kS2CLeaveRoom, rsp);
        return;
    }

    auto room_it = room_map_.find(rid);
    if(room_it == room_map_.end()) {
        rsp.set_result(ROOM_NOT_EXIST);
        sender->Send(kS2CLeaveRoom, rsp);
        return;
    }

    int8_t index = roomuser_index_[rid][uid];
    auto pRoom = room_it->second;

    if(pRoom->GetState() != kRoomWaiting) {
        rsp.set_result(ROOM_HAS_START);
        sender->Send(kS2CLeaveRoom, rsp);
        return;
    }

    pRoom->LeavePlayer(index);
    rsp.set_result(SUCCESS);
    sender->Send(kS2CLeaveRoom, rsp);
    NtfLeaveRoom(uid, rid, index);

    user_map_.erase(uid);
    roomuser_index_[rid].erase(uid);
    if(pRoom->GetCount() == 0) {
        logger_debug("close room: {}", rid);
        RoomFactory::Instance().Release(pRoom);
        room_map_.erase(rid);
        roomuser_index_.erase(rid);
    }
}

void RoomMgr::GetRoom(ISender * sender, room::GetRoomReq & req)
{
    GetRoomResp rsp;

    for(auto it : room_map_) {
        auto pRoom = rsp.add_rooms();
        pRoom->set_id(it.second->GetId());
        pRoom->set_name(it.second->GetName());
        pRoom->set_count(it.second->GetCount());
        switch(it.second->GetState()) {
        case kRoomWaiting:
        case kRoomEnd:
            pRoom->set_state(RoomInfo::WAIT);
            break;
        case kRoomPlaying:
            pRoom->set_state(RoomInfo::START);
            break;
        }
    }

    sender->Send(kS2CGetRoom, rsp);
}

void RoomMgr::Ready(ISender * sender, room::ReadyReq & req)
{
    ReadyResp rsp;
    int32_t uid = req.uid();
    int32_t rid = req.rid();
    bool is_ready = req.ready();

    if(!CheckIsInRoom(uid)) {
        rsp.set_result(PLAYER_NOT_IN_ROOM);
        sender->Send(kS2CReady, rsp);
        return;
    }

    auto room_it = room_map_.find(rid);
    if(room_it == room_map_.end()) {
        rsp.set_result(ROOM_NOT_EXIST);
        sender->Send(kS2CReady, rsp);
        return;
    }

    auto pRoom = room_it->second;
    int8_t index = roomuser_index_[rid][uid];

    if(!pRoom->Ready(index, is_ready)) {
        rsp.set_result(is_ready ? ERROR_HAS_READY : ERROR_NOT_READY);
        sender->Send(kS2CReady, rsp);
        return;
    }

    rsp.set_result(SUCCESS);
    sender->Send(kS2CReady, rsp);

    NtfReady(uid, rid, index, is_ready);
}

bool RoomMgr::CheckIsInRoom(int32_t uid)
{
    auto find = user_map_.find(uid);
    return find != user_map_.end();
}

void RoomMgr::NtfEnterRoom(int32_t uid, int32_t rid, int8_t index)
{
    if(room_map_.find(rid) == room_map_.end()) {
        logger_error("can't found room: {}", rid);
        return;
    }
    auto pRoom = room_map_[rid];
    if(pRoom == nullptr) {
        return;
    }
    EnterRoomNtf ntf;
    ntf.set_rid(rid);
    ntf.set_uid(uid);
    ntf.set_index(index);
    ntf.set_ready(pRoom->GetIsReady(index));

    for(auto id : roomuser_index_[rid]) {
        if(id.first != uid) {
            auto & sender = user_map_[id.first].sender;
            if(sender != nullptr) {
                logger_debug("ntf {} player {} enter room", id.first, uid);
                sender->Send(kS2CEnterRoomNtf, ntf);
            }
        }
    }
}

void RoomMgr::NtfLeaveRoom(int32_t uid, int32_t rid, int8_t index)
{
    if(room_map_.find(rid) == room_map_.end()) {
        logger_error("can't found room: {}", rid);
        return;
    }
    auto pRoom = room_map_[rid];
    if(pRoom == nullptr) {
        return;
    }
    LeaveRoomNtf ntf;
    ntf.set_rid(rid);
    ntf.set_uid(uid);
    ntf.set_index(index);

    for(auto id : roomuser_index_[rid]) {
        if(id.first != uid) {
            auto & sender = user_map_[id.first].sender;
            if(sender != nullptr) {
                sender->Send(kS2CLeaveRoomNtf, ntf);
            }
        }
    }
}

void RoomMgr::NtfReady(int32_t uid, int32_t rid, int8_t index, bool ready)
{
    if(room_map_.find(rid) == room_map_.end()) {
        logger_error("can't found room: {}", rid);
        return;
    }
    auto pRoom = room_map_[rid];
    if(pRoom == nullptr) {
        return;
    }
    ReadyNtf ntf;
    ntf.set_rid(rid);
    ntf.set_uid(uid);
    ntf.set_index(index);
    ntf.set_ready(ready);

    for(auto id : roomuser_index_[rid]) {
        if(id.first != uid) {
            auto & sender = user_map_[id.first].sender;
            if(sender != nullptr) {
                sender->Send(kS2CReadyNtf, ntf);
            }
        }
    }
}

void RoomMgr::GameBegin(proto::EventBuff & buff)
{
    int32_t rid = 0;
    buff >> rid;

    auto pRoom = room_map_[rid];
    if(!pRoom) {
        logger_error("room {} null", rid);
        return;
    }
    int8_t landlord_index = pRoom->BeginGame();
    for(auto & seat : pRoom->GetAllSeats()) {
        auto uid = seat.GetUID();
        auto sender = user_map_[uid].sender;
        if(sender == nullptr) {
            continue;
        }

        // send room begin
        GameBeginNtf gb;
        gb.set_rid(rid);
        gb.set_first(landlord_index);
        sender->Send(kS2CGameBeginNtf, gb);

        // send card
        PutCardNtf putcard;
        putcard.set_uid(uid);
        for(auto & card :seat.GetCards()) {
            putcard.add_cid(card.id);
        }
        sender->Send(kS2CPutCardNtf, putcard);
    }
}

void RoomMgr::GameOver(proto::EventBuff & buff)
{
    int32_t rid = 0;
    int8_t winner_index = 0;
    int8_t landlord_index = 0;

    buff >> rid;
    buff >> winner_index;
    buff >> landlord_index;

    auto find = room_map_.find(rid);
    if(find == room_map_.end()) {
        logger_error("can not find room {}", rid);
        return;
    }

    auto room = find->second;
    if(room->GetState() != RoomState::kRoomEnd) {
        logger_error("room state is't end!");
        return;
    }
    room->ResetRoom();

    GameOverNtf ntf;
    ntf.set_rid(rid);
    ntf.set_winner(winner_index);
    ntf.set_landlord(landlord_index);
    for(auto id : roomuser_index_[rid]) {
        auto & sender = user_map_[id.first].sender;
        if(sender != nullptr) {
            sender->Send(kS2CGameOverNtf, ntf);
        }
    }

    for(auto & seat : room->GetAllSeats()) {
        seat.SetReady(true);
    }
    room->CheckBegin();
}

void RoomMgr::OnLandlord(ISender * sender, room::LandlordReq & req)
{
    int32_t uid = req.uid();
    int32_t rid = req.rid();
    bool call = req.call();
    LandlordResp rsp;
    if(!CheckIsInRoom(uid)) {
        rsp.set_result(PLAYER_NOT_IN_ROOM);
        sender->Send(kS2CLandlord, rsp);
        return;
    }

    auto room_it = room_map_.find(rid);
    if(room_it == room_map_.end()) {
        rsp.set_result(ROOM_NOT_EXIST);
        sender->Send(kS2CLandlord, rsp);
        return;
    }

    auto & pRoom = room_it->second;
    int8_t index = roomuser_index_[rid][uid];
    if(pRoom->GetCurrentIndex() != index) {
        rsp.set_result(NOT_TURN_YOU);
        sender->Send(kS2CLandlord, rsp);
        return;
    }

    pRoom->DealLandlord(call);
    rsp.set_result(SUCCESS);
    sender->Send(kS2CLandlord, rsp);
}

void RoomMgr::CallLandlordNtf(proto::EventBuff & buff)
{
    int32_t rid = 0;
    int8_t cur = 0;
    int8_t next = 0;
    bool call = false;
    buff >> rid;
    buff >> cur;
    buff >> next;
    buff >> call;

    LandlordNtf ntf;
    ntf.set_cindex(cur);
    ntf.set_nindex(next);
    ntf.set_call(call);

    for(auto id : roomuser_index_[rid]) {
        auto & sender = user_map_[id.first].sender;
        if(sender != nullptr) {
            sender->Send(kS2CLandlordNtf, ntf);
        }
    }
}

void RoomMgr::PutLandlordCard(proto::EventBuff & buff)
{
    PutLandlordCardNtf ntf;
    int32_t rid = 0;
    int8_t landlord_index = 0;
    int8_t landlord_card = 0;

    buff >> rid;
    buff >> landlord_index;
    ntf.set_landlord(landlord_index);
    for(int i = 0; i < 3; i++) {
        buff >> landlord_card;
        ntf.add_cid(landlord_card);
    }

    if(roomuser_index_.find(rid) == roomuser_index_.end()) {
        logger_error("can't find room {}", rid);
        return;
    }
    for(auto id : roomuser_index_[rid]) {
        auto & sender = user_map_[id.first].sender;
        if(sender != nullptr) {
            sender->Send(kS2CLandlordNtf, ntf);
        }
    }
}

void RoomMgr::OnPlay(ISender * sender, room::PlayReq & req)
{

}

void RoomMgr::PlayError(proto::EventBuff & buff)
{
    int32_t uid = 0;
    int8_t ret;

    buff >> ret;

    auto uinfo = user_map_.find(uid);
    if(uinfo == user_map_.end()) {
        logger_error("can't not find user {}", uid);
        return;
    }

    auto sender = uinfo->second.sender;
    if(sender == nullptr) {
        return;
    }

    PlayResp rsp;
    switch((CombRet)ret)
    {
    case kCombFail:
        rsp.set_result(COMB_FAIL);
        break;
    case kCombMustPlay:
        rsp.set_result(MUST_PLAY);
        break;
    case kCombChooseError:
        rsp.set_result(CHOOSE_ERROR);
        break;
    default:
        return;
    }
    sender->Send(kS2CPlay, rsp);
}

void RoomMgr::PlaySuccess(proto::EventBuff & buff)
{
    int32_t rid = 0;
    int8_t current_index = 0;
    int8_t next_index = 0;
    int16_t size = 0;

    buff >> rid;
    buff >> current_index;
    buff >> next_index;
    buff >> size;

    auto find = room_map_.find(rid);
    if(find == room_map_.end()) {
        logger_error("can not find room {}", rid);
        return;
    }
    auto room = find->second;

    PlayResp rsp;
    rsp.set_result(SUCCESS);
    rsp.set_next(room->GetCurrentIndex());

    // ntf other
    PlayNtf ntf;
    ntf.set_current(current_index);
    ntf.set_next(next_index);
    int8_t cid = 0;
    for(int i = 0; i < size; i++) {
        buff >> cid;
        ntf.add_cid(cid);
    }

    for(auto id : roomuser_index_[rid]) {
        auto & sender = user_map_[id.first].sender;
        if(sender != nullptr) {
            if(id.second == current_index) {
                sender->Send(kS2CPlay, rsp);
            }
            sender->Send(kS2CPlayNtf, ntf);
        }
    }
}
