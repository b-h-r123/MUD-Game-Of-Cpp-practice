#include "Player.h"

#include <algorithm>
#include <iostream>
#include <sstream>

Player::Player()
	: BattleSystem(100, 100, 20, 5, 5),
	  Gold(100),
	  Atp(0),
	  catalog_(),
	  slots_(),
	  upgrade_()
{
}

// ============================ 档案 ============================
void Player::setPlayerName(const std::string& newName)
{
	name = newName;
}

const std::string& Player::getPlayerName() const
{
	return name;
}

int Player::getGold() const
{
	return Gold;
}

int Player::getAtp() const
{
	return Atp;
}

void Player::addGold(int val)
{
	if (val > 0)
	{
		Gold += val;
	}
}

bool Player::spendGold(int val)
{
	if (val < 0 || Gold < val)
	{
		return false;
	}
	Gold -= val;
	return true;
}

// ======================= 等级 / 经验（Upgrade）=======================
int Player::getLevel() const
{
	return upgrade_.getLevel();
}

int Player::getExp() const
{
	return upgrade_.getExperience();
}

int Player::getRequiredExpForNextLevel() const
{
	return upgrade_.getRequiredExperienceForNextLevel();
}

void Player::addExp(int val)
{
	// Upgrade 要求 currentHp <= maxHp；装备生命加成可能让当前 HP 超过基础上限，
	// 这里先把参与结算的当前 HP 夹到基础上限，避免误判为非法状态。
	int hpForUpgrade = std::min(b_Hp, b_MHp);
	int gained = 0;
	if (upgrade_.addExperience(val, catalog_, hpForUpgrade, b_MHp, b_atk, gained))
	{
		b_Hp = hpForUpgrade;
		// 与“两者都保留”的取舍一致：自动芯片之外，每升一级再发 1 点属性点。
		Atp += gained;
	}
}

// ========================= 属性点手动分配 =========================
bool Player::allocateAttack()
{
	if (Atp <= 0)
	{
		return false;
	}
	b_atk += 5;
	--Atp;
	return true;
}

bool Player::allocateMaxHp()
{
	if (Atp <= 0)
	{
		return false;
	}
	b_MHp += 20;
	b_Hp += 20;
	--Atp;
	return true;
}

// ====================== 有效属性（基础 + 装备）======================
const Item* Player::findEquippedItem(EquipmentSlot slot) const
{
	const ItemId id = slots_.getEquippedItem(slot);
	if (id == ItemId::NONE)
	{
		return 0;
	}
	return catalog_.findById(id);
}

int Player::sumEquippedBonus(int EquipmentData::*field) const
{
	static const EquipmentSlot slots[3] = {
		EquipmentSlot::WEAPON,
		EquipmentSlot::SHIELD,
		EquipmentSlot::ACCESSORY};

	int total = 0;
	for (int i = 0; i < 3; ++i)
	{
		const Item* item = findEquippedItem(slots[i]);
		if (item != 0 && item->isEquipment())
		{
			total += item->getEquipmentData().*field;
		}
	}
	return total;
}

int Player::getAtk() const
{
	return b_atk + sumEquippedBonus(&EquipmentData::attackBonus);
}

int Player::getMHp() const
{
	return b_MHp + sumEquippedBonus(&EquipmentData::maxHpBonus);
}

int Player::getMEnergy() const
{
	return b_Menergy + sumEquippedBonus(&EquipmentData::maxEnergyBonus);
}

int Player::getEffectiveAtk() const
{
	return getAtk();
}

int Player::getEffectiveMaxHp() const
{
	return getMHp();
}

int Player::getEffectiveMaxEnergy() const
{
	return getMEnergy();
}

int Player::getFlatDamageReduction() const
{
	return sumEquippedBonus(&EquipmentData::flatDamageReduction);
}

// =========================== 背包 / 装备 ===========================
ItemSlots& Player::getItemSlots()
{
	return slots_;
}

const ItemSlots& Player::getItemSlots() const
{
	return slots_;
}

ItemCatalog& Player::getCatalog()
{
	return catalog_;
}

const ItemCatalog& Player::getCatalog() const
{
	return catalog_;
}

Upgrade& Player::getUpgrade()
{
	return upgrade_;
}

bool Player::receiveItem(const Item& item)
{
	if (!item.isValid())
	{
		return false;
	}

	if (item.isConsumable())
	{
		return slots_.addConsumable(item.getId(), 1);
	}

	if (item.isDirectUpgrade())
	{
		int hpForChip = std::min(b_Hp, b_MHp);
		if (!upgrade_.applyChip(item, hpForChip, b_MHp, b_atk))
		{
			return false;
		}
		b_Hp = hpForChip;
		return true;
	}

	if (item.isEquipment())
	{
		return slots_.replaceEquipment(item);
	}

	return false;
}

bool Player::useConsumable(ItemId id)
{
	const Item* item = catalog_.findById(id);
	if (item == 0 || !item->isConsumable())
	{
		return false;
	}
	if (slots_.findConsumableSlot(id) == 0)
	{
		return false;
	}

	// 先应用效果，再消耗数量，保证失败时数量不变。
	const std::vector<ItemEffect>& effects = item->getEffects();
	for (std::vector<ItemEffect>::const_iterator it = effects.begin(); it != effects.end(); ++it)
	{
		if (!it->isValid())
		{
			return false;
		}

		switch (it->type)
		{
		case EffectType::RESTORE_HP:
			heal(it->value);
			break;
		case EffectType::RESTORE_ENERGY:
			b_Energy += it->value;
			if (b_Energy > getMEnergy())
			{
				b_Energy = getMEnergy();
			}
			break;
		default:
			return false;
		}

		// 持续效果：本回合先立即生效一次，剩余回合挂到临时效果里。
		if (it->isTemporary() && it->durationTurns > 1)
		{
			TempEffect temp;
			temp.type = it->type;
			temp.value = it->value;
			temp.remainingTurns = it->durationTurns - 1;
			tempEffects_.push_back(temp);
		}
	}

	return slots_.consumeConsumable(id);
}

// =========================== 战斗被动 ===========================
const PassiveEffect* Player::findPassive(PassiveType type) const
{
	static const EquipmentSlot slots[3] = {
		EquipmentSlot::WEAPON,
		EquipmentSlot::SHIELD,
		EquipmentSlot::ACCESSORY};

	for (int i = 0; i < 3; ++i)
	{
		const Item* item = findEquippedItem(slots[i]);
		if (item != 0 && item->isEquipment())
		{
			const PassiveEffect& passive = item->getEquipmentData().passive;
			if (passive.isActive() && passive.type == type)
			{
				return &passive;
			}
		}
	}
	return 0;
}

bool Player::consumeOncePassive(PassiveType type)
{
	if (std::find(usedOncePassives_.begin(), usedOncePassives_.end(), type) != usedOncePassives_.end())
	{
		return false;
	}
	usedOncePassives_.push_back(type);
	return true;
}

void Player::beginBattle()
{
	usedOncePassives_.clear();
	tempEffects_.clear();
}

void Player::processTurnStartEffects()
{
	for (std::vector<TempEffect>::iterator it = tempEffects_.begin(); it != tempEffects_.end();)
	{
		if (it->type == EffectType::RESTORE_HP)
		{
			heal(it->value);
			std::cout << "[持续恢复] 恢复 " << it->value << " 点 HP。\n";
		}
		else if (it->type == EffectType::RESTORE_ENERGY)
		{
			b_Energy += it->value;
			if (b_Energy > getMEnergy())
			{
				b_Energy = getMEnergy();
			}
			std::cout << "[持续恢复] 恢复 " << it->value << " 点 Energy。\n";
		}

		--it->remainingTurns;
		if (it->remainingTurns <= 0)
		{
			it = tempEffects_.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void Player::restoreToFull()
{
	b_Hp = getMHp();
	b_Energy = getMEnergy();
	tempEffects_.clear();
}

void Player::restoreEnergyToFull()
{
	b_Energy = getMEnergy();
	tempEffects_.clear();
}

// ============================ 存档 ============================
std::string Player::serialize() const
{
	std::ostringstream oss;
	oss << name << ' '
		<< upgrade_.getLevel() << ' '
		<< upgrade_.getExperience() << ' '
		<< Gold << ' '
		<< Atp << ' '
		<< b_Hp << ' '
		<< b_MHp << ' '
		<< b_atk << ' '
		<< b_Energy << ' '
		<< b_Menergy << ' ';

	const std::array<ConsumableSlot, 4>& consumables = slots_.getConsumableSlots();
	for (std::size_t i = 0; i < consumables.size(); ++i)
	{
		oss << consumables[i].getCount() << ' ';
	}

	oss << static_cast<int>(slots_.getEquippedItem(EquipmentSlot::WEAPON)) << ' '
		<< static_cast<int>(slots_.getEquippedItem(EquipmentSlot::SHIELD)) << ' '
		<< static_cast<int>(slots_.getEquippedItem(EquipmentSlot::ACCESSORY));

	return oss.str();
}

bool Player::deserialize(const std::string& data)
{
	std::istringstream iss(data);
	std::string loadedName;
	int level = 1;
	int exp = 0;
	int gold = 0;
	int atp = 0;
	int hp = 0;
	int mhp = 0;
	int atk = 0;
	int energy = 0;
	int menergy = 0;
	int counts[4] = {0, 0, 0, 0};
	int weapon = 0;
	int shield = 0;
	int accessory = 0;

	if (!(iss >> loadedName >> level >> exp >> gold >> atp
		>> hp >> mhp >> atk >> energy >> menergy
		>> counts[0] >> counts[1] >> counts[2] >> counts[3]
		>> weapon >> shield >> accessory))
	{
		return false;
	}

	if (mhp <= 0 || menergy <= 0 || atk < 0 || hp < 0 || energy < 0 || gold < 0 || atp < 0)
	{
		return false;
	}
	if (!upgrade_.setState(level, exp))
	{
		return false;
	}

	name = loadedName;
	Gold = gold;
	Atp = atp;
	b_Hp = std::min(hp, mhp);
	b_MHp = mhp;
	b_atk = atk;
	b_Energy = std::min(energy, menergy);
	b_Menergy = menergy;

	slots_ = ItemSlots();
	slots_.setConsumableCount(ItemId::MEDKIT, counts[0]);
	slots_.setConsumableCount(ItemId::ENERGY_CELL, counts[1]);
	slots_.setConsumableCount(ItemId::NANO_MEDKIT, counts[2]);
	slots_.setConsumableCount(ItemId::SUPERCONDUCTING_CORE, counts[3]);

	const int equipmentIds[3] = { weapon, shield, accessory };
	const EquipmentSlot equipmentSlots[3] = {
		EquipmentSlot::WEAPON, EquipmentSlot::SHIELD, EquipmentSlot::ACCESSORY };
	for (int i = 0; i < 3; ++i)
	{
		if (equipmentIds[i] != static_cast<int>(ItemId::NONE))
		{
			const Item* item = catalog_.findById(static_cast<ItemId>(equipmentIds[i]));
			if (item != 0)
			{
				slots_.replaceEquipment(*item);
			}
		}
	}

	tempEffects_.clear();
	usedOncePassives_.clear();
	return true;
}

// =========================== 伤害修正 ===========================
int Player::calcDamageOutput(int rawDmg)
{
	int dmg = BattleSystem::calcDamageOutput(rawDmg);

	// 低血增伤（狂战神经插件）：HP 低于阈值时按百分比提高输出。
	const PassiveEffect* lowHp = findPassive(PassiveType::LOW_HP_DAMAGE_BONUS);
	if (lowHp != 0 && lowHp->thresholdPercent > 0 &&
		getHp() * 100 <= getMHp() * lowHp->thresholdPercent)
	{
		dmg = dmg * (100 + lowHp->value) / 100;
	}
	return dmg;
}

int Player::calcDamageReceive(int rawDmg)
{
	int dmg = BattleSystem::calcDamageReceive(rawDmg);

	// 固定减伤（复合护盾等）。
	dmg -= getFlatDamageReduction();

	// 百分比减伤（相位偏转盾等）。
	const PassiveEffect* percent = findPassive(PassiveType::PERCENT_DAMAGE_REDUCTION);
	if (percent != 0)
	{
		dmg = dmg * (100 - percent->value) / 100;
	}

	return dmg < 0 ? 0 : dmg;
}
