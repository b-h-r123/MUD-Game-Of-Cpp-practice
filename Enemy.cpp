#include "Enemy.h"

// 能量给 999 是刻意的：表示敌人“不消耗能量”，可在 useSkill 中无限放技能。
Enemy::Enemy(std::string name, int hp, int mhp, int atk, int expReward, int goldReward, bool isSkillHave)
	: BattleSystem(hp, mhp, atk, 999, 999),
	  name(name),
	  Expreward(expReward),
	  Goldreward(goldReward),
	  isSkillHave(isSkillHave) {}

const std::string& Enemy::getName() const
{
	return name;
}

int Enemy::getExpReward() const
{
	return Expreward;
}

int Enemy::getGoldReward() const
{
	return Goldreward;
}

bool Enemy::getIsSkillHave() const
{
	return isSkillHave;
}

// 敌人表（与最初设计数值一致）：名称 | HP | ATK | EXP | GOLD
std::vector<Enemy> Enemy::EnemyList =
{
	Enemy("废土鼠",      30,  30,  8,  15,  20,  false),
	Enemy("街头猎犬",    45,  45,  11, 20,  30,  false),
	Enemy("改造人",      65,  65,  15, 35,  50,  true),
	Enemy("黑客佣兵",    75,  75,  17, 50,  90,  true),
	Enemy("攻击无人机",  55,  55,  18, 40,  60,  false),
	Enemy("重装保镖",    100, 100, 20, 60,  80,  true),
	Enemy("企业猎杀者",  130, 130, 25, 100, 150, true),
	Enemy("NEON - X",   300, 300, 30, 500, 500, true)
};
