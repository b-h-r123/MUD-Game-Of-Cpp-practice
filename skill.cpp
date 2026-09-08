#include<iostream>
#include<string>
#include<vector>
#include"skill.h"


Skill::Skill(std::string skillName, int skillDamageRate, int skillEnergyCost, std::string skillDescription, SkillEffect skillEffect, int skillEffectValue)
	: name(skillName), damageRate(skillDamageRate), energyCost(skillEnergyCost), description(skillDescription), effect(skillEffect), effectValue(skillEffectValue) {}




