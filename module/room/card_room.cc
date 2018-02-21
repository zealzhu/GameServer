#include "card_room.h"
#include "event_processor.hpp"
#include <game_log.h>
#include <algorithm>
#include <stdlib.h>
#include <sstream>

#if 0 // test
int8_t rule_index = 0;
int8_t current_rule = 0;
bool rule[][4] = {
    {true, true, true, true},
    {true, true, true, false},
    {true, false, false, false},
    {true, true, false, true},
    {true, true, false, false},
    {true, false, true, true},
    {true, false, true, false},
    {false, false, false, true},
    {false, true, false, true},
    {false, true, true, true},
    {false, true, true, false},
    {false, false, true, false}
};
#endif

void CardRoom::Initialize()
{
    for(int i = 0; i < 3; i++) {
        seats_.push_back(Seat());
        seats_[i].SetIndex(i);
        seats_[i].SetEmpty(true);
    }

    for(int i = 0; i < 54; i++) {
        all_cards_.emplace_back(Card(i));
    }

    state_ = kRoomWaiting;
    name_ = "";
    id_ = 0;
    owner_ = 0;
    current_index_ = 0;
}

void CardRoom::DealLandlord(bool call)
{
    seats_[current_index_].SetIsLandlord(call);
    if(state_ == kRoomCallLandlord) {
        seats_[current_index_].SetHasCall(true);
        seats_[current_index_].SetHasQiang(!call);
    }
    else if(state_ == kRoomQiangLandlord) {
        seats_[current_index_].SetHasQiang(true);
    }
    else {
        logger_error("error state.");
        return;
    }

    int8_t cur = current_index_;
    LandlordRet ret = LoopJudgeLandlord();
    LandlordNtf(cur, current_index_, call);
    StopTimer();

    switch(ret) {
    case kLandlordBeginPlay:
        logger_debug("{} begin play", current_index_);
        PutLandlordCard();
        break;
    case kLandlordWaitNextQiang:
        logger_debug("{} call: {}  wait {} qiang", cur, call, current_index_);
        START_TIMER(this, 0, 1, LANDLORD_EXPIRE);
        break;
    case kLandlordWaitNextCall:
        logger_debug("{} call: {}  wait {} call", cur, call, current_index_);
        START_TIMER(this, 0, 1, LANDLORD_EXPIRE);
        break;
    case kLandlordNoOneCall:
        logger_debug("no one call. restart");
        BeginGame();
        break;
    }
}

void CardRoom::LandlordNtf(int8_t cur, int8_t next, bool call)
{
    logger_debug("player {} call: {}, next: {}", cur, call, next);
    auto buff = new proto::EventBuff(kEventCallLandlordNtf);
    buff->Append(id_);
    buff->Append(cur);
    buff->Append(next);
    buff->Append(call);
    ExecEvent(buff);
}

LandlordRet CardRoom::LoopJudgeLandlord()
{
    int8_t pre_index = (current_index_ + 2) % 3;/*{{{*/
    int8_t next_index = (current_index_ + 1) % 3;
    auto pre_seat = seats_[pre_index];
    auto cur_seat = seats_[current_index_];
    auto next_seat = seats_[next_index];

    switch(state_) {
    case kRoomCallLandlord:
        // call
        if(cur_seat.GetIsLandlord()) {
            // begin
            if(next_seat.GetHasCall()) {
                landlord_index_ = current_index_;
                state_ = kRoomPlaying;
                return kLandlordBeginPlay;
            }
            // qiang
            else {
                landlord_index_ = current_index_;
                current_index_ = next_index;
                state_ = kRoomQiangLandlord;
                return kLandlordWaitNextQiang;
            }
        }
        // not call
        else {
            current_index_ = next_index;
            // restart
            if(next_seat.GetHasCall() == true) {
                return kLandlordNoOneCall;
            }
            return kLandlordWaitNextCall;
        }
        break;
    case kRoomQiangLandlord:
        // call
        if(cur_seat.GetIsLandlord()) {
            // begin
            if(current_index_ == landlord_index_) {
                state_ = kRoomPlaying;
                return kLandlordBeginPlay;
            }

            if(next_seat.GetHasQiang()) {
                current_index_ = next_index;
                return LoopJudgeLandlord();
            }
            else {
                current_index_ = next_index;
                return kLandlordWaitNextQiang;
            }
        }
        // not call
        else {
            // oxx
            if(landlord_index_ == next_index && !pre_seat.GetIsLandlord()) {
                landlord_index_ = current_index_ = next_index;
                state_ = kRoomPlaying;
                return kLandlordBeginPlay;
            }

            // ooox
            if(landlord_index_ == current_index_ && pre_seat.GetIsLandlord()) {
                landlord_index_ = current_index_ = pre_index;
                state_ = kRoomPlaying;
                return kLandlordBeginPlay;
            }

            // next can call
            if(!next_seat.GetHasQiang()) {
                current_index_ = next_index;
                return kLandlordWaitNextQiang;
            }
            else {
                if(next_seat.GetIsLandlord())
                {
                    landlord_index_ = current_index_ = next_index;
                }
                else
                {
                    landlord_index_ = current_index_ = (next_index + 1) % 3;
                }

                state_ = kRoomPlaying;
                return kLandlordBeginPlay;
            }
        }
        break;
    default:
        break;
    }/*}}}*/
}

void CardRoom::OnTime()
{
    SetNode(nullptr);
    logger_debug("time out.");

    switch(state_) {
    case kRoomCallLandlord:
    case kRoomQiangLandlord:
        OnLandlordExpire();
        break;
    case kRoomPlaying:
        OnPlayingExpire();
        break;
    case kRoomEnd:
        break;
    default:
        break;
    }
}
void CardRoom::OnLandlordExpire()
{
    bool call = rand() % 2;
    DealLandlord(call);
}

void CardRoom::OnPlayingExpire()
{
    std::vector< Card > out_cards;
    seats_[current_index_].AutoPlay(out_cards, last_out_);
    if(CompareOut(out_cards) == false) {
        logger_error("auto play choose error!");
        return;
    }
    if(state_ == kRoomPlaying) {
        logger_debug("wait {} play", current_index_);
        START_TIMER(this, 0, 1, PLAY_EXPIRE);
    }
}

int8_t CardRoom::SetPlayer(int32_t uid)
{
    int8_t index = -1;
    for(auto & seat : seats_) {
        index++;
        if(seat.IsEmpty()) {
            seat.SetEmpty(false);
            seat.SetUID(uid);
            seat.SetReady(true);
            logger_debug("user {} in seat {}", uid, index);
            return index;
        }
    }
}

void CardRoom::LeavePlayer(int8_t index)
{
    logger_debug("seat {} leave", index);
    seats_[index].SetEmpty(true);
    seats_[index].SetUID(0);
    seats_[index].SetReady(false);
}

bool CardRoom::CheckBegin()
{
    for(auto & seat : seats_) {
        if(seat.IsEmpty() || !seat.IsReady()) {
            return false;
        }
    }

    auto buff = new proto::EventBuff(kEventGameBegin);
    buff->Append(id_);
    ExecEvent(buff);
    state_ = kRoomCallLandlord;
    return true;
}

bool CardRoom::Ready(int8_t index, bool ready)
{
    if(seats_[index].IsReady() == ready)
    {
        return false;
    }
    seats_[index].SetReady(ready);

    // check can begin game
    if(ready)
    {
        CheckBegin();
    }

    return true;
}

int8_t CardRoom::BeginGame()
{
    for(auto & seat : seats_) {
        seat.GetCardsMap().clear();
        seat.GetCards().clear();
        seat.SetHasQiang(false);
        seat.SetHasCall(false);
        seat.SetIsLandlord(false);
    }
    Shuffle();
    PutCard();

    state_ = kRoomCallLandlord;
    current_index_ = landlord_index_ = rand() % 3;
    no_play_count_ = 0;
    last_out_.type = kNoPlay;

    logger_debug("game begin. start timer.");
    START_TIMER(this, 0, 1, LANDLORD_EXPIRE);
    return landlord_index_;
}

void CardRoom::PutCard()
{
    // put card
    for(int i = 0; i < 51;) {
        for(auto & seat : seats_) {
            seat.GetCardsMap()[all_cards_[i].id] = all_cards_[i];
            seat.GetCards().push_back(all_cards_[i++]);
        }
    }
    // sort
    for(auto & seat : seats_) {
        seat.Sort();
    }
    // landlord card
    landlord_cards_.clear();
    for(int i = 51; i < 54; i++) {
        landlord_cards_.push_back(all_cards_[i]);
    }
}

void CardRoom::PutLandlordCard()
{
    auto buff = new proto::EventBuff(kEventPutLandlordCardNtf);
    buff->Append(id_);
    buff->Append(landlord_index_);
    logger_debug("put landlord card size: {}", landlord_cards_.size());
    for(auto & card : landlord_cards_)
    {
        buff->Append(card.id);
    }
    ExecEvent(buff);
    START_TIMER(this, 0, 1, PLAY_EXPIRE);
}

void CardRoom::Shuffle()
{
    srand(time(nullptr));
    std::random_shuffle(all_cards_.begin(), all_cards_.end());
}

void CardRoom::GameOver()
{
    state_ = kRoomEnd;
    logger_debug("game over. the winner index is {}", current_index_);

    auto buff = new proto::EventBuff(kEventGameOver);
    buff->Append(id_);
    buff->Append(current_index_);  // winner
    buff->Append(landlord_index_); // landlord
    ExecEvent(buff);
}

void CardRoom::ResetRoom()
{
    state_ = kRoomWaiting;
    for(auto & seat : seats_) {
        seat.SetReady(false);
    }
}

void CardRoom::SendPlayError(CombRet ret)
{
    auto buff = new proto::EventBuff(kEventPlayError);
    buff->Append(id_);
    buff->Append(seats_[current_index_].GetUID());
    buff->Append((int8_t)ret);
    ExecEvent(buff);
}

void CardRoom::SendPlaySuccess(std::vector< Card > & cards)
{
    auto buff = new proto::EventBuff(kEventPlaySuccess);
    buff->Append((int32_t)id_);
    buff->Append((int8_t)current_index_);
    buff->Append((int8_t)(current_index_ + 1) % 3);
    buff->Append((int16_t)cards.size());
    std::ostringstream os;
    os << "out grade: ";
    for(auto & card : cards)
    {
        buff->Append(card.id);
        os << (int32_t)card.grade << "";
    }
    seats_[current_index_].RemoveCards(cards);
    logger_debug("{} out grade: {}. current size: {}", current_index_, os.str(), seats_[current_index_].GetCards().size());
    ExecEvent(buff);
}

int8_t CardRoom::GetCount()
{
    int8_t count = 0;
    for(auto & seat : seats_) {
        if(!seat.IsEmpty()) {
            count++;
        }
    }
    return count;
}

bool CardRoom::CompareOut(std::vector< Card > & cards)
{
    // not have cards{{{
    if(!seats_[current_index_].FindCards(cards)) {
        SendPlayError(kCombChooseError);
        return false;
    }

    std::sort(cards.begin(), cards.end(), [](const Card & f, const Card & s) { return f.grade > s.grade; });
    auto type = GetCardsType(cards);
    // choose error
    if(type == kPlayError) {
        SendPlayError(kCombChooseError);
        return false;
    }

    // no play
    if(type == kNoPlay) {
        // must play
        if(no_play_count_ == 2) {
            SendPlayError(kCombMustPlay);
            return false;
        }
        // second no play
        else if(no_play_count_ == 1) {
            last_out_.type = type;
            last_out_.play_index = current_index_;
            last_out_.cards.clear();
        }
        // no play
        SendPlaySuccess(cards);
        no_play_count_++;
        current_index_ = (current_index_ + 1) % 3;
        return true;
    }
    // last is double joker can not big more
    else if(last_out_.type == kPlayDoubleJacker) {
        if(type != kNoPlay) {
            SendPlayError(kCombFail);
            return false;
        }
    }

    // last is self or no play
    if(last_out_.play_index == current_index_ || last_out_.type == kNoPlay) {
        SendPlaySuccess(cards);
    }
    else {
        // joker bomb
        if(type == kPlayDoubleJacker) {
            SendPlaySuccess(cards);
        }
        // bomb
        else if(type == kPlayBomb) {
            if(last_out_.type == kPlayBomb) {
                if (last_out_.cards[0].grade > cards[0].grade) {
                    SendPlayError(kCombFail);
                    return false;
                }
            }
            SendPlaySuccess(cards);
        }
        // other type
        else if(type == last_out_.type) {
            size_t belt_number = 0;
            switch(type) {
            case kPlaySingle:
            case kPlayDoubleSame:
            case kPlayThreeSame:
            case kPlayDoubleThreeSame:
            case kPlayThreeThreeSame:
            case kPlayFourThreeSame:
            case kPlayFiveThreeSame:
            case kPlaySingleStraight:
            case kPlayDoubleStraight:
                // 这些排型只用比较第一张
                if (last_out_.cards[0].grade > cards[0].grade) {
                    SendPlayError(kCombFail);
                    return false;
                }
                SendPlaySuccess(cards);
                break;
            case kPlayThreeSameWithOne:
            case kPlayThreeSameWithTwo:
            case kPlayBombWithSingle:
            case kPlayDoubleThreeSameWithSingle:
            case kPlayThreeThreeSameWithSingle:
                // 炸弹带单、双飞机带单、三飞机带单、三带一和三带二比较第四张牌大小
                if (last_out_.cards[3].grade > cards[3].grade) {
                    SendPlayError(kCombFail);
                    return false;
                }
                SendPlaySuccess(cards);
                break;
            case kPlayDoubleThreeSameWithDouble:
            case kPlayFourThreeSameWithSingle:
                // 双飞机带双和四飞机带单比第五张
                if (last_out_.cards[4].grade > cards[4].grade) {
                    SendPlayError(kCombFail);
                    return false;
                }
                SendPlaySuccess(cards);
                break;
            case kPlayThreeThreeSameWithDouble:
                // 三飞机带双比6
                if (last_out_.cards[6].grade > cards[6].grade) {
                    SendPlayError(kCombFail);
                    return false;
                }
                SendPlaySuccess(cards);
                break;
            case kPlayFourThreeSameWithDouble:
            case kPlayFiveThreeSameWithSingle:
                if (last_out_.cards[12].grade > cards[12].grade) {
                    SendPlayError(kCombFail);
                    return false;
                }
                SendPlaySuccess(cards);
                break;
            case kPlayBombWithDouble:
                if(cards[0].grade == cards[2].grade) {
                    if(last_out_.cards[0].grade == last_out_.cards[2].grade) {
                        if (last_out_.cards[0].grade > cards[0].grade) {
                            SendPlayError(kCombFail);
                            return false;
                        }
                    }
                    else {
                        if (last_out_.cards[7].grade > cards[0].grade) {
                            SendPlayError(kCombFail);
                            return false;
                        }
                    }
                }
                else {
                    if(last_out_.cards[0].grade == last_out_.cards[2].grade) {
                        if (last_out_.cards[0].grade > cards[7].grade) {
                            SendPlayError(kCombFail);
                            return false;
                        }
                    }
                    else {
                        if (last_out_.cards[7].grade > cards[7].grade) {
                            SendPlayError(kCombFail);
                            return false;
                        }
                    }
                }
                SendPlaySuccess(cards);
                break;
            default:
                logger_error("unknown type");
                return false;
            }
        }
        // type not match
        else {
            SendPlayError(kCombFail);
            return false;
        }
    }

    last_out_.type = type;
    last_out_.play_index = current_index_;
    last_out_.cards.swap(cards);
    no_play_count_ = 0;
    // is over
    if(seats_[current_index_].GetCards().size() == 0) {
        GameOver();
    }
    else {
        current_index_ = (current_index_ + 1) % 3;
    }
    return true;/*}}}*/
}

PlayType CardRoom::GetCardsType(std::vector< Card > & cards)
{
    PlayType type;/*{{{*/
    switch(cards.size())
    {
    case 0:
        type = kNoPlay;
        break;
    case 1:
        type = kPlaySingle;
        break;
    case 2:
        if(PokerUtils::IsDoubleSame(cards)) type = kPlayDoubleSame;
        else if(PokerUtils::IsJokerBoom(cards)) type = kPlayDoubleJacker;
        else type = kPlayError;
        break;
    case 3:
        if(PokerUtils::IsThreeSame(cards)) type = kPlayThreeSame;
        else type = kPlayError;
        break;
    case 4:
        if(PokerUtils::IsThreeWithOne(cards)) type = kPlayThreeSameWithOne;
        else if(PokerUtils::IsBoom(cards)) type = kPlayBomb;
        else type = kPlayError;
        break;
    case 5:
        if(PokerUtils::IsThreeWithTwo(cards)) type = kPlayThreeSameWithTwo;
        else if(PokerUtils::IsSingleStraight(cards)) type = kPlaySingleStraight;
        else type = kPlayError;
        break;
    case 6:
        if(PokerUtils::IsBoomWithSingle(cards)) type = kPlayBombWithSingle;
        else if(PokerUtils::IsTwoPlane(cards)) type = kPlayDoubleThreeSame;
        else if(PokerUtils::IsDoubleStraight(cards)) type = kPlayDoubleStraight;
        else if(PokerUtils::IsSingleStraight(cards)) type = kPlaySingleStraight;
        else type = kPlayError;
        break;
    case 7:
        if(PokerUtils::IsSingleStraight(cards)) type = kPlaySingleStraight;
        else type = kPlayError;
        break;
    case 8:
        if(PokerUtils::IsTwoPlaneWithSingle(cards)) type = kPlayDoubleThreeSameWithSingle;
        else if(PokerUtils::IsBoomWithDouble(cards)) type = kPlayBombWithDouble;
        else if(PokerUtils::IsDoubleStraight(cards)) type = kPlayDoubleStraight;
        else if(PokerUtils::IsSingleStraight(cards)) type = kPlaySingleStraight;
        else type = kPlayError;
        break;
    case 9:
        if(PokerUtils::IsSingleStraight(cards)) type = kPlaySingleStraight;
        else if(PokerUtils::IsThreePlane(cards)) type = kPlayThreeThreeSame;
        else type = kPlayError;
        break;
    case 10:
        if(PokerUtils::IsSingleStraight(cards)) type = kPlaySingleStraight;
        else if(PokerUtils::IsDoubleStraight(cards)) type = kPlayDoubleStraight;
        else type = kPlayError;
        break;
    case 11:
        if(PokerUtils::IsSingleStraight(cards)) type = kPlaySingleStraight;
        else type = kPlayError;
        break;
    case 12:
        if(PokerUtils::IsSingleStraight(cards)) type = kPlaySingleStraight;
        else if(PokerUtils::IsFourPlane(cards)) type = kPlayFourThreeSame;
        else if(PokerUtils::IsThreePlaneWithSingle(cards)) type = kPlayThreeThreeSameWithSingle;
        else if(PokerUtils::IsDoubleStraight(cards)) type = kPlayDoubleStraight;
        else type = kPlayError;
        break;
    case 14:
        if(PokerUtils::IsDoubleStraight(cards)) type = kPlayDoubleStraight;
        else type = kPlayError;
        break;
    case 15:
        if(PokerUtils::IsFivePlane(cards)) type = kPlayFiveThreeSame;
        else if(PokerUtils::IsThreePlaneWithDouble(cards)) type = kPlayThreeThreeSameWithDouble;
        else type = kPlayError;
        break;
    case 16:
        if(PokerUtils::IsDoubleStraight(cards)) type = kPlayDoubleStraight;
        else if(PokerUtils::IsFourPlaneWithSingle(cards)) type = kPlayFourThreeSameWithSingle;
        else type = kPlayError;
        break;
    case 20:
        if(PokerUtils::IsFivePlaneWithSingle(cards)) type = kPlayFiveThreeSameWithSingle;
        else if(PokerUtils::IsFourPlaneWithDouble(cards)) type = kPlayFourThreeSameWithDouble;
        else type = kPlayError;
        break;
    default:
        type = kPlayError;
        break;
    };
    return type;/*}}}*/
}

CardRoom * RoomFactory::Create()
{
    CardRoom * pRoom = new CardRoom;
    pRoom->Initialize();
    pRoom->SetId(GenerateId());

    return pRoom;
}

void RoomFactory::Release(CardRoom * pRoom)
{
    if(pRoom == nullptr) {
        return;
    }

    pRoom->StopTimer();
    old_ids_.emplace_back(pRoom->GetId());
    delete pRoom;
}

int32_t RoomFactory::GenerateId()
{
    int32_t id = 0;
    if(old_ids_.empty()) {
        id = ++increase_id_;
    }
    else {
        id = old_ids_.front();
        old_ids_.pop_front();
    }
    return id;
}

