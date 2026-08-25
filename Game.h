#pragma once
#include <string>
// 展示载入游戏后的背景剧情介绍
class game {
public:
	int game_id;
	void showStoryIntro();
	void player_start(std::string name);
	void showbag()const ;
	void show_state()const;
	std::string name;

	
};
