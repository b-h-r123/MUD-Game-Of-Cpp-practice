#include<iostream>
#include<string>
#include<random>
#include<algorithm>
#include"battleSystem.h"
#include<vector>
#include"skill.h"
#include"Enemy.h"
#include"Player.h"

//随机数生成器
std::mt19937 rng(std::random_device{}());
int getRandomInt(int min, int max) 
{
	std::uniform_int_distribution<int> dist(min, max);
	return dist(rng);
}


//初步构造函数，初始化战斗系统的基本属性
BattleSystem::BattleSystem(int Hp, int MHp, int atk, int Energy, int Menergy) : b_Hp(Hp), b_MHp(MHp), b_atk(atk), b_Energy(Energy), b_Menergy(Menergy) {}
//获取实时属性
int BattleSystem::getHp() const { return b_Hp; }
int BattleSystem::getMHp() const { return b_MHp; }
int BattleSystem::getAtk() const { return b_atk; }
//受到伤害
void BattleSystem::takedamage(int atk)
{
	b_Hp -= atk;
	if (b_Hp < 0) b_Hp = 0;
}
//回血
void BattleSystem::heal(int val)
{
	b_Hp += val;
	if (b_Hp > b_MHp) b_Hp = b_MHp;
}
//技能攻击
void BattleSystem::useSkill(Skill& skill, BattleSystem* caster, BattleSystem* target)
{
	if (caster->b_Energy != 999) //999表示敌人不消耗蓝量
	{
		if (caster->b_Energy >= skill.energyCost)
		{
			caster->b_Energy -= skill.energyCost;
			std::cout << caster->b_Energy << "蓝量剩余\n";
		}
		else
		{
			std::cout << "蓝量不足，无法释放技能！\n";
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
		//这里可以根据技能的effectValue来决定施加哪种debuff
		if (skill.effectValue == 1) //假设1代表中毒
		{
			target->applyPoison(3); //中毒3回合
			std::cout << "目标中毒，持续3回合\n";
		}
		else if (skill.effectValue == 2) //假设2代表电击
		{
			target->applyShock(1); //电击1回合
			std::cout << "目标受到电击，下一次输出伤害降低5点\n";
		}
		else if (skill.effectValue == 3) //假设3代表防御
		{
			target->applyDefend(); //开启防御
			std::cout << "目标进入防御状态，下一次受伤伤害减半\n";
		}
		break;
	case SkillEffect::NONE:
	default:
		break;
	}
}

//判断是否结束战斗
bool BattleSystem::isBattleOver() const { return b_Hp <= 0; }

//施加中毒
void BattleSystem::applyPoison(int turn)
{
	m_poisoned = true;
	m_poisonTurn = turn;
}

//施加电击
void BattleSystem::applyShock(int turn)
{
	m_shocked = true;
	m_shockTurn = turn;
}

//敌人开启防御
void BattleSystem::applyDefend()
{
	m_defending = true;
}

// ========== 回合开始时调用：处理中毒 ==========
void BattleSystem::processStatusStartTurn()
{
	//中毒：回合开始扣血
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

// ========== 计算【我方打出去】的伤害：电击效果 ==========
// rawDmg：原始伤害，返回修正后伤害；电击：下一次输出伤害-5，用完就解除
int BattleSystem::calcDamageOutput(int rawDmg)
{
	int dmg = rawDmg;
	if (m_shocked)
	{
		dmg -= 5;
		std::cout << "[电击干扰] 本次输出伤害降低5点！\n";
		m_shocked = false; //电击只生效一次输出，直接清除
		m_shockTurn = 0;
	}
	return std::max(0, dmg); //伤害不能小于0
}

// ========== 计算【我受到】的伤害：敌人防御减伤 ==========
int BattleSystem::calcDamageReceive(int rawDmg)
{
	int dmg = rawDmg;
	if (m_defending)
	{
		dmg = dmg / 2;
		std::cout << "[防御] 伤害减半！\n";
		m_defending = false; //防御只挡下一次伤害就消失
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

//战斗过程
void BattleSystem::battle(BattleSystem& p, BattleSystem& e)
{
	Player* player = dynamic_cast<Player*>(&p);
	Enemy* enemy = dynamic_cast<Enemy*>(&e);
	if (player == nullptr || enemy == nullptr)
	{
		std::cout << "类型转换失败！传入对象不是Player/Enemy\n";
		return;
	}
	int i = 1;
	while (true)
	{
		//玩家行动
		//回合开始处理状态
		player->processStatusStartTurn();
		std::cout << "1.普通攻击  2.使用技能  3.使用道具" << std::endl;
		std::cin >> i;
		switch (i)
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
			std::cout << "选择你的技能" << std::endl;
			int choose = 0;
			std::cin >> choose;
			choose++;
			choose=choose % (PlayerSkillList.size());
			useSkill(PlayerSkillList[choose], &p, &e);
			break;
		}
		case 3:
			std::cout << "使用道具功能尚未实现" << std::endl;
			break;
		}
		//敌人行动
		enemy->processStatusStartTurn();
		i = getRandomInt(1, 3);
		switch (i)
		{
		case 1:
		{
			int raw = enemy->getAtk();
			int out = enemy->calcDamageOutput(raw);
			int finalDmg = player->calcDamageReceive(out);
			player->takedamage(finalDmg);
			break;
		}
		case 2:
			useSkill(EnemySkillList[getRandomInt(0,3)], &e, &p);
			break;
		case 3:
			std::cout << "使用防御" << std::endl;
			enemy->applyDefend();
			break;
		}
		//判断战斗是否结束
		if (player->isBattleOver())
		{
			std::cout << "你输了！" << std::endl;
			break;
		}
		else if (enemy->isBattleOver())
		{
			std::cout << "你赢了！" << std::endl;
			player->addExp(enemy->getExpReward());
			player->addGold(enemy->getGoldReward());
			break;
		}
	}
}