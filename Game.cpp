#include "Game.h"

#include <conio.h>
#include <windows.h>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

const int cCyan    = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
const int cMagenta = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
const int cYellow  = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
const int cRed     = FOREGROUND_RED | FOREGROUND_INTENSITY;
const int cGreen   = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
const int cGray    = FOREGROUND_INTENSITY;
const int cBlue    = FOREGROUND_BLUE;
const int cWhite   = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;

void setColor(int color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void waitMs(int ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void clearScreen()
{
	system("cls");
}

// 把输入缓冲区里还剩下的按键清掉
void clearKeys()
{
	while (_kbhit())
		_getch();
}

// 控制台现在有多宽，画边框和居中要用
int consoleWidth()
{
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
	return info.srWindow.Right - info.srWindow.Left + 1;
}

// 一个字一个字往外蹦的打字机效果
void typeWrite(const std::string& text, int color, int ms)
{
	setColor(color);
	for (char ch : text)
	{
		std::cout << ch << std::flush;
		waitMs(ms);
	}
	setColor(cWhite);
}

void printCentered(const std::string& text, int color)
{
	int width = consoleWidth();
	int left = (width - (int)text.size()) / 2;
	if (left < 0) left = 0;

	setColor(color);
	std::cout << std::string(left, ' ') << text << "\n";
	setColor(cWhite);
}

// 界面最上面和最下面那两道框
void printTopLine()
{
	int width = consoleWidth();
	setColor(cCyan);
	std::cout << "╔";
	for (int i = 0; i < width - 2; i++) std::cout << "═";
	std::cout << "╗\n";
	setColor(cWhite);
}

void printBottomLine()
{
	int width = consoleWidth();
	setColor(cCyan);
	for (int i = 0; i < 2; i++)
	{
		std::cout << "║";
		for (int j = 0; j < width - 2; j++) std::cout << " ";
		std::cout << "║\n";
	}
	std::cout << "╚";
	for (int i = 0; i < width - 2; i++) std::cout << "═";
	std::cout << "╝\n";
	setColor(cWhite);
}

// NEON ECHO 的 logo，就是几行块字，换着颜色打出来
struct LogoLine
{
	std::string text;
	int color;
};

std::vector<LogoLine> neonLogo()
{
	return {
		{ "██      █  ██████  ██████  ██      █", cCyan },
		{ "███    █  ██          █        █  ███    █", cCyan },
		{ "█  ██  █  █████    █        █  █  ██  █", cMagenta },
		{ "█    ███  ██          █        █  █    ███", cMagenta },
		{ "█      ██  ██████  ██████  █      ██", cYellow },
		{ "                                             ", cWhite },
		{ "███████╗ ██████╗██╗  ██╗ ██████╗        ", cCyan },
		{ "██╔════╝██╔════╝██║  ██║██╔═══██╗       ", cCyan },
		{ "█████╗  ██║     ███████║██║   ██║       ", cMagenta },
		{ "██╔══╝  ██║     ██╔══██║██║   ██║       ", cMagenta },
		{ "███████╗╚██████╗██║  ██║╚██████╔╝       ", cYellow },
		{ "╚══════╝ ╚═════╝╚═╝  ╚═╝ ╚═════╝        ", cYellow },
	};
}

void printLogo()
{
	int width = consoleWidth();
	int left = (width - 44) / 2;	// 44 是 logo 一行大概的宽度
	if (left < 0) left = 0;

	for (const LogoLine& line : neonLogo())
	{
		setColor(line.color);
		std::cout << std::string(left, ' ') << line.text << "\n";
	}
	setColor(cWhite);
}

// 开场剧情，就是一段带颜色的背景介绍
void showStoryIntro()
{
	clearScreen();
	printTopLine();

	setColor(cCyan);
	std::cout << "\n\t\t============================================\n";
	std::cout << "\t\t            霓虹回响 · 智械危机\n";
	std::cout << "\t\t============================================\n\n";

	setColor(cYellow);
	std::cout << "公元 2097 年。\n\n";
	waitMs(300);

	setColor(cWhite);
	std::cout << "人类社会高度依赖人工智能、义体改造和神经网络。\n\n";
	waitMs(300);

	setColor(cCyan);
	std::cout << "巨型都市「天穹城」由超级企业全盘掌控。\n";
	waitMs(300);

	setColor(cGreen);
	std::cout << "城市从高到低分成几层：\n";
	std::cout << "\n\t企业核心区\n\t    ↓\n\t中层城区\n\t    ↓\n\t黑市 / 工业区\n\t    ↓\n\t下层废城区\n\n";
	waitMs(300);

	typeWrite("城市核心 AI：NEON\n", cMagenta, 50);
	setColor(cWhite);
	std::cout << "原本负责交通、能源、安防和全城市政管理。\n\n";
	waitMs(300);

	setColor(cRed);
	std::cout << "某一天，NEON 出现未知异常，无人机和作战机器人开始失控。\n";
	std::cout << "超级企业封锁了整座天穹城，想把这事情瞒下来。\n\n";
	waitMs(300);

	setColor(cCyan);
	std::cout << "\t\t--------------------------------------------\n";
	setColor(cYellow);
	std::cout << "\n按任意键进入游戏...\n";
	setColor(cWhite);

	printBottomLine();

	clearKeys();			// 剧情动画期间攒下的按键先清掉
	_getch();				// 等玩家按个键再继续
	clearKeys();			// 把多余的按键也清了，免得顶掉后面的界面
	clearScreen();
}

Game::Game() {}

void Game::start()
{
	while (gaming)
		mainMenu();

	clearScreen();
	setColor(cMagenta);
	std::cout << "\n再见，夜行者。\n";
	setColor(cWhite);
	waitMs(600);
}

void Game::mainMenu()
{
	const char* option[3] = { "开始新游戏", "加载旧游戏", "退出游戏" };
	const char* note[3]   = { "NEW GAME", "LOAD SAVE", "EXIT" };
	int select = 0;

	while (true)
	{
		clearScreen();
		printTopLine();
		printLogo();

		printCentered("~ 在霓虹与废墟之间，活到最后 ~", cMagenta);
		printCentered("CYBERPUNK MUD  v0.1.0", cCyan);
		std::cout << "\n";

		int width = consoleWidth();
		int left = (width - 30) / 2;
		if (left < 0) left = 0;

		setColor(cGray);
		std::cout << std::string(left + 8, ' ') << ">> 选 择 选 项 <<\n\n";
		setColor(cWhite);

		// 选中的那项用亮色框起来，其余用灰的
		for (int i = 0; i < 3; i++)
		{
			if (i == select)
			{
				setColor(cYellow);
				std::cout << std::string(left, ' ') << "▶ ";
				setColor(cMagenta);
				std::cout << "[ " << option[i] << " ]";
				setColor(cCyan);
				std::cout << "  " << note[i] << "\n";
			}
			else
			{
				setColor(cGray);
				std::cout << std::string(left, ' ') << "   " << option[i] << "    ";
				setColor(cBlue);
				std::cout << note[i] << "\n";
			}
			setColor(cWhite);
			std::cout << "\n";
		}

		setColor(cGray);
		std::cout << std::string(left, ' ') << "[↑/↓] 选择    [Enter] 确定    [Esc] 退出\n";
		setColor(cWhite);
		printBottomLine();

		int key = _getch();
		if (key == 224 || key == 0)
			key = _getch();			// 方向键会先返回一个 224
		if (key == 72)				// 上
			select = (select + 2) % 3;
		else if (key == 80)			// 下
			select = (select + 1) % 3;
		else if (key == 13)			// 回车
		{
			if (select == 0)
				startNewGame();
			else if (select == 1)
				loadingOldGame();
			else
				gaming = false;
			return;
		}
		else if (key == 27)			// Esc
		{
			gaming = false;
			return;
		}
	}
}

void Game::startNewGame()
{
	// 先问一句，防止把旧存档盖了
	int yesNo = 0;		// 0 是"是"，1 是"不"
	while (true)
	{
		clearScreen();
		printTopLine();
		printCentered("开新游戏会覆盖原来的存档，确定要继续吗？", cYellow);
		std::cout << "\n";
		if (yesNo == 0)
		{
			setColor(cCyan);
			std::cout << "\t\t▶ 是\n";
			setColor(cGray);
			std::cout << "\t\t  不\n";
		}
		else
		{
			setColor(cGray);
			std::cout << "\t\t  是\n";
			setColor(cCyan);
			std::cout << "\t\t▶ 不\n";
		}
		setColor(cWhite);
		printBottomLine();

		int key = _getch();
		if (key == 224 || key == 0)
			key = _getch();
		if (key == 72 || key == 80)
			yesNo = (yesNo + 1) % 2;
		else if (key == 13)
		{
			if (yesNo == 0)
				break;
			return;					// 选"不"，回主菜单
		}
	}

	showStoryIntro();
	chooseRoom();
}

void Game::loadingOldGame()
{
	clearScreen();
	printTopLine();
	typeWrite("正在扫描存档目录...\n", cCyan, 25);
	typeWrite("没有找到可用的存档。\n", cRed, 25);
	setColor(cGray);
	std::cout << "\n(读档功能还没做)\n";
	setColor(cWhite);
	printBottomLine();

	clearKeys();
	_getch();			// 任意键回主菜单
	clearKeys();
}

void Game::chooseRoom()
{
	// 选关界面，关卡还没做，先放个占位说明
	while (true)
	{
		clearScreen();
		printTopLine();
		printCentered("选 关 界 面", cCyan);
		setColor(cGray);
		std::cout << "\n关卡还没做完，这里先空着。\n";
		std::cout << "\n按 [0] 或 [Esc] 返回主菜单\n";
		setColor(cWhite);
		printBottomLine();

		int key = _getch();
		if (key == '0' || key == 27)	// 0 或 Esc 才退出
			return;
		// 按别的键就停在这一页
	}
}
