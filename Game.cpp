#include "game.h"
#include <iostream>
#include <windows.h>
#include <string>
void printSlow(const std::string a, int ms)
{
    for (int i = 0; a[i] != '\0'; ++i)
    {
        std::cout << a[i];
        std::cout.flush();
        Sleep(ms);
    }
}
// 本地工具函数，设置控制台文字颜色
static void setColor(WORD color)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

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
    printSlow("===== 下层城区 · 佣兵故事 =====\n", 25);
    printSlow("【" + name + "】，你是一名挣扎在下层城区的佣兵。\n", 22);
    printSlow("阴暗潮湿的街巷，霓虹闪烁，大企业掌控着整座城市。\n", 22);
    printSlow("某天，你的通讯终端收到一条匿名加密信息……\n\n", 22);

    Sleep(800);
    printSlow("————【匿名消息】————\n", 20);
    Sleep(400);
    printSlow("如果你想知道这座城市真正的秘密，就进入企业核心，找到 NEON。\n", 20);
    Sleep(400);
    printSlow("——————————————————\n\n", 20);

    Sleep(1000);
    printSlow("任务目标：潜入城市核心，击败失控的 NEON‑X。\n", 25);
    std::cout << "\a\n";
    printSlow("姓名：玩家输入\n等级：1\nHP：100 / 100\nATK：20\nEXP：0GOLD：100\nENERGY：5 / 5\n属性点：0",10);
    printSlow("技能1：电磁脉冲   技能2：过载动能炮", 20);
    system("cls");
};