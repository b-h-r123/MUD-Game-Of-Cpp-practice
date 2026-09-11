#include "Game.h"

#include <conio.h>
#include <windows.h>
#include <chrono>
#include <iostream>
#include <limits>
#include <string>
#include <thread>
#include <vector>

#include "map.h"
#include "plot.h"
#include "Shop.h"

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

// 存档文件名（相对于程序工作目录）。
static const char* SAVE_FILE = "savegame.txt";

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

// 战斗内部用 std::cin 读数字，结束后把 cin 里残留的换行丢弃，
// 避免影响后面 _getch() 的判断。
void clearCin()
{
	std::cin.clear();
	std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
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

	// 重置为一个全新玩家：满血满能量，属性点清零，并写入玩家名。
	player = Player();
	player.setPlayerName(playerName_);
	player.restoreToFull();

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
		std::cout << "\t\t[4] 进入普通商店\n";
		std::cout << "\t\t[5] 进入黑市\n";
		std::cout << "\t\t[6] 强化（分配属性点）\n";
		std::cout << "\t\t[7] 保存游戏\n";
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
			showShop();
		}
		else if (key == '5')
		{
			showBlackMarket();
		}
		else if (key == '6')
		{
			showUpgrade();
		}
		else if (key == '7')
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
// 主线流程：从第 1 章一路打到第 6 章 Boss。
// 中途阵亡会立刻结束这一局，回到主菜单。
void Game::playMainStory()
{
	for (int n = 1; n <= 6; ++n)
	{
		if (!chapter(n))
		{
			defeat();
			return;
		}

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

// 按章节挑选一只敌人（副本），敌人数据表见 Enemy::EnemyList。
static Enemy enemyForChapter(int num)
{
	// 前五章用递增难度的普通敌人，第六章是 Boss NEON-X。
	int index = 0;
	switch (num)
	{
	case 1: index = 0; break;  // 废土鼠
	case 2: index = 1; break;  // 街头猎犬
	case 3: index = 2; break;  // 改造人
	case 4: index = 3; break;  // 黑客佣兵
	case 5: index = 5; break;  // 重装保镖
	case 6: index = 7; break;  // NEON - X（Boss）
	default: index = 0; break;
	}
	return Enemy::EnemyList[index];
}

// 单个章节：播一章开头的剧情 → 触发对应房间的真实战斗 → 播一章结尾的剧情。
// 返回 true 表示本章打通；玩家阵亡返回 false。
bool Game::chapter(int num)
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

	// 每章视作一次“回到安全区”，开打前回满血与能量，保证体验连贯。
	player.restoreToFull();

	clearKeys();
	std::cout << "\n\n按任意键进入战斗...\n";
	_getch();
	clearKeys();

	// 构造本章敌人并进入真实回合制战斗。
	Enemy foe = enemyForChapter(num);
	setColor(cRed);
	std::cout << "\n\t你遭遇了 " << foe.getName() << " ！\n";
	setColor(cWhite);

	bool won = player.battle(player, foe);

	// 战斗内部用 std::cin，结束后清掉残留输入，避免干扰后续 _getch。
	clearCin();

	// 战斗结束后给一小段停顿，避免结果一闪而过。
	clearKeys();
	std::cout << "\n按任意键继续...\n";
	_getch();
	clearKeys();

	if (!won)
	{
		return false;
	}

	// 胜利：回写房间解锁状态。
	unlockRoomAfterChapter(num);

	// 每过一关自动存档一次，避免进度丢失。
	if (saveManager.save(SAVE_FILE, player, rooms))
	{
		setColor(cCyan);
		std::cout << "\n\t\t(进度已自动保存)\n";
	}

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
	return true;
}

void Game::unlockRoomAfterChapter(int num)
{
	// 把对应的主线房标记为 CLEARED，并解锁以它为前置的房间。
	Room* cur = findRoomById(rooms, static_cast<RoomId>(num));
	if (cur != nullptr && cur->getState() == RoomState::AVAILABLE && !cur->isRepeatable())
	{
		cur->markCleared();
		for (std::vector<Room>::iterator it = rooms.begin(); it != rooms.end(); ++it)
		{
			if (it->getState() == RoomState::LOCKED &&
				it->getUnlockPrerequisite() == cur->getId())
			{
				it->unlock();
			}
		}
	}
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

	if (saveManager.load(SAVE_FILE, player, rooms))
	{
		playerName_ = player.getPlayerName();
		setColor(cGreen);
		std::cout << "\n读档成功，欢迎回来，" << playerName_ << "。\n";
		setColor(cWhite);
		printBottomLine();
		clearKeys();
		std::cout << "\n按任意键继续...\n";
		_getch();
		clearKeys();
		showInGameMenu();
	}
	else
	{
		typeWrite("没有找到可用的存档。\n", cRed, 25);
		setColor(cWhite);
		printBottomLine();
		clearKeys();
		std::cout << "\n按任意键返回主菜单...\n";
		_getch();
		clearKeys();
	}
}

void Game::chooseRoom()
{
	// 进入网格地图自由探索：W/A/S/D 移动、Q 退出返回菜单。
	mapMove(rooms);
	clearKeys();
}

void Game::enterNowRoom(int roomNum)
{
	// 从地图进入一间战斗房时，按房间号挑选敌人开战。
	// 主线房 1~6 对应章节敌人，其余房间退回第一档敌人。
	Enemy foe = enemyForChapter(roomNum);
	bool won = player.battle(player, foe);
	if (won)
	{
		unlockRoomAfterChapter(roomNum);
	}
	else
	{
		defeat();
	}
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
	runShop(false);
}

void Game::showBlackMarket()
{
	runShop(true);
}

void Game::runShop(bool blackMarket)
{
	// 门禁：对应的商店房尚未解锁时不允许进入。
	const RoomId shopRoomId = blackMarket ? RoomId::BLACK_MARKET : RoomId::SHOP;
	const Room* shopRoom = findRoomById(rooms, shopRoomId);
	if (shopRoom == nullptr || !shopRoom->canEnter())
	{
		clearScreen();
		printTopLine();
		printCentered(blackMarket ? "黑市尚未开放。" : "商店尚未开放。", cRed);
		printBottomLine();
		clearKeys();
		_getch();
		clearKeys();
		return;
	}

	const ShopType type = blackMarket ? ShopType::BLACK_MARKET : ShopType::NORMAL;
	Shop shop(type, player.getCatalog(), ShopConfig());
	if (!shop.enter())
	{
		clearScreen();
		printTopLine();
		printCentered("商店暂时无法营业。", cRed);
		printBottomLine();
		_getch();
		return;
	}

	while (true)
	{
		clearScreen();
		printTopLine();
		printCentered(blackMarket ? "黑 市" : "普 通 商 店", cMagenta);
		setColor(cCyan);
		std::cout << "\n\t\t金币：" << player.getGold() << "\n\n";

		const std::vector<ShopOffer>& offers = shop.getOffers();
		if (offers.empty())
		{
			setColor(cGray);
			std::cout << "\t\t(货架已空)\n";
		}
		else
		{
			for (std::size_t i = 0; i < offers.size(); ++i)
			{
				const Item* def = player.getCatalog().findById(offers[i].itemId);
				setColor(cWhite);
				std::cout << "\t\t[" << (i + 1) << "] "
					<< (def != 0 ? def->getName() : "未知物品")
					<< "  " << offers[i].price << " 金";
				setColor(cGray);
				std::cout << "  " << (def != 0 ? def->getDescription() : "") << "\n";
			}
		}

		setColor(cGray);
		std::cout << "\n\t\t输入编号购买，[0] 离开：";
		setColor(cWhite);
		int key = _getch();
		if (key == '0' || key == 27)
		{
			break;
		}

		const int index = key - '1';
		if (index < 0 || index >= static_cast<int>(offers.size()))
		{
			continue;
		}

		const ShopOffer& offer = offers[index];
		const Item* def = player.getCatalog().findById(offer.itemId);
		if (def == 0)
		{
			continue;
		}

		if (player.getGold() < offer.price)
		{
			setColor(cRed);
			std::cout << "\n\t\t金币不足，无法购买。\n";
		}
		else if (!player.receiveItem(*def))
		{
			setColor(cRed);
			std::cout << "\n\t\t无法携带该物品（装备栏或背包限制）。\n";
		}
		else
		{
			player.spendGold(offer.price);
			shop.commitPurchase(index);
			setColor(cGreen);
			std::cout << "\n\t\t购买成功：" << def->getName() << "\n";
		}
		setColor(cWhite);
		std::cout << "\t\t按任意键继续...";
		_getch();
	}
	clearKeys();
}

void Game::showUpgrade()
{
	while (true)
	{
		clearScreen();
		printTopLine();
		printCentered("强 化", cCyan);

		setColor(cWhite);
		std::cout << "\n\t\t等级：" << player.getLevel()
			<< "    经验：" << player.getExp()
			<< " / " << player.getRequiredExpForNextLevel() << "\n";
		std::cout << "\t\t攻击：" << player.getAtk()
			<< "    最大生命：" << player.getMHp()
			<< "    最大能量：" << player.getMEnergy() << "\n";
		setColor(cGreen);
		std::cout << "\t\t可用属性点：" << player.getAtp() << "\n\n";

		setColor(cGray);
		std::cout << "\t\t[1] 攻击力 +5\n";
		std::cout << "\t\t[2] 最大生命 +20\n";
		std::cout << "\t\t[0] 返回\n";
		setColor(cWhite);
		printBottomLine();

		int key = _getch();
		if (key == '1')
		{
			if (player.allocateAttack())
			{
				setColor(cGreen);
				std::cout << "\n\t\t攻击力提升！\n";
			}
			else
			{
				setColor(cRed);
				std::cout << "\n\t\t属性点不足。\n";
			}
			setColor(cWhite);
			std::cout << "\t\t按任意键继续...";
			_getch();
		}
		else if (key == '2')
		{
			if (player.allocateMaxHp())
			{
				setColor(cGreen);
				std::cout << "\n\t\t最大生命提升！\n";
			}
			else
			{
				setColor(cRed);
				std::cout << "\n\t\t属性点不足。\n";
			}
			setColor(cWhite);
			std::cout << "\t\t按任意键继续...";
			_getch();
		}
		else if (key == '0' || key == 27)
		{
			break;
		}
	}
	clearKeys();
}

void Game::showBag()
{
	while (true)
	{
		clearScreen();
		printTopLine();
		printCentered("背 包", cCyan);

		const ItemSlots& slots = player.getItemSlots();
		const std::array<ConsumableSlot, 4>& consumables = slots.getConsumableSlots();

		setColor(cWhite);
		std::cout << "\n\t\t--- 消耗品 ---\n";
		for (std::size_t i = 0; i < consumables.size(); ++i)
		{
			const Item* def = player.getCatalog().findById(consumables[i].getItemId());
			std::cout << "\t\t[" << (i + 1) << "] "
				<< (def != 0 ? def->getName() : "未知")
				<< "  x" << consumables[i].getCount() << "\n";
		}

		std::cout << "\n\t\t--- 装备 ---\n";
		const EquipmentSlot eqSlots[3] = { EquipmentSlot::WEAPON, EquipmentSlot::SHIELD, EquipmentSlot::ACCESSORY };
		const char* eqNames[3] = { "武器", "护盾", "配饰" };
		for (int i = 0; i < 3; ++i)
		{
			const ItemId id = slots.getEquippedItem(eqSlots[i]);
			const Item* def = player.getCatalog().findById(id);
			std::cout << "\t\t" << eqNames[i] << "："
				<< (def != 0 ? def->getName() : "（空）") << "\n";
		}

		setColor(cCyan);
		std::cout << "\n\t\t攻击 " << player.getAtk()
			<< "   最大生命 " << player.getMHp()
			<< "   最大能量 " << player.getMEnergy() << "\n";

		setColor(cGray);
		std::cout << "\n\t\t输入消耗品编号使用，[0] 返回：";
		setColor(cWhite);

		int key = _getch();
		if (key == '0' || key == 27)
		{
			break;
		}
		const int index = key - '1';
		if (index < 0 || index >= static_cast<int>(consumables.size()))
		{
			continue;
		}
		const ItemId id = consumables[index].getItemId();
		const Item* def = player.getCatalog().findById(id);
		if (def == 0 || consumables[index].getCount() <= 0)
		{
			continue;
		}
		if (!def->canUseOutsideBattle())
		{
			setColor(cRed);
			std::cout << "\n\t\t该道具只能在战斗中使用。\n";
		}
		else if (player.useConsumable(id))
		{
			setColor(cGreen);
			std::cout << "\n\t\t使用了 " << def->getName() << "。\n";
		}
		else
		{
			setColor(cRed);
			std::cout << "\n\t\t使用失败。\n";
		}
		setColor(cWhite);
		std::cout << "\t\t按任意键继续...";
		_getch();
	}
	clearKeys();
}

void Game::showPlayerState()
{
	clearScreen();
	printTopLine();
	printCentered("玩 家 状 态", cCyan);

	setColor(cYellow);
	std::cout << "\n\t\t名字：" << player.getPlayerName() << "\n";
	setColor(cWhite);
	std::cout << "\t\t等级：" << player.getLevel() << "\n";
	std::cout << "\t\t生命：" << player.getHp() << " / " << player.getMHp() << "\n";
	std::cout << "\t\t攻击：" << player.getAtk() << "\n";
	std::cout << "\t\t能量：" << player.getEnergy() << " / " << player.getMEnergy() << "\n";
	setColor(cCyan);
	std::cout << "\t\t经验：" << player.getExp() << "\n";
	std::cout << "\t\t金币：" << player.getGold() << "\n";
	setColor(cGreen);
	std::cout << "\t\t可用属性点：" << player.getAtp() << "\n";
	setColor(cWhite);
	printBottomLine();

	clearKeys();
	std::cout << "\n按任意键返回...\n";
	_getch();
	clearKeys();
}

void Game::saveGame()
{
	clearScreen();
	printTopLine();
	printCentered("保 存 游 戏", cCyan);
	if (saveManager.save(SAVE_FILE, player, rooms))
	{
		setColor(cGreen);
		std::cout << "\n\t\t游戏已保存。\n";
	}
	else
	{
		setColor(cRed);
		std::cout << "\n\t\t保存失败。\n";
	}
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
	if (saveManager.load(SAVE_FILE, player, rooms))
	{
		playerName_ = player.getPlayerName();
		setColor(cGreen);
		std::cout << "\n\t\t读档成功。\n";
	}
	else
	{
		setColor(cRed);
		std::cout << "\n\t\t没有可用的存档。\n";
	}
	setColor(cWhite);
	printBottomLine();
	clearKeys();
	_getch();
	clearKeys();
}
