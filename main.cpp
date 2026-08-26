#include "Game.h"
// ============================================================
//  Neon Echo (霓虹回响) - MUD 启动界面
//  赛博朋克风格 · Windows 控制台 · C++
//  编译: g++ NeonEcho.cpp -o NeonEcho.exe
// ============================================================

#include <iostream>
#include <windows.h>
#include <conio.h>
#include <string>
#include <vector>

// ---------- 颜色定义 (Windows 控制台属性) ----------
// 高亮度位 FOREGROUND_INTENSITY 让颜色更"霓虹"
enum Color {
    BLACK = 0,
    DARKRED = FOREGROUND_RED,
    DARKGREEN = FOREGROUND_GREEN,
    DARKBLUE = FOREGROUND_BLUE,
    CYAN = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
    MAGENTA = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
    YELLOW = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
    WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
    GRAY = FOREGROUND_INTENSITY,
    RED = FOREGROUND_RED | FOREGROUND_INTENSITY,
    GREEN = FOREGROUND_GREEN | FOREGROUND_INTENSITY,
    BLUE = FOREGROUND_BLUE | FOREGROUND_INTENSITY,
};

// ---------- 工具函数 ----------
void setColor(Color c) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

void resetColor() {
    setColor(WHITE);
}

void gotoXY(int x, int y) {
    COORD pos = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void hideCursor() {
    CONSOLE_CURSOR_INFO ci;
    ci.dwSize = 1;
    ci.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ci);
}

void clearScreen() {
    system("cls");
}

// 逐字打印 (带打字机效果)
void typePrint(const std::string& text, Color c, int delay = 20) {
    setColor(c);
    for (char ch : text) {
        std::cout << ch << std::flush;
        Sleep(delay);
    }
    resetColor();
}

// ---------- NEON ECHO ASCII 艺术字 ----------
// 每行对应一个颜色，营造霓虹渐变感
struct ArtLine {
    std::string text;
    Color color;
};

std::vector<ArtLine> getLogoArt() {
    return {
        // N
        { "███╗   ██╗███████╗ ██████╗ ███╗   ██╗", CYAN },
        { "████╗  ██║██╔════╝██╔═══██╗████╗  ██║", CYAN },
        { "██╔██╗ ██║█████╗  ██║   ██║██╔██╗ ██║", MAGENTA },
        { "██║╚██╗██║██╔══╝  ██║   ██║██║╚██╗██║", MAGENTA },
        { "██║ ╚████║███████╗╚██████╔╝██║ ╚████║", YELLOW },
        { "╚═╝  ╚═══╝╚══════╝ ╚═════╝ ╚═╝  ╚═══╝", YELLOW },
        { "                                            ", WHITE },
        // ECHO
        { "███████╗ ██████╗██╗  ██╗ ██████╗        ", CYAN },
        { "██╔════╝██╔════╝██║  ██║██╔═══██╗       ", CYAN },
        { "█████╗  ██║     ███████║██║   ██║       ", MAGENTA },
        { "██╔══╝  ██║     ██╔══██║██║   ██║       ", MAGENTA },
        { "███████╗╚██████╗██║  ██║╚██████╔╝       ", YELLOW },
        { "╚══════╝ ╚═════╝╚═╝  ╚═╝ ╚═════╝        ", YELLOW },
    };
}

// 打印 Logo (居中)
void printLogo() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int consoleWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;

    auto art = getLogoArt();
    int artWidth = 44; // 艺术字最大宽度
    int padding = (consoleWidth - artWidth) / 2;
    if (padding < 0) padding = 0;

    for (const auto& line : art) {
        setColor(line.color);
        std::cout << std::string(padding, ' ') << line.text << "\n";
    }
    resetColor();
}

// ---------- 装饰边框 ----------
void printBorder() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int w = csbi.srWindow.Right - csbi.srWindow.Left + 1;

    setColor(CYAN);
    std::cout << "╔";
    for (int i = 0; i < w - 2; i++) std::cout << "═";
    std::cout << "╗\n";

    for (int i = 0; i < 2; i++) {
        std::cout << "║";
        for (int j = 0; j < w - 2; j++) std::cout << " ";
        std::cout << "║\n";
    }
    resetColor();
}

void printBottomBorder() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int w = csbi.srWindow.Right - csbi.srWindow.Left + 1;

    setColor(CYAN);
    for (int i = 0; i < 2; i++) {
        std::cout << "║";
        for (int j = 0; j < w - 2; j++) std::cout << " ";
        std::cout << "║\n";
    }
    std::cout << "╚";
    for (int i = 0; i < w - 2; i++) std::cout << "═";
    std::cout << "╝\n";
    resetColor();
}

// ---------- 菜单 ----------
struct MenuItem {
    std::string label;
    std::string subLabel; // 英文副标题
};

std::vector<MenuItem> getMenuItems() {
    return {
        { "新 游 戏",   "NEW GAME" },
        { "读 取 存 档", "LOAD SAVE" },
        { "退       出", "EXIT" },
    };
}

// 绘制菜单 (selected: 当前选中项索引)
void drawMenu(int selected) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int w = csbi.srWindow.Right - csbi.srWindow.Left + 1;

    auto items = getMenuItems();
    int menuWidth = 30;
    int padding = (w - menuWidth) / 2;
    if (padding < 0) padding = 0;

    std::cout << "\n";

    // 菜单标题
    setColor(GRAY);
    std::cout << std::string(padding + 8, ' ') << ">> 选 择 选 项 <<\n\n";
    resetColor();

    for (size_t i = 0; i < items.size(); i++) {
        bool isSel = (i == selected);

        if (isSel) {
            // 选中项: 霓虹高亮 + 箭头指示
            setColor(YELLOW);
            std::cout << std::string(padding, ' ') << "  ▶ ";
            setColor(MAGENTA);
            std::cout << "[ " << items[i].label << " ]";
            setColor(CYAN);
            std::cout << "  " << items[i].subLabel << "\n";
        }
        else {
            // 未选中: 暗灰色
            setColor(GRAY);
            std::cout << std::string(padding, ' ') << "    ";
            std::cout << "  " << items[i].label << "   ";
            setColor(DARKBLUE);
            std::cout << "  " << items[i].subLabel << "\n";
        }
        resetColor();
        std::cout << "\n";
    }

    // 操作提示
    setColor(GRAY);
    std::cout << std::string(padding, ' ')
        << "  [↑/↓] 移动    [Enter] 确认    [Esc] 退出\n";
    resetColor();
}

// ---------- 闪烁霓虹效果 (Logo 颜色呼吸) ----------
void neonPulse(int cycles = 3) {
    // 简单实现: 在 Logo 区域用不同亮度重绘几次
    // 这里用文字闪烁代替
    for (int c = 0; c < cycles; c++) {
        setColor(WHITE);
        std::cout << "\r";
        Sleep(150);
        setColor(CYAN);
        Sleep(150);
    }
    resetColor();
}

// ---------- 主函数 ----------
int main() {
    SetConsoleOutputCP(65001);   // 65001 = UTF‑8
    SetConsoleCP(65001);

    // 设置控制台标题
    SetConsoleTitleA("Neon Echo - 霓虹回响");

    // 隐藏光标
    hideCursor();

    // 设置控制台大小 (可选)
    // system("mode con cols=100 lines=40");

    int selected = 0;
    auto items = getMenuItems();

    while (true) {
        clearScreen();

        // 顶部边框
        printBorder();

        // 打印 Logo
        printLogo();

        // 副标题 / 标语
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        int w = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        int pad = (w - 40) / 2;
        if (pad < 0) pad = 0;

        setColor(MAGENTA);
        std::cout << "\n" << std::string(pad, ' ')
            << "  ~ 在霓虹与废墟之间，活到最后。 ~\n";
        setColor(CYAN);
        std::cout << std::string(pad, ' ')
            << "    [ CYBERPUNK MUD · v0.1.0 ]\n\n";
        resetColor();

        // 菜单
        drawMenu(selected);

        // 底部边框
        printBottomBorder();

        // 读取按键
        int key = _getch();
        if (key == 0 || key == 224) {
            // 方向键是两个字节
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
            case 13: // Enter
                clearScreen();
                if (selected == 0) {
                    // 新游戏
                    setColor(CYAN);
                    std::cout << "\n\n\n";
                    typePrint("  >> 正在初始化游戏系统...\n", CYAN, 30);
                    typePrint("  >> 进入游戏中...\n", MAGENTA, 30);
                    typePrint("  >> 欢迎来到 霓虹回响。\n\n", YELLOW, 40);
                    resetColor();
                    system("pause");
                    game G;
                    G.game_id = 0;
                    G.showStoryIntro();
                    typePrint("  >>输入你的名字\n", CYAN, 30);
                    std::string name;
                    std::cin >> name;
                    G.player_start(name);
                    G.chapter(1);
                    G.showMainMenu();
                }
                else if (selected == 1) {
                    // 读取存档
                    setColor(CYAN);
                    std::cout << "\n\n\n";
                    typePrint("  >> 扫描存档扇区...\n", CYAN, 30);
                    typePrint("  >> 未检测到可用存档。\n", RED, 30);
                    typePrint("  >> [功能开发中]\n\n", GRAY, 30);
                    resetColor();
                    system("pause");
                }
                else if (selected == 2) {
                    // 退出
                    clearScreen();
                    setColor(MAGENTA);
                    std::cout << "\n\n\n";
                    typePrint("  >> 断开控制器连接...\n", MAGENTA, 30);
                    typePrint("  >> 再见，夜行者。\n\n", CYAN, 40);
                    resetColor();
                    Sleep(800);
                    return 0;
                }
                break;
            case 27: // Esc
                clearScreen();
                setColor(MAGENTA);
                std::cout << "\n\n\n  >> 断开控制器链接... 再见，夜行者。\n\n";
                resetColor();
                Sleep(600);
                return 0;
            }
        }
    }

    return 0;
}
