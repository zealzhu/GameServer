#include "poker_utils.h"

using namespace std;

bool PokerUtils::IsJokerBoom(std::vector< Card > & cards)
{
    if(cards.size() != 2) return false;

    if(cards[0].grade == 15 && cards[1].grade == 14) return true;
    else if(cards[0].grade == 14 && cards[1].grade == 15) return true;

    return false;
}

bool PokerUtils::IsDoubleSame(std::vector< Card > & cards)
{
    if(cards.size() != 2) return false;

    return cards[0].grade == cards[1].grade;
}

bool PokerUtils::IsThreeSame(std::vector< Card > & cards)
{
    if(cards.size() != 3) return false;

    return cards[0].grade == cards[1].grade && cards[1].grade == cards[2].grade;
}

bool PokerUtils::IsBoom(std::vector< Card > & cards)
{
    if(cards.size() != 0) return false;

    for(int i = 0; i < 3; i++) {
        if(cards[i].grade != cards[i + 1].grade) return false;
    }
    return true;
}

bool PokerUtils::IsThreeWithOne(vector< Card >& cards) {
	if(cards.size() != 4) return false;
	if (cards[0].grade == cards[1].grade &&
		cards[0].grade == cards[2].grade)
		return true;
	else if (cards[0].grade == cards[2].grade &&
		cards[0].grade == cards[3].grade)
		return true;
	else if (cards[1].grade == cards[2].grade &&
		cards[1].grade == cards[3].grade)
		return true;
	else
		return false;
}

bool PokerUtils::IsThreeWithTwo(vector< Card >& cards) {
	if(cards.size() != 5) return false;
	if (cards[0].grade == cards[1].grade &&
		cards[0].grade == cards[2].grade &&
		cards[3].grade == cards[4].grade)
		return true;
	else if (cards[2].grade == cards[3].grade &&
		cards[2].grade == cards[4].grade &&
		cards[0].grade == cards[1].grade)
		return true;

	return false;
}

bool PokerUtils::IsSingleStraight(vector< Card >& cards) {
	if (cards.size() > 12 || cards.size() < 5)
		return false;

	// A的值是12，超过这个值就不是顺子
	if (cards[0].grade > 12)
		return false;

	// 循环遍历
	int last_grade = cards[0].grade;
	int current_grade;
	for (int i = 1; i < cards.size(); i++) {
		current_grade = cards[i].grade;
		//值不连续就不是顺子
		if ((current_grade + 1) != last_grade)
			return false;
		last_grade = current_grade;
	}
	return true;
}

bool PokerUtils::IsDoubleStraight(vector< Card >& cards) {
	int size = cards.size();
	if (size < 6 || size % 2 != 0)
		return false;

	// A的值是12，超过这个值就不是连对
	if (cards[0].grade > 12)
		return false;

	// 循环遍历
	int last_grade = cards[0].grade;
	int current_grade;
	for (int i = 1; i < cards.size(); i++) {
		current_grade = cards[i].grade;
		// i是奇数要和上一张比是不是相同牌
		if ((i + 1) % 2 == 0 && current_grade != last_grade)
			return false;

		// i是偶数要比较与上一张牌数值是不是相差1
		if (i % 2 == 0 && (current_grade + 1) != last_grade)
			return false;

		last_grade = current_grade;
	}
	return true;
}

bool PokerUtils::IsBoomWithSingle(vector< Card >& cards) {
	int	size = cards.size();
	if (size != 6)
		return false;

	// 中间两张一定相同
	if (cards[2].grade != cards[3].grade)
		return false;

	// 三种情况成立
	if ((cards[0].grade == cards[1].grade &&
		cards[0].grade == cards[2].grade) ||
		(cards[1].grade == cards[4].grade &&
			cards[1].grade == cards[2].grade) ||
			(cards[4].grade == cards[5].grade &&
				cards[4].grade == cards[2].grade))
		return true;

	return false;
}

bool PokerUtils::IsBoomWithDouble(vector< Card >& cards) {
	if (cards.size() != 8)
		return false;
	vector<Card> temp1;
	vector<Card> temp2;
	for (int i = 0; i < 6; i++)
		temp1.push_back(cards[i]);;
	for (int i = 2; i < 8; i++)
		temp2.push_back(cards[i]);;

	if (IsBoomWithSingle(temp1)) {
		if (cards[7].grade != cards[6].grade)
			return false;
		if (cards[0].grade != cards[1].grade &&
			cards[4].grade != cards[5].grade)
			return false;
	}
	else if (IsBoomWithSingle(temp2)) {
		if (cards[0].grade != cards[1].grade)
			return false;
		if (cards[2].grade != cards[3].grade &&
			cards[6].grade != cards[7].grade)
			return false;
	}

	return true;
}

bool PokerUtils::IsTwoPlane(vector< Card >& cards) {
	if(cards.size() != 6) return false;

	//头尾三张都相同是
	if (cards[0].grade == cards[1].grade &&
		cards[0].grade == cards[2].grade &&
		cards[3].grade == cards[4].grade &&
		cards[3].grade == cards[5].grade)
		return true;

	return false;
}

bool PokerUtils::IsThreePlane(vector< Card >& cards) {
	if(cards.size() != 9) return false;

	vector<Card> temp1;
	vector<Card> temp2;
	for (int i = 0; i < 6; i++)
		temp1.push_back(cards[i]);;
	for (int i = 3; i < 9; i++)
		temp2.push_back(cards[i]);;
	// 头尾都是双飞机
	if (IsTwoPlane(temp1) && IsTwoPlane(temp2))
		return true;
	return false;
}

bool PokerUtils::IsFourPlane(vector< Card >& cards) {
	if(cards.size() != 12) return false;

	vector<Card> temp1;
	vector<Card> temp2;
	for (int i = 0; i < 9; i++)
		temp1.push_back(cards[i]);;
	for (int i = 3; i < 12; i++)
		temp2.push_back(cards[i]);;
	// 头尾都是三飞机
	if (IsThreePlane(temp1) && IsThreePlane(temp2))
		return true;
	return false;
}

bool PokerUtils::IsFivePlane(vector< Card >& cards) {
	if(cards.size() != 15) return false;

	vector<Card> temp1;
	vector<Card> temp2;
	for (int i = 0; i < 12; i++)
		temp1.push_back(cards[i]);;
	for (int i = 3; i < 15; i++)
		temp2.push_back(cards[i]);;
	// 头尾都是四飞机
	if (IsFourPlane(temp1) && IsFourPlane(temp2))
		return true;
	return false;
}

bool PokerUtils::IsTwoPlaneWithSingle(vector< Card >& cards) {
	if(cards.size() != 8) return false;

	vector<Card> temp1;
	vector<Card> temp2;
	for (int i = 0; i < 6; i++)
		temp1.push_back(cards[i]);;
	for (int i = 2; i < 8; i++)
		temp2.push_back(cards[i]);;
	if (IsTwoPlane(temp1))
		return true;
	else if (IsTwoPlane(temp2))
		return true;

	return false;
}

bool PokerUtils::IsTwoPlaneWithDouble(vector< Card >& cards) {
	if(cards.size() != 10) return false;

	vector<Card> temp1;
	vector<Card> temp2;
	for (int i = 0; i < 6; i++)
		temp1.push_back(cards[i]);;
	for (int i = 4; i < 9; i++)
		temp2.push_back(cards[i]);;
	if (IsTwoPlane(temp1) &&
		cards[6].grade == cards[7].grade &&
		cards[8].grade == cards[9].grade)
		return true;
	else if (IsTwoPlane(temp2) &&
		cards[0].grade == cards[1].grade &&
		cards[2].grade == cards[3].grade)
		return true;

	return false;
}

bool PokerUtils::IsThreePlaneWithSingle(vector< Card >& cards) {
	if(cards.size() != 12) return false;

	vector<Card> temp1;
	vector<Card> temp2;
	for (int i = 0; i < 9; i++)
		temp1.push_back(cards[i]);;
	for (int i = 3; i < 12; i++)
		temp2.push_back(cards[i]);;
	if (IsThreePlane(temp1))
		return true;
	else if (IsThreePlane(temp2))
		return true;

	return false;
}

bool PokerUtils::IsThreePlaneWithDouble(vector< Card >& cards) {
	if(cards.size() != 15) return false;

	vector<Card> temp1;
	vector<Card> temp2;
	for (int i = 0; i < 9; i++)
		temp1.push_back(cards[i]);;
	for (int i = 6; i < 15; i++)
		temp2.push_back(cards[i]);;
	if (IsThreePlane(temp1)) {
		// 判断其余牌是不是对子
		int last_grade = cards[9].grade;
		int current_grade;
		for (int i = 10; i < 15; i++) {
			current_grade = cards[i].grade;
			// i是偶数要和上一张比是不是相同牌
			if (i % 2 == 0 && current_grade != last_grade)
				return false;

			last_grade = current_grade;
		}
		return true;
	}
	else if (IsThreePlane(temp2)) {
		// 判断其余牌是不是对子
		int last_grade = cards[0].grade;
		int current_grade;
		for (int i = 1; i < 6; i++) {
			current_grade = cards[i].grade;
			// i是偶数要和上一张比是不是相同牌
			if ((i + 1) % 2 == 0 && current_grade != last_grade)
				return false;

			last_grade = current_grade;
		}
		return true;
	}

	return true;
}

bool PokerUtils::IsFourPlaneWithSingle(vector< Card >& cards) {
	if(cards.size() != 16) return false;

	vector<Card> temp1;
	vector<Card> temp2;
	for (int i = 0; i < 12; i++)
		temp1.push_back(cards[i]);;
	for (int i = 4; i < 16; i++)
		temp2.push_back(cards[i]);;
	if (IsFourPlane(temp1)) {
		return true;
	}
	else if (IsFourPlane(temp2)) {
		return true;
	}

	return false;
}

bool PokerUtils::IsFourPlaneWithDouble(std::vector< Card > & cards)
{
    if(cards.size() != 15) return false;

    vector<Card> temp1;
    vector<Card> temp2;
    for (int i = 0; i < 12; i++)
        temp1.push_back(cards[i]);
    for (int i = 8; i < 20; i++)
        temp2.push_back(cards[i]);
    if (IsFourPlane(temp1)) {
        // 判断其余牌是不是对子
        int last_grade = cards[12].grade;
        int current_grade;
        for (int i = 13; i < 20; i++) {
        	current_grade = cards[i].grade;
        	// i是偶数要和上一张比是不是相同牌
        	if ((i + 1) % 2 == 0 && current_grade != last_grade)
        		return false;

        	last_grade = current_grade;
        }
    }
    else if (IsFourPlane(temp2)) {
        // 判断其余牌是不是对子
        int last_grade = cards[0].grade;
        int current_grade;
        for (int i = 1; i < 8; i++) {
        	current_grade = cards[i].grade;
        	// i是偶数要和上一张比是不是相同牌
        	if ((i + 1) % 2 == 0 && current_grade != last_grade)
        		return false;

        	last_grade = current_grade;
    }
    return true;
}

return true;
}

bool PokerUtils::IsFivePlaneWithSingle(vector< Card > & cards)
{
	if(cards.size() != 20) return false;

	vector<Card> temp1;
	vector<Card> temp2;
	for (int i = 0; i < 15; i++)
		temp1.push_back(cards[i]);;
	for (int i = 5; i < 20; i++)
		temp2.push_back(cards[i]);;
	if (IsFivePlane(temp1)) {
		return true;
	}
	else if (IsFivePlane(temp2)) {
		return true;
	}

	return false;
}
