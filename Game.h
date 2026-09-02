#pragma once
#include <string>
#include"plot.h"
#include"battleSystem.h"
#include"Player.h"
#include"saveManager.h"
#include"map.h"
class game :public Plot,BattleSystem,Player,saveManager,Map{
public:
	int game_id;
	void showStoryIntro();
	void player_start(std::string name);
	void chapter(int num);
	void showMainMenu();
	std::string name;
	int location;
	
};
