#pragma once

#include <vector>
#include "Enemy.h"
#include "Player.h"
#include "Room.h"
#include "battleSystem.h"
#include "saveManager.h"

class Game
{
public:
	Game();							// 构造函数
	void start();					// 游戏开始主循环
	void mainMenu();				// 游戏主菜单（开始新游戏、加载旧游戏、退出游戏）
	void startNewGame();			// 开始新游戏
	void loadingOldGame();			// 加载旧游戏
	void chooseRoom();				// 选关进入
	void enterNowRoom(int roomNum);	// 进入当前关卡
	void victory();					// 胜利结局
	void defeat();					// 失败结局
	void showShop();				// 商店界面
	void showUpgrade();				// 强化界面
	void showBag();					// 背包界面
	void showPlayerState();			// 玩家状态界面
	void saveGame();				// 保存游戏
	void loadGame();				// 读取游戏
private:
	Player player;
	std::vector<Room> rooms;
	BattleSystem battleSystem;
	SaveManager saveManager;
	int roomNum = 1;
	bool gaming = true;
};
