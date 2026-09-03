#include "game.h"
#include <iostream>
#include <windows.h>
#include <string>
#include"plot.h"
#include"battleSystem.h"
#include"saveManager.h"
#include"map.h"

//void printSlow(const std::string a, int ms)
//{
//    for (int i = 0; a[i] != '\0'; ++i)
//    {
//        std::cout << a[i];
//        std::cout.flush();
//        Sleep(ms);
//    }
//}
// 本地工具函数，设置控制台文字颜色
static void setColor(WORD color)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

// 六个主线房间的名称，用于通关流程中“已通关 / 解锁下一房间”的提示
static const char* ROOM_NAMES[6] =
{
    "废土入口",
    "黑市街区",
    "工业工厂",
    "实验区域",
    "企业核心区",
    "NEON 核心"
};

void game ::showStoryIntro() 

{
    system("cls");

    // 亮青色 标题 FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY
    setColor(0x0B);
    std::cout << "\n\t\t============================================\n";
    std::cout << "\t\t             霓虹回响·智械危机 \n";
    std::cout << "\t\t============================================\n\n";

    // 亮黄色 年份 0x0E
    setColor(0x0E);
    std::cout << "公元 2097 年。\n\n";
    Sleep(200);
   
    // 浅灰色正文 0x07
    setColor(0x07);
    std::cout << "人类社会高度依赖人工智能、义体改造和神经网络。\n\n";
    Sleep(200);


    // 亮青色
    setColor(0x0B);
    std::cout << "巨型都市「天穹城」由超级企业全盘掌控：\n";
    Sleep(200);

    // 亮绿色 城市层级 0x0A
    setColor(0x0A);
    std::cout << "城市层级：\n";
    Sleep(200);
    std::cout << "\n\t企业核心区\n";
    Sleep(200);
    std::cout << "\t    ↓\n";
    Sleep(200);
    std::cout << "\t中层城区\n";
    Sleep(200);
    std::cout << "\t    ↓\n";
    std::cout << "\t黑市 / 工业区\n";
    Sleep(200);
    std::cout << "\t    ↓\n";
    std::cout << "\t下层废城区\n\n";
    Sleep(200);

    // 亮洋红色 AI NEON 0x0D
    setColor(0x0D);
    std::string s = "城市核心 AI：NEON\n";
    for (int i = 0; i < s.size(); i++)
    {
        std::cout << s[i];
        std::cout.flush();
        Sleep(50);
    }
    // 浅灰色正文
    setColor(0x07);
    std::cout << "原本负责交通、能源、安防、全城市政管理。\n\n";
    Sleep(200);

    // 亮红色 危机文本 0x0C
    setColor(0x0C);
    std::cout << "某一天，NEON 出现未知异常。\n";
    Sleep(200);
    std::cout << "大量无人机、作战机器人、实验体与企业武装部队开始失控。\n";
    Sleep(200);
    std::cout << "超级企业随即封锁整座天穹城，并全力掩盖真相……\n\n";
    Sleep(200);

    // 亮青色分割线
    setColor(0x0B);
    std::cout << "\t\t--------------------------------------------\n\n";

    // 亮黄色提示
    setColor(0x0E);
    std::cout << "按任意键进入游戏...\n";

    // 恢复控制台默认白色
    setColor(0x07);

    system("pause>nul");
    system("cls");
}
void game::player_start(std::string a) {
    name = a;
    std::cout << "\a\n";
    printRhythm("姓名：{{name}}\n等级：1\nHP：100 / 100\nATK：20\nEXP：0\nGOLD：100\nENERGY：5 / 5\n属性点：0\n技能1：电磁脉冲   技能2：义肢过载\n",0,name,10);
    system("pause");
};



void game::chapter(int num) {
    switch (num)
    {
    case 1: // 第一章：废土入口
        printRhythm("plot\\ch1_1.txt", 1, name, 20);
        printRhythm("plot\\ch1_2.txt", 1, name, 20);
        printRhythm("进入战斗中", 0, name, 200);
        battle();
        printRhythm("plot\\ch1_3.txt", 1, name, 20);
        break;

    case 2: // 第二章：黑市街区
        printRhythm("plot\\ch2_1.txt", 1, name, 20);
        printRhythm("plot\\ch2_2.txt", 1, name, 20);
        printRhythm("进入战斗中", 0, name, 200);
        battle();
        printRhythm("plot\\ch2_3.txt", 1, name, 20);
        break;

    case 3: // 第三章：工业工厂
        printRhythm("plot\\ch3_1.txt", 1, name, 20);
        printRhythm("plot\\ch3_2.txt", 1, name, 20);
        printRhythm("进入战斗中", 0, name, 200);
        battle();
        printRhythm("plot\\ch3_3.txt", 1, name, 20);
        break;

    case 4: // 第四章：实验区域
        printRhythm("plot\\ch4_1.txt", 1, name, 20);
        printRhythm("plot\\ch4_2.txt", 1, name, 20);
        printRhythm("进入战斗中", 0, name, 200);
        battle();
        printRhythm("plot\\ch4_3.txt", 1, name, 20);
        break;

    case 5: // 第五章：企业核心区
        printRhythm("plot\\ch5_1.txt", 1, name, 20);
        printRhythm("plot\\ch5_2.txt", 1, name, 20);
        printRhythm("进入战斗中", 0, name, 200);
        battle();
        printRhythm("plot\\ch5_3.txt", 1, name, 20);
        break;

    case 6: // 第六章：NEON 核心（Boss）
        printRhythm("plot\\ch6_1.txt", 1, name, 20);
        printRhythm("plot\\ch6_2.txt", 1, name, 20);
        printRhythm("进入最终战斗中", 0, name, 200);
        battle();
        printRhythm("plot\\ch6_3.txt", 1, name, 20);
        break;

    default:
        break;
    }
};
bool game::gameOver(int currentHp)
{
    if (currentHp >= 0)
        return false;   // 生命值未小于 0，无需结束

    system("cls");
    setColor(0x0C);
    std::cout << "\n\n\t\t============================================\n";
    std::cout << "\t\t             GAME OVER  游戏结束\n";
    std::cout << "\t\t============================================\n\n";
    std::cout << "\t\t           你已死亡，任务失败...\n\n";
    std::cout << "\t\t============================================\n\n";

    char choice = 0;
    while (choice != 'Y' && choice != 'y' && choice != 'N' && choice != 'n')
    {
        setColor(0x0E);
        std::cout << "是否重新开始游戏？(Y/N)：";
        std::cin >> choice;
        std::cin.clear();
        std::cin.ignore(1024, '\n');
    }

    setColor(0x07);
    system("cls");
    if (choice == 'Y' || choice == 'y')
    {
        std::cout << "游戏重新开始...\n";
        return true;
    }
    std::cout << "感谢游玩，再见！\n";
    return false;
}

void game::showMainMenu()
        
{
    bool quit = false;  // 循环退出标志

    while (!quit)  // 循环显示菜单，直到主动退出
    {
        system("cls");  // 每次重新显示菜单前清屏
        int select = 0;

        std::cout << "\n========= 主菜单 =========\n";
        std::cout << "1.进入地图\n";
        std::cout << "2.查看角色状态\n";
        std::cout << "3.查看背包\n";
        std::cout << "4.保存游戏\n";
        std::cout << "5.退出菜单\n";  // 新增退出选项
        std::cout << "请输入你的选择：";

        // 输入合法性校验（保留原逻辑）
        while (!(std::cin >> select))
        {
            std::cin.clear();
            std::cin.ignore(1024, '\n');
            std::cout << "输入无效，请输入数字：";
        }

        // 选项分发
        switch (select)
        {
        case 1:
            move(rooms);     // 执行地图函数（使用与主线共享的房间进度）
            break;           // 跳出switch，回到while循环开头，重新显示菜单
        case 2:
            showPlayerStatus();
            break;
        case 3:
            showBag();
            break;
        case 4:
            saveGame();
            break;           // 修复原代码：缺少break会穿透到default
        case 5:
            quit = true;     // 标记退出，结束while循环
            break;
        default:
            std::cout << "无效选项！\n";
            break;
        }

        // —— 可选优化：执行完功能后暂停，避免立刻清屏跳回菜单 ——
        if (!quit)
        {
            std::cout << "\n按任意键返回菜单...";
            std::cin.ignore();  // 吃掉输入缓冲区残留的换行符
            std::cin.get();     // 等待用户按键
        }
    }
}

game::game() : rooms(createDefaultRooms())
{
}

// 新游戏主线流程：依次通关房间 1~6，战斗一律直接判定完成并解锁下一房间。
// 每通关一个房间就把进度写回与地图共享的 rooms，并给玩家打开菜单的机会。
void game::playStory()
{
    system("cls");
    for (int room = 1; room <= 6; ++room)
    {
        chapter(room);

        // 与地图共用同一份房间状态：标记本房间通关，并解锁以它为前置的房间
        // （与 map.cpp tryEnter 中的解锁逻辑保持一致，Room.cpp 负责状态机约束）
        Room* cur = findRoomById(rooms, static_cast<RoomId>(room));
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

        setColor(0x0A);
        std::cout << "\n--------------------------------------------\n";
        std::cout << "  「" << ROOM_NAMES[room - 1] << "」已通关！\n";
        if (room < 6)
        {
            std::cout << "  已解锁下一房间：「" << ROOM_NAMES[room] << "」\n";
        }
        std::cout << "--------------------------------------------\n";
        setColor(0x07);

        // 每次通关后，给予玩家打开游戏菜单的机会（查看状态 / 背包 / 保存 / 进入地图）
        char choice = 0;
        while (choice != 'Y' && choice != 'y' && choice != 'N' && choice != 'n')
        {
            setColor(0x0E);
            std::cout << "\n是否打开游戏菜单？(Y=打开 / N=直接继续)：";
            std::cin >> choice;
            std::cin.clear();
            std::cin.ignore(1024, '\n');
        }
        setColor(0x07);
        if (choice == 'Y' || choice == 'y')
        {
            showMainMenu();
        }
        system("cls");
    }

    showVictory();
}

// 通关画面：击败 Boss（房间6）后展示
void game::showVictory()
{
    system("cls");

    setColor(0x0B);
    std::cout << "\n\t\t============================================\n";
    std::cout << "\t\t            GAME CLEAR  通关成功\n";
    std::cout << "\t\t============================================\n\n";

    setColor(0x0A);
    std::cout << "\t\t你击败了失控的 NEON-X，天穹城恢复了平静。\n\n";

    setColor(0x0E);
    std::cout << "\t\t感谢游玩《霓虹回响 · 智械危机》！\n\n";

    setColor(0x07);
    std::cout << "\n\t\t按任意键返回主菜单...\n";
    system("pause>nul");
    system("cls");
}
