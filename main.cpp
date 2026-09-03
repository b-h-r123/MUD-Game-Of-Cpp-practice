#include "Game.h"
#include <windows.h>
#include <conio.h>
#include <iostream>

// 隐藏光标
static void hideCursor() {
    CONSOLE_CURSOR_INFO ci;
    ci.dwSize = 1;
    ci.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ci);
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // 设置控制台标题
    SetConsoleTitleA("Neon Echo - 霓虹回响");

    // 隐藏光标
    hideCursor();

    Game game;
    game.start();

    return 0;
}
