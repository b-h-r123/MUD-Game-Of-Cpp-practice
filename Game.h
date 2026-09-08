#pragma once

#include <string>
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
	void chooseRoom();				// 选关进入（网格地图自由探索）
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
	// 主线剧情：从第 1 章一路推进到 Boss 战。
	void playMainStory();
	// 单个章节：播放章节前后剧情，并触发对应房间的战斗。
	// 返回 true 表示本章打通，false 表示玩家阵亡。
	bool chapter(int num);
	// 章节结束后给玩家选择的“游戏内菜单”。
	void showInGameMenu();
	// 战斗胜利后的结算：按章节解锁对应的主线房间。
	void unlockRoomAfterChapter(int num);
	std::string playerName_;		// 本局玩家名，用于替换剧情中的 {{name}}
	Player player;
	std::vector<Room> rooms;
	SaveManager saveManager;
	int roomNum = 1;
	bool gaming = true;
};
