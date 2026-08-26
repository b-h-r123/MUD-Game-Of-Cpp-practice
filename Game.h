#pragma once

#include<vector>
#include "Enemy.h"
#include "Player.h"
#include "Room.h"
#include "battleSystem.h"
#include "saveManager.h"

class Game
{
public:
	Game(); //构造函数
	void start();	//游戏开始与运行
	void mainMenu();	//游戏主菜单（开始新游戏、加载旧游戏、退出游戏）	
	void startNewGame();	//开始新游戏
	void loadingOldGame();	//加载旧游戏
	void chooseRoom();	//选关界面
	void enterNowRoom(int roomNum);		//进入当前关卡
	void victory();	//胜利界面
	void defeat();	//失败界面
	void showShop();	//商店界面
	void showUpgrade();	//升级界面
	void showBag();	//背包界面
	void showPlayerState();	//玩家状态界面
	void saveGame();	//保存游戏
	void loadGame();	//加载游戏
	//TODO: 添加游戏开始、加载等功能，包括加载进入菜单等
private:
	Player player; 
	std::vector<Room> rooms;
	BattleSystem battleSystem;
	SaveManager saveManager;
	int roomNum = 1;
	bool gaming = true;
};
