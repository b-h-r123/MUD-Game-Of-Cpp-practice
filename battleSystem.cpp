#include "battleSystem.h"
#include <iostream>
#include <windows.h>
#include <cstdlib>

void BattleSystem::battle()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // 战斗系统尚未实现：不展示任何战斗细节，只声明战斗已经完成
    SetConsoleTextAttribute(hConsole, 0x0C);
    std::cout << "\n================ 战 斗 ================\n";
    std::cout << "\n<< 进入战斗 >>\n";
    SetConsoleTextAttribute(hConsole, 0x0A);
    std::cout << "战斗已完成！\n";
    SetConsoleTextAttribute(hConsole, 0x07);
    std::cout << "======================================\n";
    std::cout << "按任意键查看后续剧情...\n";
    system("pause>nul");
}