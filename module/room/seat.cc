#include "seat.h"
#include <game_log.h>

bool Compare(const Card & first, const Card & second)
{
    return first.grade > second.grade;
}

void Seat::Sort()
{
    cards_.sort(Compare);
}

void Seat::AutoPlay(std::vector< Card > & out_cards, CombType & last_out)
{
    if(last_out.type == kNoPlay)
    {
        logger_debug("auto choose: {}", cards_.back().grade);
        out_cards.emplace_back(cards_.back());
    }
}

void Seat::RemoveCards(std::vector< Card > & cards)
{
    for(auto & card : cards)
    {
        cards_.remove_if([&](const Card & c) { return card.id == c.id; });
        cards_map_.erase(card.id);
    }
}

bool Seat::FindCards(std::vector< Card > & cards)
{
    for(auto & card : cards)
    {
        if(cards_map_.find(card.id) == cards_map_.end()) {
            return false;
        }
    }
    return true;
}
