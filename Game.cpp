#include "Game.h"

#include <conio.h>
#include <windows.h>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "map.h"
#include "plot.h"

// 主线六个房间的显示名，用于通关提示。
std::string roomDisplayName(int num);

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

Game::Game()
	: rooms(createDefaultRooms())
{
}

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

	// 开新局：清掉旧的解锁进度并重置地图。
	rooms = createDefaultRooms();
	roomNum = 1;

	clearScreen();
	printTopLine();
	printCentered("在出发之前，先告诉夜行者你的名字：", cCyan);
	setColor(cGray);
	std::cout << "\n\n\t\t请输入名字：";
	setColor(cWhite);
	std::string name;
	std::cin >> name;
	playerName_ = name;
	clearKeys();
	clearScreen();

	showStoryIntro();

	// 输入缓冲修复：清掉剧情动画期间攒下的多余按键。
	clearKeys();

	// 从第一间主线房开始主线剧情。
	playMainStory();
}

// 游戏内菜单：主角在剧情间隙可自由选择做什么。
void Game::showInGameMenu()
{
	while (true)
	{
		clearScreen();
		printTopLine();
		printCentered("游 戏 菜 单", cCyan);
		std::cout << "\n";
		setColor(cGray);
		std::cout << "\t\t[1] 进入地图自由探索\n";
		std::cout << "\t\t[2] 查看玩家状态\n";
		std::cout << "\t\t[3] 打开背包\n";
		std::cout << "\t\t[4] 保存游戏\n";
		std::cout << "\t\t[0] 返回主线\n";
		setColor(cWhite);
		printBottomLine();

		int key = _getch();
		if (key == '1')
		{
			clearScreen();
			chooseRoom();			// 在地图上自由移动
		}
		else if (key == '2')
		{
			showPlayerState();
		}
		else if (key == '3')
		{
			showBag();
		}
		else if (key == '4')
		{
			saveGame();
		}
		else if (key == '0' || key == 27)
		{
			return;
		}
	}
}

// 主线流程：从第 1 章一路打到第 6 章 Boss。
void Game::playMainStory()
{
	// 与 cxz-task 一致：六个主线房间按顺序推进，一章一仗。
	for (int n = 1; n <= 6; ++n)
	{
		chapter(n);

		// 每个房间打通后，给玩家一个打开菜单的机会。
		char choice = 0;
		while (choice != 'Y' && choice != 'y' && choice != 'N' && choice != 'n')
		{
			setColor(cYellow);
			std::cout << "\n是否打开游戏菜单？(Y=打开 / N=直接继续)：";
			setColor(cWhite);
			choice = (char)_getch();
			if (choice == 224 || choice == 0)
				choice = (char)_getch();
			std::cout << "\n";
		}
		if (choice == 'Y' || choice == 'y')
		{
			showInGameMenu();
		}
		clearKeys();
		clearScreen();
	}

	victory();
}

// 单个章节：播一章开头的剧情 → 触发对应房间的战斗 → 播一章结尾的剧情。
void Game::chapter(int num)
{
	clearScreen();
	printTopLine();
	setColor(cCyan);
	std::cout << "\n\t\t======== 第 " << num << " 章 ========\n\n";
	setColor(cWhite);
	printBottomLine();

	std::string chapterTag = "plot\\ch" + std::to_string(num) + "_";

	// 章节开头的过场剧情。
	printRhythm(chapterTag + "1.txt", true, playerName_, 20);
	printRhythm(chapterTag + "2.txt", true, playerName_, 20);

	clearKeys();
	std::cout << "\n\n按任意键进入战斗...\n";
	_getch();
	clearKeys();

	// 触发该房间的战斗；battle() 胜利后会回写房间解锁状态。
	battleSystem.battle(num, rooms);

	// 章节结尾的剧情。
	printRhythm(chapterTag + "3.txt", true, playerName_, 20);

	setColor(cGreen);
	std::cout << "\n\n\t\t「" << roomDisplayName(num) << "」已通关！\n";
	if (num < 6)
	{
		setColor(cCyan);
		std::cout << "\t\t已解锁下一章主线房间。\n";
	}
	setColor(cWhite);
}

std::string roomDisplayName(int num)
{
	static const char* names[6] =
	{
		"废土入口",
		"黑市街区",
		"工业工厂",
		"实验区域",
		"企业核心区",
		"NEON 核心"
	};
	if (num >= 1 && num <= 6)
		return names[num - 1];
	return "未知区域";
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
	// 进入网格地图自由探索：W/A/S/D 移动、Q 退出返回菜单。
	mapMove(rooms);
	clearKeys();
}

void Game::enterNowRoom(int roomNum)
{
	// 按房间类型分发：目前统一走进战斗，胜利后回写解锁状态。
	battleSystem.battle(roomNum, rooms);
}

void Game::victory()
{
	clearScreen();
	printTopLine();
	printCentered("GAME CLEAR  通关成功", cGreen);
	std::cout << "\n";
	setColor(cGreen);
	std::cout << "\n\t\t你击败了失控的 NEON，天穹城恢复了平静。\n";
	setColor(cYellow);
	std::cout << "\n\t\t感谢游玩《霓虹回响 · 智械危机》！\n";
	setColor(cWhite);
	printBottomLine();

	clearKeys();
	std::cout << "\n按任意键返回主菜单...\n";
	_getch();
	clearKeys();
	clearScreen();
}

void Game::defeat()
{
	clearScreen();
	printTopLine();
	printCentered("GAME OVER  游戏结束", cRed);
	std::cout << "\n";
	setColor(cRed);
	std::cout << "\n\t\t你已阵亡，任务失败...\n";
	setColor(cWhite);
	printBottomLine();

	clearKeys();
	std::cout << "\n按任意键返回主菜单...\n";
	_getch();
	clearKeys();
	clearScreen();
}

void Game::showShop()
{
	clearScreen();
	printTopLine();
	printCentered("商 店", cCyan);
	setColor(cGray);
	std::cout << "\n(商店功能开发中)\n";
	setColor(cWhite);
	printBottomLine();
	_getch();
}

void Game::showUpgrade()
{
	clearScreen();
	printTopLine();
	printCentered("强 化", cCyan);
	setColor(cGray);
	std::cout << "\n(强化功能开发中)\n";
	setColor(cWhite);
	printBottomLine();
	_getch();
}

void Game::showBag()
{
	clearScreen();
	printTopLine();
	printCentered("背 包", cCyan);
	setColor(cGray);
	std::cout << "\n(背包功能开发中)\n";
	setColor(cWhite);
	printBottomLine();
	clearKeys();
	_getch();
	clearKeys();
}

void Game::showPlayerState()
{
	clearScreen();
	printTopLine();
	printCentered("玩 家 状 态", cCyan);
	setColor(cYellow);
	std::cout << "\n\t\t名字：" << playerName_ << "\n";
	setColor(cGray);
	std::cout << "\n(完整属性面板开发中)\n";
	setColor(cWhite);
	printBottomLine();
	clearKeys();
	_getch();
	clearKeys();
}

void Game::saveGame()
{
	clearScreen();
	printTopLine();
	printCentered("保 存 游 戏", cCyan);
	setColor(cGray);
	std::cout << "\n(存档功能开发中)\n";
	setColor(cWhite);
	printBottomLine();
	clearKeys();
	_getch();
	clearKeys();
}

void Game::loadGame()
{
	clearScreen();
	printTopLine();
	printCentered("读 取 游 戏", cCyan);
	setColor(cGray);
	std::cout << "\n(读档功能开发中)\n";
	setColor(cWhite);
	printBottomLine();
	clearKeys();
	_getch();
	clearKeys();
}
