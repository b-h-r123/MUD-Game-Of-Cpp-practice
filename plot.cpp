#include "plot.h"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>
#ifdef _WIN32
#include <windows.h>
#include <conio.h>   // _kbhit _getch
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif
// 辅助：非阻塞检测是否按下空格
static bool checkSpaceKey()
{
#ifdef _WIN32
    if (_kbhit())
    {
        int ch = _getch();
        if (ch == ' ')
        {
            return true;
        }
    }
    return false;
#else
    // Linux/macOS 非阻塞读取按键
    struct termios oldt, newt;
    int ch;
    int oldflags;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldflags = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, oldflags | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldflags);

    if (ch == ' ')
    {
        return true;
    }
    if (ch != EOF) ungetc(ch, stdin);
    return false;
#endif
}

// 字符串替换 {{name}}
static std::string replaceAll(std::string text, const std::string& from, const std::string& to)
{
    size_t pos = 0;
    while ((pos = text.find(from, pos)) != std::string::npos)
    {
        text.replace(pos, from.length(), to);
        pos += to.length();
    }
    return text;
}

static void setRandomWholeColor()
{
#ifdef _WIN32
    int color = rand() % 14 + 1;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
#else
    int c = rand() % 7 + 31;
    std::cout << "\033[" << c << "m";
#endif
}

static void resetColor()
{
#ifdef _WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
#else
    std::cout << "\033[0m";
#endif
}
void Plot:: printRhythm(const std::string& input, bool isFile, const std::string& playerName, int delayMs)
{

    std::string content;

    if (isFile)
    {
        std::ifstream fin(input);
        if (!fin.is_open())
        {
            std::cerr << "[错误] 无法打开文件：" << input << std::endl;
            return;
        }
        std::stringstream buffer;
        buffer << fin.rdbuf();
        content = buffer.str();
        fin.close();
    }
    else
    {
        content = input;
    }

    if (!playerName.empty())
    {
        content = replaceAll(content, "{{name}}", playerName);
    }

    setRandomWholeColor();

    size_t i = 0;
    const size_t len = content.size();
    bool skipMode = false;

    while (i < len)
    {
        if (!skipMode && checkSpaceKey())
        {
            // 按下空格：直接把剩下全部内容一次性输出，开启跳过模式
            std::cout << content.substr(i);
            std::cout.flush();
            break;
        }

        std::cout << content[i];
        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
        i++;
    }

    resetColor();
}