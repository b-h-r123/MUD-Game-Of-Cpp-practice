#pragma once
#include <string>
#include <vector>

// 技能特殊效果类型
enum class SkillEffect
{
	NONE,        // 无特殊效果
	SELF_DAMAGE, // 消耗自身血量
	SELF_HEAL,   // 自身回血
	ENEMY_DEBUFF_ATK, // 降低敌方攻击
	DEBUFF   //附加debuff
};

struct Skill
{
	std::string name;  //技能名称
	int damageRate;    //技能伤害倍率
	int energyCost;      //技能消耗蓝量
	std::string description;  //技能描述
	SkillEffect effect;  //效果类型
	int effectValue;    // 效果数值：扣多少血/回多少血
	Skill(std::string skillName, int skillDamageRate, int skillEnergyCost, std::string skillDescription, SkillEffect skillEffect, int skillEffectValue);
};

// 玩家 / 敌人技能表。定义放在 skill.cpp，避免头文件被多处包含时产生重复定义。
extern std::vector<Skill> PlayerSkillList;
extern std::vector<Skill> EnemySkillList;
