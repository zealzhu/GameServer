#ifndef _ROOM_H
#define _ROOM_H

#include "seat.h"
#include <timer_processor.hpp>

#include <string>
#include <vector>
#include <list>

#define LANDLORD_EXPIRE 100
#define PLAY_EXPIRE 100

enum RoomState
{
    kRoomWaiting,
    kRoomCallLandlord,
    kRoomQiangLandlord,
    kRoomPlaying,
    kRoomEnd,
};

enum LandlordRet
{
    kLandlordNoOneCall,
    kLandlordWaitNextCall,
    kLandlordWaitNextQiang,
    kLandlordBeginPlay,
};

class CardRoom : public ITimer
{
public:
    virtual void OnTime();

    void Initialize();
    int8_t SetPlayer(int32_t uid);
    int8_t GetCount();
    void LeavePlayer(int8_t index);
    bool Ready(int8_t index, bool ready);

    bool CheckBegin();
    int8_t BeginGame();
    void Shuffle();
    void PutCard();
    void GameOver();
    void ResetRoom();

    void LandlordNtf(int8_t cur, int8_t next, bool call);
    void PutLandlordCard();

    void DealLandlord(bool call);
    void DealPlay();
    LandlordRet LoopJudgeLandlord();
    void LoopJudgyPlay();

    void SetId(int32_t id) { id_ = id; }
    int32_t GetId() { return id_; }
    void SetOwner(int8_t index) { owner_ = index; }
    int8_t GetOwner() { return owner_; }
    int8_t GetCurrentIndex() { return current_index_; }
    RoomState GetState() { return state_; }
    std::string GetName() { return name_; }
    std::vector< Card > & GetLandlordCard() { return landlord_cards_; }

    bool GetIsReady(int8_t index) { return seats_[index].IsEmpty() ? false : seats_[index].IsReady(); }
    std::vector< Seat > & GetAllSeats() { return seats_; }

    void StopTimer() { if(GetNode() != nullptr) { STOP_TIMER(this); } }
    void OnCallLandlord();
    void OnLandlordExpire();
    void OnPlayingExpire();

private:
    bool CompareOut(std::vector< Card > & cards);
    PlayType GetCardsType(std::vector< Card > & cards);

    void SendPlayError(CombRet ret);
    void SendPlaySuccess(std::vector< Card > & cards);

    std::vector< Seat > seats_;
    std::vector< Card > all_cards_;
    std::vector< Card > landlord_cards_;

    CombType last_out_;

    int8_t current_index_;
    int8_t landlord_index_;
    int8_t no_play_count_;

    int32_t owner_;
    int32_t id_;
    std::string name_;
    RoomState state_;
};

class RoomFactory
{
public:
    static RoomFactory & Instance() {
        static RoomFactory instance;
        return instance;
    }

    CardRoom * Create();
    void Release(CardRoom * room);

private:
    RoomFactory() = default;
    ~RoomFactory() = default;

    int32_t GenerateId();

    int32_t increase_id_;
    std::list< int32_t > old_ids_;
};

#endif // _ROOM_H
