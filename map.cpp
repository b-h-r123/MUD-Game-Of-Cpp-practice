#include "map.h"
#include "Room.h"
#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <conio.h>
#include <windows.h>
using namespace std;

// 权限判断统一以 Room.cpp 中 createDefaultRooms() 生成的 Room 状态为准，
// 实际状态由 Game 持有并在主线通关后更新，move() 直接读取同一份进度。
static const RoomId GRID_TO_ROOM[12] =
{
    RoomId::MAIN_1,         // 0  主线房1
    RoomId::MAIN_2,         // 1  主线房2
    RoomId::MAIN_3,         // 2  主线房3
    RoomId::MAIN_4,         // 3  主线房4
    RoomId::MAIN_5,         // 4  主线房5
    RoomId::MAIN_6,         // 5  主线房6(BOSS)
    RoomId::SHOP,           // 6  商店
    RoomId::BLACK_MARKET,   // 7  黑市
    RoomId::ADVANCED_CHEST, // 8  高级宝箱房
    RoomId::EXTRA_BATTLE,   // 9  附加怪房
    RoomId::ELITE_BATTLE,   // 10 精英怪房
    RoomId::CHEST           // 11 宝箱房
};

// 选择进入目标位置：先让玩家确认是否进入，再依据 Room 的解锁状态判断权限。
static void tryEnter(const Map grid[], std::vector<Room>& rooms, int& p, int nxt)
{
    cout << "\n是否进入「" << grid[nxt].name << "」？ (Y=进入 / N=取消)：";
    cout.flush();
    char yn = tolower(_getch());
    if (yn != 'y')
    {
        cout << "\n已取消，未进入「" << grid[nxt].name << "」。\n";
        _getch();
        return;
    }

    Room* target = findRoomById(rooms, GRID_TO_ROOM[nxt]);
    if (target != 0 && target->canEnter())
    {
        p = nxt;
        cout << "\n已成功进入「" << grid[nxt].name << "」。\n";
        // 进入一次性房间后视为完成其内容，并解锁以它为前置的房间（对应 Room.cpp 的状态机）
        if (target->getState() == RoomState::AVAILABLE && !target->isRepeatable())
        {
            target->markCleared();
            for (std::vector<Room>::iterator it = rooms.begin(); it != rooms.end(); ++it)
            {
                if (it->getState() == RoomState::LOCKED &&
                    it->getUnlockPrerequisite() == target->getId())
                {
                    it->unlock();
                }
            }
        }
    }
    else
    {
        const Room* pre = target != 0 ? findRoomById(rooms, target->getUnlockPrerequisite()) : 0;
        cout << "\n无法进入「" << grid[nxt].name << "」：该位置尚未对你开放。";
        if (pre != 0) cout << "（需先通关「" << pre->getName() << "」）";
        cout << "\n";
    }
    _getch();
}

void initRooms(Map rooms[])
{
    rooms[0] = { "主线房1", "开始点 · 主线战斗",         "MA1",  1, -1, -1, -1, 5, 1 };
    rooms[1] = { "主线房2", "主线战斗",                 "MA2",  2,  0, -1, -1, 5, 2 };
    rooms[2] = { "主线房3", "主线战斗",                 "MA3",  3,  1,  6, -1, 5, 3 };
    rooms[3] = { "主线房4", "主线战斗",                 "MA4",  4,  2, -1,  9, 5, 4 };
    rooms[4] = { "主线房5", "主线战斗",                 "MA5",  5,  3,  7, -1, 5, 5 };
    rooms[5] = { "主线房6(BOSS)", "最终战斗",           "MA6", -1,  4, -1, -1, 5, 6 };
    rooms[6] = { "商店",     "可重复进入 · MAIN_3后开放", "商店 ", -1, -1, -1,  2, 1, 3 };
    rooms[7] = { "黑市",     "可重复进入 · MAIN_5后开放", "黑市 ",  8, -1, -1,  4, 1, 5 };
    rooms[8] = { "高级宝箱房", "一次性奖励 · MAIN_5后开放", "高宝 ", -1, 7, -1, -1, 1, 6 };
    rooms[9] = { "附加怪房", "可选战斗 · MAIN_4后开放",  "附怪 ", 10, 11,  3, -1, 9, 4 };
    rooms[10] = { "精英怪房", "精英可选战斗",            "精怪 ", -1,  9, -1, -1, 9, 5 };
    rooms[11] = { "宝箱房",   "一次性奖励",              "宝箱 ",  9, -1, -1, -1, 9, 3 };
}

string buildFrame(const Map rooms[], int p)
{
    const char* BOX_T = "┌─────┐";
    const char* BOX_B = "└─────┘";
    const char* BOX_U = "┌──┴──┐";
    const char* BOX_D = "└──┬──┘";
    const char* ROAD = "───────";
    const char* BLANK = "       ";
    int x0 = 10, x1 = 0, y0 = 7, y1 = 0;
    for (int i = 0; i < 12; ++i)
    {
        if (rooms[i].mapX < x0) x0 = rooms[i].mapX;
        if (rooms[i].mapX > x1) x1 = rooms[i].mapX;
        if (rooms[i].mapY < y0) y0 = rooms[i].mapY;
        if (rooms[i].mapY > y1) y1 = rooms[i].mapY;
    }
    auto roomAt = [&](int x, int y)
        {
            for (int i = 0; i < 12; ++i)
                if (rooms[i].mapX == x && rooms[i].mapY == y) return i;
            return -1;
        };
    auto linked = [&](int x, int y)
        {
            int a = roomAt(x, y + 1), b = roomAt(x, y);
            return a != -1 && b != -1 && (rooms[b].north == a || rooms[a].south == b);
        };
    auto hroad = [&](int x, int y)
        {
            for (int i = 0; i < 12; ++i)
                if (rooms[i].mapY == y)
                {
                    if (rooms[i].east != -1 && x > rooms[i].mapX && x < rooms[rooms[i].east].mapX) return true;
                    if (rooms[i].west != -1 && x < rooms[i].mapX && x > rooms[rooms[i].west].mapX) return true;
                }
            return false;
        };
    auto pad5 = [](string t) { while (t.size() < 5) t += ' '; return t; };
    string s = "====== 《霓虹回响》网格地图 ======\n";
    s += "  W上 S下 A左 D右 移动 | Q退出\n\n";
    for (int y = y1; y >= y0; --y)
        for (int pass = 0; pass < 3; ++pass)
        {
            s += "    ";
            for (int x = x0; x <= x1; ++x)
            {
                int r = roomAt(x, y);
                if (r == -1)
                    s += (pass == 1 && hroad(x, y)) ? ROAD : BLANK;
                else if (pass == 0)
                    s += linked(x, y) ? BOX_U : BOX_T;
                else if (pass == 2)
                    s += linked(x, y - 1) ? BOX_D : BOX_B;
                else
                    s += r == p ? "│  *  │" : "│" + pad5(rooms[r].tag) + "│";
            }
            s += "\n";
        }
    auto pad = [](string t, int n) { while (t.size() < n) t += ' '; return t; };
    string dirs;
    if (rooms[p].north != -1) dirs += "W(上) ";
    if (rooms[p].south != -1) dirs += "S(下) ";
    if (rooms[p].west != -1) dirs += "A(左) ";
    if (rooms[p].east != -1) dirs += "D(右) ";
    s += "\n图例：MA1‑6=主线房1‑6 | 高宝=高级宝箱房 | 附怪=附加怪房 | 精怪=精英怪房\n";
    s += "      * = 你的当前位置\n";
    s += "------------------------------------------------------------------------\n";
    s += "当前房间：" + pad(rooms[p].name, 36) + "\n";
    s += "房间描述：" + pad(rooms[p].desc, 36) + "\n";
    s += "可移动方向：" + pad(dirs, 30) + "\n";
    s += "请输入操作：";
    return s;
}

// 在地图上移动：rooms 是 Game 持有的共享房间进度，避免每次进入地图都重置解锁状态。
void move(std::vector<Room>& rooms)
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cci = { 1, FALSE };
    SetConsoleCursorInfo(hOut, &cci);
    Map grid[12];
    initRooms(grid);
    int p = 0;
    while (true)
    {
        SetConsoleCursorPosition(hOut, { 0, 0 });
        cout << buildFrame(grid, p);
        cout.flush();
        char key = tolower(_getch());
        if (key == 'q') break;
        int nxt = -1;
        switch (key)
        {
        case 'w': nxt = grid[p].north; break;
        case 's': nxt = grid[p].south; break;
        case 'a': nxt = grid[p].west;  break;
        case 'd': nxt = grid[p].east;  break;
        default: continue;
        }
        if (nxt != -1) tryEnter(grid, rooms, p, nxt);
    }
    cci.bVisible = TRUE;
    SetConsoleCursorInfo(hOut, &cci);
    cout << "\n已退出地图程序。\n";
    

}
