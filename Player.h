#pragma once
#include <string>
#include <vector>
#include "battleSystem.h"
#include "item.h"
#include "Upgrade.h"

// Player 继承 BattleSystem，拥有血量/攻击/能量与 debuff 接口；
// 同时组合 ItemSlots（背包/装备）、Upgrade（等级经验）与 ItemCatalog（道具定义表）。
//
// 设计取舍：
// - 基础属性（b_Hp/b_MHp/b_atk/b_Energy）只保存“裸装”数值；
// - 装备提供的攻击/生命/能量加成通过 getEffectiveXxx() 动态叠加，不写回基础属性；
// - 升级同时做两件事：自动应用强化芯片（Upgrade）+ 发放 1 点属性点（手动分配）。
class Player : public BattleSystem
{
public:
	// 战斗中的临时持续效果（纳米急救针、超导能量核心）。
	struct TempEffect
	{
		EffectType type;   // RESTORE_HP / RESTORE_ENERGY
		int value;         // 每回合恢复量
		int remainingTurns;
	};

	Player();

	// ---------- 档案 ----------
	void setPlayerName(const std::string& newName);
	const std::string& getPlayerName() const;

	int getGold() const;
	int getAtp() const;              // 可用属性点
	void addGold(int val);
	bool spendGold(int val);         // 金币不足返回 false

	// ---------- 等级 / 经验（委托 Upgrade）----------
	int getLevel() const;
	int getExp() const;
	int getRequiredExpForNextLevel() const;
	void addExp(int val);            // 升级：自动芯片 + 属性点

	// ---------- 属性点手动分配 ----------
	bool allocateAttack();           // 消耗 1 点，攻击 +5
	bool allocateMaxHp();            // 消耗 1 点，最大生命 +20

	// ---------- 有效属性（基础 + 装备）----------
	// getAtk/getMHp/getMEnergy 覆写基类：返回叠加装备加成后的有效值。
	int getAtk() const override;
	int getMHp() const override;
	int getMEnergy() const override;
	int getEffectiveAtk() const;
	int getEffectiveMaxHp() const;
	int getEffectiveMaxEnergy() const;
	int getFlatDamageReduction() const;

	// ---------- 背包 / 装备 ----------
	ItemSlots& getItemSlots();
	const ItemSlots& getItemSlots() const;
	ItemCatalog& getCatalog();
	const ItemCatalog& getCatalog() const;
	Upgrade& getUpgrade();

	// 购买成功后交付物品：消耗品入槽、芯片立即生效、装备替换。
	// 返回 false 表示该物品无法交付（调用方不应扣钱）。
	bool receiveItem(const Item& item);

	// 使用一个消耗品；成功返回 true。战斗内外均可调用。
	bool useConsumable(ItemId id);

	// ---------- 战斗被动查询 ----------
	// 返回当前已装备物品携带的指定被动；没有则返回空指针。
	const PassiveEffect* findPassive(PassiveType type) const;
	// 每场战斗限一次的被动：第一次调用返回 true 并记录，之后返回 false。
	bool consumeOncePassive(PassiveType type);
	// 战斗开始时重置“每场一次”记录与临时效果。
	void beginBattle();

	// 战斗中每回合开始调用，结算纳米急救针等持续效果。
	void processTurnStartEffects();

	// 战间整备：把当前 HP / 能量补到有效上限。
	void restoreToFull();

	// 只把能量补满（章节间休整时用；血量不再回满）。
	void restoreEnergyToFull();

	// ---------- 存档 ----------
	// 单行文本序列化；deserialize 失败时不保证状态不变（由 SaveManager 兜底）。
	std::string serialize() const;
	bool deserialize(const std::string& data);

	// 受击修正：先走基类，再叠加装备固定/百分比减伤。
	int calcDamageReceive(int rawDmg) override;
	// 造成伤害修正：叠加低血增伤等被动。
	int calcDamageOutput(int rawDmg) override;

	~Player() = default;

private:
	const Item* findEquippedItem(EquipmentSlot slot) const;
	int sumEquippedBonus(int EquipmentData::*field) const;

	std::string name;
	int Gold = 100;
	int Atp = 0;

	ItemCatalog catalog_;   // 本玩家的道具定义表（唯一数据源）
	ItemSlots slots_;       // 七格物品槽：4 消耗品 + 武器/护盾/配饰
	Upgrade upgrade_;       // 等级与经验
	std::vector<TempEffect> tempEffects_;
	std::vector<PassiveType> usedOncePassives_; // 本场战斗已触发的“每场一次”被动
};
