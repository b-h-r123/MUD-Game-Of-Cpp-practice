#include "plot.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

#ifdef _WIN32
#include <conio.h>
#else
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif

namespace
{
// 把文本中的 from 全部替换成 to。
std::string replaceAll(std::string text, const std::string& from, const std::string& to)
{
    if (from.empty())
    {
        return text;
    }
    size_t pos = 0;
    while ((pos = text.find(from, pos)) != std::string::npos)
    {
        text.replace(pos, from.length(), to);
        pos += to.length();
    }
    return text;
}

// 返回以 s[0] 开头的那个 UTF-8 字符占用几个字节。
// 这样中文等宽字符可以整字输出，不会因逐字节输出而在控制台产生乱码。
size_t utf8CharLen(unsigned char lead)
{
    if (lead < 0x80)
    {
        return 1;
    }
    if ((lead & 0xE0) == 0xC0)
    {
        return 2;
    }
    if ((lead & 0xF0) == 0xE0)
    {
        return 3;
    }
    if ((lead & 0xF8) == 0xF0)
    {
        return 4;
    }
    return 1;
}

#ifdef _WIN32
// Windows：非阻塞地检查空格键是否被按下。按过空格返回 true，其它键忽略。
bool spaceKeyPressed()
{
    if (_kbhit())
    {
        int ch = _getch();
        if (ch == ' ')
        {
            return true;
        }
    }
    return false;
}
#else
// Linux/macOS：把终端切到非阻塞原始模式再读一次，尽量避免干扰正常输入。
bool spaceKeyPressed()
{
    struct termios oldt;
    struct termios newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    int oldFlags = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, oldFlags | O_NONBLOCK);

    int ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldFlags);

    if (ch == ' ')
    {
        return true;
    }
    if (ch != EOF)
    {
        ungetc(ch, stdin);
    }
    return false;
}
#endif
} // namespace

void printRhythm(const std::string& input, bool isFile, const std::string& playerName, int delayMs)
{
    std::string content;

    if (isFile)
    {
        std::ifstream fin(input);
        if (!fin.is_open())
        {
            std::cerr << "[错误] 无法打开剧情文件：" << input << std::endl;
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

    // 按完整字符播放。按下空格后进入跳过模式，一次输出完剩余内容。
    bool skip = false;
    for (size_t i = 0; i < content.size();)
    {
        if (!skip && spaceKeyPressed())
        {
            skip = true;
        }

        if (skip)
        {
            std::cout << content.substr(i);
            std::cout.flush();
            break;
        }

        size_t len = utf8CharLen(static_cast<unsigned char>(content[i]));
        if (i + len > content.size())
        {
            len = content.size() - i;
        }
        std::cout << content.substr(i, len);
        std::cout.flush();
        i += len;

        if (delayMs > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
        }
    }
}
