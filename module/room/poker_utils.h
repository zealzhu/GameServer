#ifndef _POKER_UTILS_H
#define _POKER_UTILS_H

#include "card.hpp"
#include <vector>

class PokerUtils {
public:
    static bool IsJokerBoom(std::vector< Card > & cards);

    static bool IsDoubleSame(std::vector< Card > & cards);

    static bool IsThreeSame(std::vector< Card > & cards);

    static bool IsBoom(std::vector< Card > & cards);

    // 三带1 4
    static bool IsThreeWithOne(std::vector< Card > & cards);

    // 三带2 5
    static bool IsThreeWithTwo(std::vector< Card > & cards);

    // 顺子 5-12
    static bool IsSingleStraight(std::vector< Card > & cards);

    // 连对
    static bool IsDoubleStraight(std::vector< Card > & cards);

    // 炸弹带单牌
    static bool IsBoomWithSingle(std::vector< Card > & cards);

    // 炸弹带对子
    static bool IsBoomWithDouble(std::vector< Card > & cards);

    // 两飞机不带
    static bool IsTwoPlane(std::vector< Card > & cards);

    // 三飞机不带 9
    static bool IsThreePlane(std::vector< Card > & cards);

    // 四飞机不带 12
    static bool IsFourPlane(std::vector< Card > & cards);

    // 五飞机不带 15
    static bool IsFivePlane(std::vector< Card > & cards);

    // 两飞机带单 8
    static bool IsTwoPlaneWithSingle(std::vector< Card > & cards);

    // 两飞机带对 10
    static bool IsTwoPlaneWithDouble(std::vector< Card > & cards);

    // 三飞机带单 12
    static bool IsThreePlaneWithSingle(std::vector< Card > & cards);

    // 三飞机带对 15
    static bool IsThreePlaneWithDouble(std::vector< Card > & cards);

    // 四飞机带单 16
    static bool IsFourPlaneWithSingle(std::vector< Card > & cards);

    // 四飞机带双 20
    static bool IsFourPlaneWithDouble(std::vector< Card > & cards);

    // 五飞机带单 20
    static bool IsFivePlaneWithSingle(std::vector< Card > & cards);
};

#endif
