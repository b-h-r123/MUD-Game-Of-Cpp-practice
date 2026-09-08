#pragma once
#include<iostream>
#include<string>
#include<vector>
#include"battleSystem.h"

class Enemy : public BattleSystem
{
	//TODO: 添加敌人属性、技能、掉落物品等功能，包括敌人的生成、行为和管理等
public:
	Enemy(std::string name, int hp, int mhp, int atk, int Expreward, int Goldreward, bool isSkillHave);
	int getExpReward() const;
	int getGoldReward() const;
	static std::vector<Enemy> EnemyList;
	~Enemy() = default;
private:
	std::string name;
	int Expreward;
	int Goldreward;
	bool isSkillHave = false; //标记敌人是否有技能
};