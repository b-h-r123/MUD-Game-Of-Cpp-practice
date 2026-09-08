#pragma once
#include<iostream>
#include<string>
#include<vector>

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

std::vector<Skill> PlayerSkillList =
{
	Skill("电磁脉冲", 150, 2, "小技能。", SkillEffect::NONE, 0),
	Skill("义体过载", 200, 3, "伤敌一千，自损八百。", SkillEffect::SELF_DAMAGE, 800),
};

std::vector<Skill> EnemySkillList =
{
	Skill("电击", 100, 0, "附加电击效果。", SkillEffect::DEBUFF, 2),//最后的参数2表示附加电击debuff，是skillEffectValue的值
	Skill("中毒攻击", 100, 0, "附加中毒效果。", SkillEffect::DEBUFF, 1),
	Skill("防御姿态", 0, 0, "降低下一次受到的伤害。", SkillEffect::DEBUFF, 3),
	Skill("破绷一击", 100, 0, "降低敌方攻击力。", SkillEffect::ENEMY_DEBUFF_ATK, 5),
};
