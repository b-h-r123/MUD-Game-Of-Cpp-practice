#pragma once
#include <vector>
#include "Room.h"

class BattleSystem
{
public:
    // roomNum：记录这是第几个房间的主线战斗（1~6 对应 MAIN_1~MAIN_6）。
    // 传入共享的房间进度 rooms，战斗胜利返回 1 并同步标记/解锁对应房间。
    int battle(int roomNum, std::vector<Room>& rooms);
};
