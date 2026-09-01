#include "ui.h"
#include <iostream>
#include <conio.h>

using std::string;
using std::vector;
using std::cout;

// 控制台初始化：编码、标题、光标
void initConsole() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    SetConsoleTitleA("Neon Echo - 霓虹回响");
    hideCursor();
}

// 设置控制台文字颜色
void setColor(Color c) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

// 重置为默认白色
void resetColor() {
    setColor(WHITE);
}

// 移动光标到指定坐标
void gotoXY(int x, int y) {
    COORD pos = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

// 隐藏控制台光标
void hideCursor() {
    CONSOLE_CURSOR_INFO ci;
    ci.dwSize = 1;
    ci.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ci);
}

// 清屏
void clearScreen() {
    system("cls");
}

// 打字机逐字打印效果
void typePrint(const string& text, Color c, int delay) {
    setColor(c);
    for (char ch : text) {
        cout << ch << std::flush;
        Sleep(delay);
    }
    resetColor();
}

// 获取霓虹Logo艺术字数据
vector<ArtLine> getLogoArt() {
    return {
        { "███╗   ██╗███████╗ ██████╗ ███╗   ██╗", CYAN },
        { "████╗  ██║██╔════╝██╔═══██╗████╗  ██║", CYAN },
        { "██╔██╗ ██║█████╗  ██║   ██║██╔██╗ ██║", MAGENTA },
        { "██║╚██╗██║██╔══╝  ██║   ██║██║╚██╗██║", MAGENTA },
        { "██║ ╚████║███████╗╚██████╔╝██║ ╚████║", YELLOW },
        { "╚═╝  ╚═══╝╚══════╝ ╚═════╝ ╚═╝  ╚═══╝", YELLOW },
        { "                                            ", WHITE },
        { "███████╗ ██████╗██╗  ██╗ ██████╗        ", CYAN },
        { "██╔════╝██╔════╝██║  ██║██╔═══██╗       ", CYAN },
        { "█████╗  ██║     ███████║██║   ██║       ", MAGENTA },
        { "██╔══╝  ██║     ██╔══██║██║   ██║       ", MAGENTA },
        { "███████╗╚██████╗██║  ██║╚██████╔╝       ", YELLOW },
        { "╚══════╝ ╚═════╝╚═╝  ╚═╝ ╚═════╝        ", YELLOW },
    };
}

// 居中打印Logo
void printLogo() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int consoleWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    auto art = getLogoArt();
    int artWidth = 44;
    int padding = (consoleWidth - artWidth) / 2;
    if (padding < 0) padding = 0;

    for (const auto& line : art) {
        setColor(line.color);
        cout << string(padding, ' ') << line.text << "\n";
    }
    resetColor();
}

// 打印顶部装饰边框
void printBorder() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int w = csbi.srWindow.Right - csbi.srWindow.Left + 1;

    setColor(CYAN);
    cout << "╔";
    for (int i = 0; i < w - 2; i++) cout << "═";
    cout << "╗\n";

    for (int i = 0; i < 2; i++) {
        cout << "║";
        for (int j = 0; j < w - 2; j++) cout << " ";
        cout << "║\n";
    }
    resetColor();
}

// 打印底部装饰边框
void printBottomBorder() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int w = csbi.srWindow.Right - csbi.srWindow.Left + 1;

    setColor(CYAN);
    for (int i = 0; i < 2; i++) {
        cout << "║";
        for (int j = 0; j < w - 2; j++) cout << " ";
        cout << "║\n";
    }

    cout << "╚";
    for (int i = 0; i < w - 2; i++) cout << "═";
    cout << "╝\n";
    resetColor();
}

// 获取主菜单选项数据
vector<MenuItem> getMenuItems() {
    return {
        { "新 游 戏",   "NEW GAME" },
        { "读 取 存 档", "LOAD SAVE" },
        { "退       出", "EXIT" },
    };
}

// 绘制菜单（高亮当前选中项）
void drawMenu(int selected) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int w = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    auto items = getMenuItems();
    int menuWidth = 30;
    int padding = (w - menuWidth) / 2;
    if (padding < 0) padding = 0;

    cout << "\n";
    // 菜单标题
    setColor(GRAY);
    cout << string(padding + 8, ' ') << ">> 选 择 选 项 <<\n\n";
    resetColor();

    for (size_t i = 0; i < items.size(); i++) {
        bool isSel = (i == selected);
        if (isSel) {
            // 选中项霓虹高亮
            setColor(YELLOW);
            cout << string(padding, ' ') << "  ▶ ";
            setColor(MAGENTA);
            cout << "[ " << items[i].label << " ]";
            setColor(CYAN);
            cout << "  " << items[i].subLabel << "\n";
        }
        else {
            // 未选中项暗灰
            setColor(GRAY);
            cout << string(padding, ' ') << "    ";
            cout << "  " << items[i].label << "   ";
            setColor(DARKBLUE);
            cout << "  " << items[i].subLabel << "\n";
        }
        resetColor();
        cout << "\n";
    }

    // 操作提示
    setColor(GRAY);
    cout << string(padding, ' ')
        << "  [↑/↓] 移动    [Enter] 确认    [Esc] 退出\n";
    resetColor();
}

// 霓虹呼吸闪烁效果
void neonPulse(int cycles) {
    for (int c = 0; c < cycles; c++) {
        setColor(WHITE);
        cout << "\r";
        Sleep(150);
        setColor(CYAN);
        Sleep(150);
    }
    resetColor();
}

// 封装：完整渲染启动界面（清屏+边框+Logo+副标题+菜单+底框）
void renderStartScreen(int selected) {
    clearScreen();
    printBorder();
    printLogo();

    // 副标题与版本号
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int w = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int pad = (w - 40) / 2;
    if (pad < 0) pad = 0;

    setColor(MAGENTA);
    cout << "\n" << string(pad, ' ')
        << "  ~ 在霓虹与废墟之间，活到最后。 ~\n";
    setColor(CYAN);
    cout << string(pad, ' ')
        << "    [ CYBERPUNK MUD · v0.1.0 ]\n\n";
    resetColor();

    drawMenu(selected);
    printBottomBorder();
}

// 新游戏加载提示
void printNewGameInit() {
    cout << "\n\n\n";
    typePrint("  >> 正在初始化游戏系统...\n", CYAN, 30);
    typePrint("  >> 进入游戏中...\n", MAGENTA, 30);
    typePrint("  >> 欢迎来到 霓虹回响。\n\n", YELLOW, 40);
    resetColor();
}

// 无存档提示
void printNoSaveFile() {
    cout << "\n\n\n";
    typePrint("  >> 扫描存档扇区...\n", CYAN, 30);
    typePrint("  >> 未检测到可用存档。\n", RED, 30);
    typePrint("  >> [功能开发中]\n\n", GRAY, 30);
    resetColor();
}

// 退出提示
void printExitMessage() {
    cout << "\n\n\n";
    typePrint("  >> 断开控制器连接...\n", MAGENTA, 30);
    typePrint("  >> 再见，夜行者。\n\n", CYAN, 40);
    resetColor();
}

// Esc快速退出提示
void printEscExitMessage() {
    setColor(MAGENTA);
    cout << "\n\n\n  >> 断开控制器链接... 再见，夜行者。\n\n";
    resetColor();
}