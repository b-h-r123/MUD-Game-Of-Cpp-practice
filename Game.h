#pragma once
#include <string>
#include <vector>
#include"plot.h"
#include"battleSystem.h"
#include"Player.h"
#include"saveManager.h"
#include"map.h"
#include"Room.h"
class game :public Plot,BattleSystem,Player,saveManager,Map{
public:
	game();
	int game_id;
	void showStoryIntro();
	void player_start(std::string name);
	void chapter(int num);
	void showMainMenu();
	void playStory();
	void showVictory();
	bool gameOver(int currentHp);
	std::string name;
	int location;
	// 房间进度：与地图 move() 共享同一份状态，主线通关与地图解锁保持一致
	std::vector<Room> rooms;
	
};
