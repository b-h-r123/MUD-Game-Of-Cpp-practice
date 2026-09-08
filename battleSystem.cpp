#include "battleSystem.h"

#include <algorithm>
#include <iostream>
#include <random>
#include "Enemy.h"
#include "Player.h"

namespace
{
// 随机数生成器（匿名命名空间内，仅本文件可见）
std::mt19937 rng(std::random_device{}());
} // namespace

int getRandomInt(int min, int max)
{
	std::uniform_int_distribution<int> dist(min, max);
	return dist(rng);
}

BattleSystem::BattleSystem(int Hp, int MHp, int atk, int Energy, int Menergy)
	: b_Hp(Hp), b_MHp(MHp), b_atk(atk), b_Energy(Energy), b_Menergy(Menergy) {}

int BattleSystem::getHp() const { return b_Hp; }
int BattleSystem::getMHp() const { return b_MHp; }
int BattleSystem::getAtk() const { return b_atk; }
int BattleSystem::getEnergy() const { return b_Energy; }
int BattleSystem::getMEnergy() const { return b_Menergy; }

void BattleSystem::takedamage(int atk)
{
	b_Hp -= atk;
	if (b_Hp < 0) b_Hp = 0;
}

void BattleSystem::heal(int val)
{
	b_Hp += val;
	if (b_Hp > b_MHp) b_Hp = b_MHp;
}

void BattleSystem::useSkill(Skill& skill, BattleSystem* caster, BattleSystem* target)
{
	if (caster->b_Energy != 999) // 999 表示敌人不消耗能量
	{
		if (caster->b_Energy >= skill.energyCost)
		{
			caster->b_Energy -= skill.energyCost;
			std::cout << caster->b_Energy << " 能量剩余\n";
		}
		else
		{
			std::cout << "能量不足，无法释放技能！\n";
			return;
		}
	}
	int rawDamage = caster->b_atk * skill.damageRate;
	int outDmg = caster->calcDamageOutput(rawDamage);	//释放者电击修正
	int finalDmg = target->calcDamageReceive(outDmg);	//受击方防御修正
	target->takedamage(finalDmg);
	std::cout << "\n【" << skill.name << "】造成" << finalDmg << "伤害！\n";

	switch (skill.effect)
	{
	case SkillEffect::SELF_DAMAGE:
		caster->takedamage(skill.effectValue);
		std::cout << "释放代价：自身损失" << skill.effectValue << "血量\n";
		break;
	case SkillEffect::SELF_HEAL:
		caster->heal(skill.effectValue);
		std::cout << "自身恢复" << skill.effectValue << "血量\n";
		break;
	case SkillEffect::ENEMY_DEBUFF_ATK:
		target->b_atk -= skill.effectValue;
		std::cout << "目标攻击力降低" << skill.effectValue << "\n";
		break;
	case SkillEffect::DEBUFF:
		if (skill.effectValue == 1) // 中毒
		{
			target->applyPoison(3);
			std::cout << "目标中毒，持续3回合\n";
		}
		else if (skill.effectValue == 2) // 电击
		{
			target->applyShock(1);
			std::cout << "目标受到电击，下一次输出伤害降低5点\n";
		}
		else if (skill.effectValue == 3) // 防御
		{
			target->applyDefend();
			std::cout << "目标进入防御状态，下一次受伤伤害减半\n";
		}
		break;
	case SkillEffect::NONE:
	default:
		break;
	}
}

bool BattleSystem::isBattleOver() const { return b_Hp <= 0; }

void BattleSystem::applyPoison(int turn)
{
	m_poisoned = true;
	m_poisonTurn = turn;
}

void BattleSystem::applyShock(int turn)
{
	m_shocked = true;
	m_shockTurn = turn;
}

void BattleSystem::applyDefend()
{
	m_defending = true;
}

void BattleSystem::processStatusStartTurn()
{
	if (m_poisoned)
	{
		std::cout << "[中毒] 受到 5 点毒素伤害！\n";
		b_Hp -= 5;
		if (b_Hp < 0) b_Hp = 0;

		m_poisonTurn--;
		if (m_poisonTurn <= 0)
		{
			m_poisoned = false;
			std::cout << "毒素效果消退。\n";
		}
	}
}

int BattleSystem::calcDamageOutput(int rawDmg)
{
	int dmg = rawDmg;
	if (m_shocked)
	{
		dmg -= 5;
		std::cout << "[电击干扰] 本次输出伤害降低5点！\n";
		m_shocked = false;
		m_shockTurn = 0;
	}
	return std::max(0, dmg);
}

int BattleSystem::calcDamageReceive(int rawDmg)
{
	int dmg = rawDmg;
	if (m_defending)
	{
		dmg = dmg / 2;
		std::cout << "[防御] 伤害减半！\n";
		m_defending = false;
	}
	return std::max(0, dmg);
}

void BattleSystem::clearAllStatus()
{
	m_poisoned = false;
	m_poisonTurn = 0;
	m_shocked = false;
	m_shockTurn = 0;
	m_defending = false;
}

bool BattleSystem::battle(BattleSystem& p, BattleSystem& e)
{
	Player* player = dynamic_cast<Player*>(&p);
	Enemy* enemy = dynamic_cast<Enemy*>(&e);
	if (player == nullptr || enemy == nullptr)
	{
		std::cout << "类型转换失败！传入对象不是 Player/Enemy\n";
		return false;
	}

	// 开战前清掉上一场残留状态，确保每场战斗干净开局。
	player->clearAllStatus();
	enemy->clearAllStatus();

	int choose = 0;
	while (true)
	{
		// ===== 玩家回合 =====
		player->processStatusStartTurn();
		if (player->isBattleOver())
		{
			break;
		}

		std::cout << "\n你的血量：" << player->getHp() << "/" << player->getMHp()
			<< "  能量：" << player->getEnergy() << "/" << player->getMEnergy() << "\n";
		std::cout << "敌人血量：" << enemy->getHp() << "/" << enemy->getMHp() << "\n\n";
		std::cout << "1.普通攻击  2.使用技能  3.防御  ";
		std::cin >> choose;
		if (std::cin.fail())
		{
			std::cin.clear();
			std::cin.ignore(1024, '\n');
			choose = 0;
		}
		switch (choose)
		{
		case 1:
		{
			int raw = player->getAtk();
			int out = player->calcDamageOutput(raw);
			int finalDmg = enemy->calcDamageReceive(out);
			enemy->takedamage(finalDmg);
			std::cout << "普通攻击造成" << finalDmg << "伤害\n";
			break;
		}
		case 2:
		{
			std::cout << "选择你的技能：\n";
			for (size_t i = 0; i < PlayerSkillList.size(); ++i)
			{
				std::cout << "  [" << (i + 1) << "] " << PlayerSkillList[i].name
					<< "（" << PlayerSkillList[i].description << "）\n";
			}
			int skillNo = 0;
			std::cin >> skillNo;
			if (std::cin.fail())
			{
				std::cin.clear();
				std::cin.ignore(1024, '\n');
				skillNo = 0;
			}
			if (skillNo >= 1 && skillNo <= static_cast<int>(PlayerSkillList.size()))
			{
				useSkill(PlayerSkillList[skillNo - 1], &p, &e);
			}
			else
			{
				std::cout << "无效的技能选择，本回合跳过。\n";
			}
			break;
		}
		case 3:
			std::cout << "你进入防御状态，下一次受伤伤害减半。\n";
			player->applyDefend();
			break;
		default:
			std::cout << "无效指令，本回合跳过。\n";
			break;
		}

		// 玩家击杀敌人 → 胜利
		if (enemy->isBattleOver())
		{
			std::cout << "\n你赢了！\n";
			player->addExp(enemy->getExpReward());
			player->addGold(enemy->getGoldReward());
			return true;
		}

		// ===== 敌人回合 =====
		enemy->processStatusStartTurn();
		if (enemy->isBattleOver())
		{
			std::cout << "\n你赢了！\n";
			player->addExp(enemy->getExpReward());
			player->addGold(enemy->getGoldReward());
			return true;
		}

		int enemyMove = getRandomInt(1, 3);
		switch (enemyMove)
		{
		case 1:
		{
			int raw = enemy->getAtk();
			int out = enemy->calcDamageOutput(raw);
			int finalDmg = player->calcDamageReceive(out);
			player->takedamage(finalDmg);
			std::cout << "敌人普通攻击造成" << finalDmg << "伤害\n";
			break;
		}
		case 2:
			useSkill(EnemySkillList[getRandomInt(0, static_cast<int>(EnemySkillList.size()) - 1)], &e, &p);
			break;
		case 3:
			std::cout << "敌人进入防御状态\n";
			enemy->applyDefend();
			break;
		default:
			break;
		}

		// 玩家被击杀 → 失败
		if (player->isBattleOver())
		{
			std::cout << "\n你输了！\n";
			return false;
		}
	}
	return false;
}
