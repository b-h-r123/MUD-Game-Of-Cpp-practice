#pragma once
#include<iostream>
#include<string>
#include"battleSystem.h"

class Player :public BattleSystem
{
	//TODO： 添加玩家属性、技能、物品等功能，包括玩家的状态管理等
public:
	Player();
	void inputPlayername();
	void addExp(int val);
	void addGold(int val);
	void checkLevelUp();
	~Player() = default;
private:
	std::string name;
	unsigned int level=1;
	int Exp;//经验
	int Gold;//金钱
	int Atp;//属性点（attributepoint）
	std::string Iny;//背包（inventory）
	std::string S;//技能(skill)
};