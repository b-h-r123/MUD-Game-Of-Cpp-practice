#pragma once
#ifndef MUD_UI_H
#define MUD_UI_H

#include <string>
#include <vector>
#include <windows.h>

// 控制台颜色枚举（霓虹风格）
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

// Logo艺术字行结构
struct ArtLine {
    std::string text;
    Color color;
};

// 菜单项结构
struct MenuItem {
    std::string label;
    std::string subLabel;
};

// ========== 控制台初始化 ==========
void initConsole();

// ========== 控制台基础工具 ==========
void setColor(Color c);
void resetColor();
void gotoXY(int x, int y);
void hideCursor();
void clearScreen();

// ========== 文字特效 ==========
void typePrint(const std::string& text, Color c, int delay = 20);

// ========== Logo与边框 ==========
std::vector<ArtLine> getLogoArt();
void printLogo();
void printBorder();
void printBottomBorder();

// ========== 菜单系统 ==========
std::vector<MenuItem> getMenuItems();
void drawMenu(int selected);

// ========== 霓虹动效 ==========
void neonPulse(int cycles = 3);

// ========== 整屏渲染封装 ==========
void renderStartScreen(int selected);

// ========== 场景提示文本 ==========
void printNewGameInit();
void printNoSaveFile();
void printExitMessage();
void printEscExitMessage();

#endif // MUD_UI_H