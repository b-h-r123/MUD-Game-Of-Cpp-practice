#include "item.h"

#include <cassert>
#include <iostream>
#include <vector>

// 这个测试文件不需要 Player、Shop 或 BattleSystem，
// 只验证 Item 模块自己承诺的数据和边界。
int main()
{
    const ItemCatalog catalog;

    // -------------------------- 目录完整性 --------------------------
    assert(catalog.isValid());
    assert(catalog.getAllItems().size() ==
           static_cast<std::size_t>(ItemId::COUNT) - 1U);
    assert(catalog.findById(ItemId::NONE) == 0);
    assert(catalog.findById(ItemId::COUNT) == 0);

    // 四种消耗品、两种芯片和十一件装备构成当前的完整道具池。
    assert(catalog.getItemsByCategory(ItemCategory::CONSUMABLE).size() == 4U);
    assert(catalog.getItemsByCategory(ItemCategory::DIRECT_UPGRADE).size() == 2U);
    assert(catalog.getItemsByCategory(ItemCategory::EQUIPMENT).size() == 11U);

    // 稀有度分布之后会直接供 Shop 计算刷新权重。
    assert(catalog.getItemsByTier(ItemTier::BASIC).size() == 7U);
    assert(catalog.getItemsByTier(ItemTier::ADVANCED).size() == 7U);
    assert(catalog.getItemsByTier(ItemTier::UNIQUE).size() == 3U);

    // 装备池覆盖四件武器、四件护盾和三件配饰。
    assert(catalog.getEquipmentBySlot(EquipmentSlot::WEAPON).size() == 4U);
    assert(catalog.getEquipmentBySlot(EquipmentSlot::SHIELD).size() == 4U);
    assert(catalog.getEquipmentBySlot(EquipmentSlot::ACCESSORY).size() == 3U);
    assert(catalog.getEquipmentBySlot(EquipmentSlot::NONE).empty());

    // -------------------------- 关键物品规则 --------------------------
    const Item* medkit = catalog.findById(ItemId::MEDKIT);
    assert(medkit != 0);
    assert(medkit->isConsumable());
    assert(medkit->canUseInBattle());
    assert(medkit->canUseOutsideBattle());
    assert(medkit->getEffects().size() == 1U);
    assert(medkit->getEffects()[0].type == EffectType::RESTORE_HP);
    assert(medkit->getEffects()[0].value == 35);
    assert(!medkit->getEffects()[0].isTemporary());

    const Item* energyCell = catalog.findById(ItemId::ENERGY_CELL);
    assert(energyCell != 0);
    assert(energyCell->canUseInBattle());
    assert(!energyCell->canUseOutsideBattle());

    const Item* nanoMedkit = catalog.findById(ItemId::NANO_MEDKIT);
    assert(nanoMedkit != 0);
    assert(nanoMedkit->getEffects()[0].durationTurns == 3);
    assert(nanoMedkit->getEffects()[0].isTemporary());

    const Item* hpChip = catalog.findById(ItemId::HP_CHIP);
    assert(hpChip != 0);
    assert(hpChip->isDirectUpgrade());
    assert(!hpChip->canUseInBattle());
    assert(!hpChip->canUseOutsideBattle());
    assert(hpChip->getEffects()[0].type == EffectType::INCREASE_MAX_HP);

    // -------------------------- 特殊被动覆盖 --------------------------
    // 每一种已定义的装备被动都至少有一件代表装备，
    // 避免出现枚举已写但道具池中永远用不到的假功能。
    assert(catalog.findById(ItemId::ARC_CHAIN_BLADE)
               ->getEquipmentData().passive.type == PassiveType::FOLLOW_UP_DAMAGE);
    assert(catalog.findById(ItemId::EXECUTION_RAIL_BLADE)
               ->getEquipmentData().passive.type == PassiveType::EXECUTE_DAMAGE);
    assert(catalog.findById(ItemId::OVERCLOCK_PULSE_RIFLE)
               ->getEquipmentData().passive.type == PassiveType::SKILL_DAMAGE_BONUS);
    assert(catalog.findById(ItemId::PHASE_DEFLECTOR)
               ->getEquipmentData().passive.type == PassiveType::PERCENT_DAMAGE_REDUCTION);
    assert(catalog.findById(ItemId::MIRROR_SHIELD)
               ->getEquipmentData().passive.type == PassiveType::REFLECT_DAMAGE);
    assert(catalog.findById(ItemId::AEGIS_ARMOR)
               ->getEquipmentData().passive.type == PassiveType::LETHAL_GUARD);
    assert(catalog.findById(ItemId::BERSERKER_IMPLANT)
               ->getEquipmentData().passive.type == PassiveType::LOW_HP_DAMAGE_BONUS);
    assert(catalog.findById(ItemId::ZERO_POINT_CAPACITOR)
               ->getEquipmentData().passive.type == PassiveType::FIRST_SKILL_COST_REDUCTION);

    // -------------------------- 物品槽集成 --------------------------
    ItemSlots slots;
    assert(slots.isValid());
    assert(slots.findConsumableSlot(ItemId::MEDKIT)->getCount() == 1);
    assert(slots.findConsumableSlot(ItemId::ENERGY_CELL)->getCount() == 1);
    assert(slots.findConsumableSlot(ItemId::NANO_MEDKIT)->getCount() == 0);
    assert(slots.findConsumableSlot(ItemId::SUPERCONDUCTING_CORE)->getCount() == 0);

    assert(slots.replaceEquipment(*catalog.findById(ItemId::PULSE_BLADE)));
    assert(slots.getEquippedItem(EquipmentSlot::WEAPON) == ItemId::PULSE_BLADE);
    assert(slots.replaceEquipment(*catalog.findById(ItemId::ARC_CHAIN_BLADE)));
    assert(slots.getEquippedItem(EquipmentSlot::WEAPON) == ItemId::ARC_CHAIN_BLADE);
    assert(!slots.replaceEquipment(*catalog.findById(ItemId::ARC_CHAIN_BLADE)));

    // 芯片不能进入消耗品槽，消耗品也不能占用装备槽。
    assert(!slots.addConsumable(ItemId::ATTACK_CHIP, 1));
    assert(!slots.replaceEquipment(*medkit));

    // -------------------------- 非法配置 --------------------------
    const Item invalidEquipmentTiming(
        ItemId::PULSE_BLADE,
        "错误武器",
        "装备不应声明为可主动使用。",
        ItemCategory::EQUIPMENT,
        ItemTier::BASIC,
        1,
        ItemUseTiming::ANYTIME,
        std::vector<ItemEffect>(),
        EquipmentData(EquipmentSlot::WEAPON, 1));
    assert(!invalidEquipmentTiming.isValid());

    const Item invalidConsumableTiming(
        ItemId::MEDKIT,
        "错误消耗品",
        "消耗品必须声明使用时机。",
        ItemCategory::CONSUMABLE,
        ItemTier::BASIC,
        1,
        ItemUseTiming::NOT_DIRECTLY_USABLE,
        std::vector<ItemEffect>(1, ItemEffect(EffectType::RESTORE_HP, 1)));
    assert(!invalidConsumableTiming.isValid());

    std::cout << "Item catalog tests passed.\n";
    return 0;
}
