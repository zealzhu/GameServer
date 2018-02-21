#ifndef _SEAT_H
#define _SEAT_H

#include "card.hpp"
#include "poker_utils.h"
#include <list>
#include <map>

class Seat
{
public:
    Seat()
        : empty_(true), ready_(false), index_(0),
        uid_(0), has_qiang_(false), has_call_(false), landlord_(false)
    {}

    void SetIndex(int8_t index) { index_ = index; }
    int8_t GetIndex() { return index_; }
    void SetEmpty(bool is_empty) { empty_ = is_empty; }
    bool IsEmpty() { return empty_; }
    void SetUID(int32_t uid) { uid_ = uid; }
    int32_t GetUID() { return uid_; }
    void SetReady(bool is_ready) { ready_ = is_ready; }
    bool IsReady() { return ready_; }
    std::list< Card > & GetCards() { return cards_; }
    std::map< int8_t, Card > & GetCardsMap() { return cards_map_; }
    void SetHasQiang(bool call) { has_qiang_ = call; }
    bool GetHasQiang() { return has_qiang_; }
    void SetHasCall(bool call) { has_call_ = call; }
    bool GetHasCall() { return has_call_; }
    bool GetIsLandlord() { return landlord_; }
    bool SetIsLandlord(bool call) { landlord_ = call; }

    void Sort();
    void AutoPlay(std::vector< Card > & out_cards, CombType & last_out);
    void RemoveCards(std::vector< Card > & cards);
    bool FindCards(std::vector< Card > & cards);

private:
    bool empty_;
    bool ready_;
    bool has_qiang_;
    bool has_call_;
    bool landlord_;
    int8_t index_;
    int32_t uid_;

    std::list< Card > cards_;
    std::map< int8_t, Card > cards_map_;
};

#endif // _SEAT_H
