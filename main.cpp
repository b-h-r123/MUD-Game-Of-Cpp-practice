#include "Game.h"
#include "ui.h"
#include <conio.h>
#include <string>
int main(){
 
    initConsole();
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    int selected = 0;
    auto items = getMenuItems();

    // 主菜单循环
    while (true) {
        // 渲染整个启动界面
        renderStartScreen(selected);

        // 读取按键
        int key = _getch();
        if (key == 0 || key == 224) {
            // 方向键双字节处理
            int arrow = _getch();
            switch (arrow) {
            case 72: // 上
                selected = (selected - 1 + (int)items.size()) % items.size();
                break;
            case 80: // 下
                selected = (selected + 1) % items.size();
                break;
            }
        }
        else {
            switch (key) {
            case 13: // Enter 确认
                clearScreen();
                if (selected == 0) {
                    // 新游戏流程
                    printNewGameInit();
                    system("pause");
                    game G;
                    G.game_id = 0;
                    G.showStoryIntro();

                    std::cout << "输入你的名字\n" << std::endl;
                    std::string name;
                    std::cin >> name;

                    G.player_start(name);
                    G.playStory();
                }
                else if (selected == 1) {
                    // 读取存档流程
                    printNoSaveFile();
                    system("pause");
                }
                else if (selected == 2) {
                    // 退出游戏
                    clearScreen();
                    printExitMessage();
                    Sleep(800);
                    return 0;
                }
                break;

            case 27: // Esc 退出
                clearScreen();
                printEscExitMessage();
                Sleep(600);
                return 0;
            }
        }
    }
    return 0;
}