#pragma once
#include <string>
#include <vector>
#include "battleSystem.h"

// Enemy 继承 BattleSystem，因此拥有血量/攻击/能量与全部 debuff 接口。
// 敌人自身再补充名称、经验与金币奖励，以及整张敌人数据表 EnemyList。
class Enemy : public BattleSystem
{
public:
	Enemy(std::string name, int hp, int mhp, int atk, int expReward, int goldReward, bool isSkillHave);

	const std::string& getName() const;
	int getExpReward() const;
	int getGoldReward() const;
	bool getIsSkillHave() const;

	// 整张敌人数据表：按难度排列，Game 选关时按章节/房间挑一只副本进行战斗。
	static std::vector<Enemy> EnemyList;

	~Enemy() = default;

private:
	std::string name;
	int Expreward;
	int Goldreward;
	bool isSkillHave = false; //标记敌人是否有技能
};
