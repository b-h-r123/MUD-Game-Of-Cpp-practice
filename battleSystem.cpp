#include "battleSystem.h"
#include <iostream>
#include <windows.h>

int BattleSystem::battle(int roomNum, std::vector<Room>& rooms)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // 战斗系统尚未实现：不展示任何战斗细节，只声明战斗已经完成
    SetConsoleTextAttribute(hConsole, 0x0C);
    std::cout << "\n================ 战 斗 ================\n";
    std::cout << "\n<< 第 " << roomNum << " 房间的战斗 >>\n";
    std::cout << "\n<< 进入战斗 >>\n";
    SetConsoleTextAttribute(hConsole, 0x0A);
    std::cout << "战斗已完成！\n";
    SetConsoleTextAttribute(hConsole, 0x07);
    std::cout << "======================================\n";
    std::cout << "按任意键查看后续剧情...\n";
    system("pause>nul");

    // 战斗胜负判定（当前直接判定胜利；接入真实战斗后按实际结果给 success 赋值）
    int success = 1;

    // 战斗胜利：返回 1，并把“对应房间”的解锁状态写回 Room。
    // roomNum 直接映射为 RoomId::MAIN_roomNum（MAIN_1==1 ... MAIN_6==6），
    // 该房间原本 AVAILABLE，胜利后标记为 CLEARED（可通行），
    // 同时解锁所有以它为解锁前置的房间（如 MAIN_1 后开放 MAIN_2，
    // MAIN_3 后开放商店等）。逻辑与 map.cpp 进入房间后的状态机一致。
    if (success == 1)
    {
        Room* cur = findRoomById(rooms, static_cast<RoomId>(roomNum));
        if (cur != 0 && cur->getState() == RoomState::AVAILABLE && !cur->isRepeatable())
        {
            cur->markCleared();
            for (std::vector<Room>::iterator it = rooms.begin(); it != rooms.end(); ++it)
            {
                if (it->getState() == RoomState::LOCKED &&
                    it->getUnlockPrerequisite() == cur->getId())
                {
                    it->unlock();
                }
            }
        }
    }

    return success;
}
