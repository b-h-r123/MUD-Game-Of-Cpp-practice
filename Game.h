#pragma once
#include <string>
#include"plot.h"
#include"battleSystem.h"
#include"Room.h"
#include"Player.h"
#include"saveManager.h"
class game :public Plot,BattleSystem,Room,Player,saveManager{
public:
	int game_id;
	void showStoryIntro();
	void player_start(std::string name);
	void chapter(int num);
	void showMainMenu();
	std::string name;
	int location;
	
};
