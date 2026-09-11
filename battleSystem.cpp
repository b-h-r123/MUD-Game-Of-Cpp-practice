#include "battleSystem.h"

#include <algorithm>
#include <iostream>
#include <random>
#include <cstdlib>
#include <conio.h>

#define NOMINMAX
#include <windows.h>

#include "Enemy.h"
#include "Player.h"

namespace
{
// 随机数生成器（匿名命名空间内，仅本文件可见）
std::mt19937 rng(std::random_device{}());

// ============================ 控制台 UI 辅助 ============================
// 这些函数只负责显示效果，不参与任何战斗数值计算。

void clearScreen()
{
	system("cls");
}

// 战斗结算后：先停留让玩家看清胜负提示，按键后再清屏。
void pauseThenClear()
{
	std::cout << "\n  按任意键继续...";
	std::cout.flush();
	_getch();
	clearScreen();
}

void setTextColor(int color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<WORD>(color));
}

void resetTextColor()
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x07);
}

// 画一条固定 20 格的 HP 条，颜色随剩余血量比例变化（绿/黄/红）。
void drawHpBar(const char* label, int current, int maxHp)
{
	if (maxHp <= 0)
	{
		maxHp = 1;
	}
	const int barWidth = 20;
	int filled = current * barWidth / maxHp;
	if (filled < 0)
	{
		filled = 0;
	}
	if (filled > barWidth)
	{
		filled = barWidth;
	}

	const int percent = current * 100 / maxHp;
	int color = 0x0A; // 绿色
	if (percent < 30)
	{
		color = 0x0C; // 红色
	}
	else if (percent < 60)
	{
		color = 0x0E; // 黄色
	}

	std::cout << "  " << label;
	setTextColor(color);
	std::cout << " [";
	for (int i = 0; i < barWidth; ++i)
	{
		std::cout << (i < filled ? "█" : "░");
	}
	std::cout << "] ";
	resetTextColor();
	std::cout << current << "/" << maxHp << "\n";
}
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
	if (b_Hp > getMHp()) b_Hp = getMHp();
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

	// 技能伤害基于有效攻击（含装备加成）。
	int rawDamage = caster->getAtk() * skill.damageRate;
	int outDmg = caster->calcDamageOutput(rawDamage);

	// 玩家装备的“技能增伤 / 斩杀”被动在此结算。
	Player* casterPlayer = dynamic_cast<Player*>(caster);
	if (casterPlayer != 0)
	{
		const PassiveEffect* skillBonus = casterPlayer->findPassive(PassiveType::SKILL_DAMAGE_BONUS);
		if (skillBonus != 0)
		{
			outDmg = outDmg * (100 + skillBonus->value) / 100;
		}
		const PassiveEffect* execute = casterPlayer->findPassive(PassiveType::EXECUTE_DAMAGE);
		if (execute != 0 && execute->thresholdPercent > 0 &&
			target->getHp() * 100 <= target->getMHp() * execute->thresholdPercent)
		{
			outDmg = outDmg * (100 + execute->value) / 100;
		}
	}

	int finalDmg = target->calcDamageReceive(outDmg);
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

	// 每场战斗开始：清状态、重置“每场一次”被动与临时效果。
	player->clearAllStatus();
	enemy->clearAllStatus();
	player->beginBattle();

	// 进入战斗：先清屏，再显示战斗横幅与双方状态。
	clearScreen();
	setTextColor(0x0D); // 亮洋红
	std::cout << "\n  ============================================\n";
	std::cout << "               战 斗 开 始\n";
	std::cout << "  ============================================\n";
	resetTextColor();
	std::cout << "\n  遭遇了「" << enemy->getName() << "」！\n\n";
	drawHpBar("你的 HP", player->getHp(), player->getMHp());
	drawHpBar("敌人 HP", enemy->getHp(), enemy->getMHp());
	std::cout << "\n  你的能量：" << player->getEnergy() << " / " << player->getMEnergy() << "\n\n";

	int choose = 0;
	while (true)
	{
		// ===== 玩家回合 =====
		player->processTurnStartEffects();
		player->processStatusStartTurn();
		if (player->isBattleOver())
		{
			break;
		}

		drawHpBar("你的 HP", player->getHp(), player->getMHp());
		drawHpBar("敌人 HP", enemy->getHp(), enemy->getMHp());
		std::cout << "\n  你的能量：" << player->getEnergy() << " / " << player->getMEnergy() << "\n\n";
		std::cout << "  1.普通攻击  2.使用技能  3.使用道具  4.防御  ";
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

			const PassiveEffect* execute = player->findPassive(PassiveType::EXECUTE_DAMAGE);
			if (execute != 0 && execute->thresholdPercent > 0 &&
				enemy->getHp() * 100 <= enemy->getMHp() * execute->thresholdPercent)
			{
				out = out * (100 + execute->value) / 100;
			}

			int finalDmg = enemy->calcDamageReceive(out);
			enemy->takedamage(finalDmg);
			std::cout << "普通攻击造成" << finalDmg << "伤害\n";

			// 电弧链刃：命中后追加一次伤害。
			const PassiveEffect* followUp = player->findPassive(PassiveType::FOLLOW_UP_DAMAGE);
			if (followUp != 0 && !enemy->isBattleOver())
			{
				enemy->takedamage(followUp->value);
				std::cout << "[电弧链刃] 追加 " << followUp->value << " 点伤害！\n";
			}
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
				// 拷贝一份，避免修改全局技能表；同时应用“首次技能减耗”被动。
				Skill chosen = PlayerSkillList[skillNo - 1];
				const PassiveEffect* costReduction =
					player->findPassive(PassiveType::FIRST_SKILL_COST_REDUCTION);
				if (costReduction != 0 &&
					player->consumeOncePassive(PassiveType::FIRST_SKILL_COST_REDUCTION))
				{
					chosen.energyCost = std::max(0, chosen.energyCost - costReduction->value);
					std::cout << "[零点电容] 本次技能能量消耗降低 " << costReduction->value << "。\n";
				}
				useSkill(chosen, player, enemy);
			}
			else
			{
				std::cout << "无效的技能选择，本回合跳过。\n";
			}
			break;
		}
		case 3:
		{
			// 列出四个固定消耗品槽里当前可用的道具。
			const std::array<ConsumableSlot, 4>& consumables = player->getItemSlots().getConsumableSlots();
			std::cout << "选择要使用的道具：\n";
			int index = 1;
			int slotOf[4] = {-1, -1, -1, -1};
			for (size_t i = 0; i < consumables.size(); ++i)
			{
				if (consumables[i].getCount() > 0)
				{
					const Item* def = player->getCatalog().findById(consumables[i].getItemId());
					if (def != 0)
					{
						slotOf[index - 1] = static_cast<int>(i);
						std::cout << "  [" << index << "] " << def->getName()
							<< " x" << consumables[i].getCount() << "\n";
						++index;
					}
				}
			}
			if (index == 1)
			{
				std::cout << "  没有可用的道具。\n";
				break;
			}
			int itemNo = 0;
			std::cin >> itemNo;
			if (std::cin.fail())
			{
				std::cin.clear();
				std::cin.ignore(1024, '\n');
				itemNo = 0;
			}
			if (itemNo >= 1 && itemNo < index)
			{
				const int realSlot = slotOf[itemNo - 1];
				const ItemId usedId = consumables[realSlot].getItemId();
				if (player->useConsumable(usedId))
				{
					const Item* def = player->getCatalog().findById(usedId);
					std::cout << "使用了 " << (def != 0 ? def->getName() : "道具") << "。\n";
				}
				else
				{
					std::cout << "道具使用失败。\n";
				}
			}
			else
			{
				std::cout << "无效的道具选择，本回合跳过。\n";
			}
			break;
		}
		case 4:
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
			setTextColor(0x0A);
			std::cout << "\n  你赢了！\n";
			resetTextColor();
			player->addExp(enemy->getExpReward());
			player->addGold(enemy->getGoldReward());
			pauseThenClear();
			return true;
		}

		// ===== 敌人回合 =====
		enemy->processStatusStartTurn();
		if (enemy->isBattleOver())
		{
			setTextColor(0x0A);
			std::cout << "\n  你赢了！\n";
			resetTextColor();
			player->addExp(enemy->getExpReward());
			player->addGold(enemy->getGoldReward());
			pauseThenClear();
			return true;
		}

		int enemyMove = getRandomInt(1, 3);
		int incomingDamage = 0;
		switch (enemyMove)
		{
		case 1:
		{
			int raw = enemy->getAtk();
			int out = enemy->calcDamageOutput(raw);
			incomingDamage = player->calcDamageReceive(out);
			break;
		}
		case 2:
		{
			// 敌人技能：直接走 useSkill，它内部已处理伤害与 debuff。
			useSkill(EnemySkillList[getRandomInt(0, static_cast<int>(EnemySkillList.size()) - 1)], enemy, player);
			incomingDamage = 0;
			break;
		}
		case 3:
			std::cout << "敌人进入防御状态\n";
			enemy->applyDefend();
			incomingDamage = 0;
			break;
		default:
			break;
		}

		if (incomingDamage > 0)
		{
			// 神盾装甲：每场战斗首次致命伤害时保留 1 点生命。
			const PassiveEffect* guard = player->findPassive(PassiveType::LETHAL_GUARD);
			if (guard != 0 && player->getHp() - incomingDamage <= 0 &&
				player->consumeOncePassive(PassiveType::LETHAL_GUARD))
			{
				const int lethal = player->getHp() > 0 ? player->getHp() - 1 : 0;
				player->takedamage(lethal);
				std::cout << "[神盾装甲] 致命伤害被抵挡，保留 1 点生命！\n";
			}
			else
			{
				player->takedamage(incomingDamage);
				std::cout << "敌人普通攻击造成" << incomingDamage << "伤害\n";
			}

			// 镜面反射盾：反弹部分最终承受伤害。
			const PassiveEffect* reflect = player->findPassive(PassiveType::REFLECT_DAMAGE);
			if (reflect != 0 && incomingDamage > 0 && !enemy->isBattleOver())
			{
				const int reflected = incomingDamage * reflect->value / 100;
				if (reflected > 0)
				{
					enemy->takedamage(reflected);
					std::cout << "[镜面反射盾] 反弹 " << reflected << " 点伤害！\n";
				}
			}
		}

		// 玩家被击杀 → 失败
		if (player->isBattleOver())
		{
			setTextColor(0x0C);
			std::cout << "\n  你输了！\n";
			resetTextColor();
			pauseThenClear();
			return false;
		}
	}
	return false;
}
