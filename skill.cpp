#include <string>
#include <vector>
#include "skill.h"

Skill::Skill(std::string skillName, int skillDamageRate, int skillEnergyCost, std::string skillDescription, SkillEffect skillEffect, int skillEffectValue)
	: name(skillName), damageRate(skillDamageRate), energyCost(skillEnergyCost), description(skillDescription), effect(skillEffect), effectValue(skillEffectValue) {}

std::vector<Skill> PlayerSkillList =
{
	Skill("电磁脉冲", 150, 2, "对敌人造成 1.5 倍攻击伤害。", SkillEffect::NONE, 0),
	Skill("义体过载", 200, 3, "造成 2 倍伤害，自身损失 800 血。", SkillEffect::SELF_DAMAGE, 800),
};

std::vector<Skill> EnemySkillList =
{
	Skill("电击", 100, 0, "附加电击效果。", SkillEffect::DEBUFF, 2),//最后的参数2表示附加电击debuff，是skillEffectValue的值
	Skill("中毒攻击", 100, 0, "附加中毒效果。", SkillEffect::DEBUFF, 1),
	Skill("防御姿态", 0, 0, "降低下一次受到的伤害。", SkillEffect::DEBUFF, 3),
	Skill("破绷一击", 100, 0, "降低敌方攻击力。", SkillEffect::ENEMY_DEBUFF_ATK, 5),
};
