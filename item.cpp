#include "item.h"

#include <algorithm>
#include <limits>

// ============================= ItemEffect =============================
// 使用初始化列表把传入参数写入三个公开数据字段。
// 这里只保存配置，不在构造函数中直接执行回血或属性修改。
ItemEffect::ItemEffect(EffectType effectType, int effectValue, int duration)
    : type(effectType), value(effectValue), durationTurns(duration)
{
}

bool ItemEffect::isValid() const
{
    // value 为 0 或负数意味着效果没有意义；durationTurns 为负数则无法解释。
    return value > 0 && durationTurns >= 0;
}

bool ItemEffect::isTemporary() const
{
    // durationTurns == 0 由调用方解释为立即效果或永久升级。
    return durationTurns > 0;
}

// ============================ PassiveEffect ============================
PassiveEffect::PassiveEffect(
    PassiveType passiveType,
    int effectValue,
    int threshold,
    bool once)
    : type(passiveType),
      value(effectValue),
      thresholdPercent(threshold),
      oncePerBattle(once)
{
}

bool PassiveEffect::isValid() const
{
    // NONE 必须是完全空的默认配置。如果 NONE 仍带有数值，说明创建物品时忘记选择真正的 PassiveType，因此直接判为非法。
    if (type == PassiveType::NONE)
    {
        return value == 0 && thresholdPercent == 0 && !oncePerBattle;
    }

    // 真实被动必须有正数强度；百分比阈值只能位于 0～100。
    return value > 0 && thresholdPercent >= 0 && thresholdPercent <= 100;
}

bool PassiveEffect::isActive() const
{
    return type != PassiveType::NONE;
}

// ============================ EquipmentData ============================
// 普通属性加成和特殊被动集中在一个小型数据结构中。
// BattleSystem 读取这些数据后再决定伤害、减伤或反弹的具体结算顺序。
EquipmentData::EquipmentData(
    EquipmentSlot equipmentSlot,
    int attack,
    int maxHp,
    int maxEnergy,
    int damageReduction,
    const PassiveEffect& passiveEffect)
    : slot(equipmentSlot),
      attackBonus(attack),
      maxHpBonus(maxHp),
      maxEnergyBonus(maxEnergy),
      flatDamageReduction(damageReduction),
      passive(passiveEffect)
{
}

bool EquipmentData::isValid() const
{
    // 当前设计没有“负属性装备”。如果以后确实需要诅咒装备，应修改这里的规则，
    // 而不是绕过校验直接写入负数。
    return attackBonus >= 0 && maxHpBonus >= 0 && maxEnergyBonus >= 0 &&
           flatDamageReduction >= 0 && passive.isValid();
}

bool EquipmentData::hasAnyBonus() const
{
    // 只要普通加成或特殊被动中有一个生效，这件装备就不是空装备。
    return attackBonus > 0 || maxHpBonus > 0 || maxEnergyBonus > 0 ||
           flatDamageReduction > 0 || passive.isActive();
}

bool EquipmentData::isEmpty() const
{
    // 非装备物品必须使用这种“无槽位、无加成”的默认数据。
    return slot == EquipmentSlot::NONE && !hasAnyBonus();
}

// ================================= Item ================================
// Item 构造函数只复制配置。使用下划线结尾的名称（如 id_）区分成员变量和参数。
// effects 是 vector，因此一个物品可以组合多条简单效果，例如生命芯片可以同时增加 maxHp 和当前 HP；具体组合将在物品目录配置阶段创建。
Item::Item(
    ItemId id,
    const std::string& name,
    const std::string& description,
    ItemCategory category,
    ItemTier tier,
    int price,
    ItemUseTiming useTiming,
    const std::vector<ItemEffect>& effects,
    const EquipmentData& equipment)
    : id_(id),
      name_(name),
      description_(description),
      category_(category),
      tier_(tier),
      price_(price),
      useTiming_(useTiming),
      effects_(effects),
      equipment_(equipment)
{
}

ItemId Item::getId() const
{
    return id_;
}

const std::string& Item::getName() const
{
    return name_;
}

const std::string& Item::getDescription() const
{
    return description_;
}

ItemCategory Item::getCategory() const
{
    return category_;
}

ItemTier Item::getTier() const
{
    return tier_;
}

int Item::getPrice() const
{
    return price_;
}

ItemUseTiming Item::getUseTiming() const
{
    return useTiming_;
}

const std::vector<ItemEffect>& Item::getEffects() const
{
    //返回 const 引用：调用方可以遍历效果，但不能随意增删配置。
    return effects_;
}

const EquipmentData& Item::getEquipmentData() const
{
    return equipment_;
}

bool Item::isConsumable() const
{
    // 这些分类查询函数隐藏了枚举比较细节，让 Shop / Player 调用时更易读。
    return category_ == ItemCategory::CONSUMABLE;
}

bool Item::isDirectUpgrade() const
{
    return category_ == ItemCategory::DIRECT_UPGRADE;
}

bool Item::isEquipment() const
{
    return category_ == ItemCategory::EQUIPMENT;
}

bool Item::canUseInBattle() const
{
    // 只有消耗品才能进入“使用道具”流程。ANYTIME 和 BATTLE_ONLY
    // 都允许在战斗中使用，芯片和装备则一律返回 false。
    return isConsumable() &&
           (useTiming_ == ItemUseTiming::ANYTIME ||
            useTiming_ == ItemUseTiming::BATTLE_ONLY);
}

bool Item::canUseOutsideBattle() const
{
    // 当前只有医疗包是 ANYTIME。能量离开战斗后会自动回满，
    // 因此能量电池和持续恢复道具没有战斗外使用价值。
    return isConsumable() && useTiming_ == ItemUseTiming::ANYTIME;
}

bool Item::isValid() const
{
    // 第一层：检查所有类别都共有的基础字段。
    // - NONE 不是实际物品；
    // - 空名称无法显示；
    // - 价格不能为负数；
    // - 装备数据中的普通数值和被动必须先各自合法。
    const int numericId = static_cast<int>(id_);
    if (numericId <= static_cast<int>(ItemId::NONE) ||
        numericId >= static_cast<int>(ItemId::COUNT) ||
        name_.empty() || price_ < 0 || !equipment_.isValid())
    {
        return false;
    }

    // 第二层：逐条检查vector中的主动效果。
    // 使用迭代器,STL容器的遍历方式。
    for (std::vector<ItemEffect>::const_iterator it = effects_.begin();
         it != effects_.end();
         ++it)
    {
        if (!it->isValid())
        {
            return false;
        }
    }

    if (isEquipment())
    {
        // 装备必须：
        // 1. 指定武器/护盾/配饰槽；
        // 2. 至少有一项普通加成或特殊被动；
        // 3. 不携带消耗品/芯片使用的 effects 数据。
        return useTiming_ == ItemUseTiming::NOT_DIRECTLY_USABLE &&
               equipment_.slot != EquipmentSlot::NONE &&
               equipment_.hasAnyBonus() && effects_.empty();
    }

    // 消耗品和芯片必须正好相反：不占装备槽、不带装备加成，并至少有一条效果。
    // 这一判断可以阻止“消耗品暗藏武器攻击加成”等类别混用错误。
    if (!equipment_.isEmpty() || effects_.empty())
    {
        return false;
    }

    // 消耗品必须声明可使用时机；芯片由购买流程直接生效，
    // 不允许它出现在战斗道具菜单中。
    if (isConsumable())
    {
        return useTiming_ != ItemUseTiming::NOT_DIRECTLY_USABLE;
    }

    return isDirectUpgrade() &&
           useTiming_ == ItemUseTiming::NOT_DIRECTLY_USABLE;
}

// ============================== ItemCatalog ==============================
// 下面的数字是“集中的首轮平衡参数”，而不是散落在战斗或商店中的逻辑。
// 未来试玩后如果需要调整，只修改本构造函数中对应 Item 的配置即可。
ItemCatalog::ItemCatalog()
{
    // 目录共有 17 件真实物品，预留空间可避免 push_back 过程中多次扩容。
    items_.reserve(static_cast<std::size_t>(ItemId::COUNT) - 1U);

    // ------------------------------ 消耗品 ------------------------------
    // 医疗包可在战斗内外使用，立即恢复 35 HP。
    items_.push_back(Item(
        ItemId::MEDKIT,
        "医疗包",
        "立即恢复 35 点 HP，可在战斗内外使用。",
        ItemCategory::CONSUMABLE,
        ItemTier::BASIC,
        30,
        ItemUseTiming::ANYTIME,
        std::vector<ItemEffect>(1, ItemEffect(EffectType::RESTORE_HP, 35))));

    // 能量电池只在战斗内有意义，因为离开战斗后 Energy 会自动回满。
    items_.push_back(Item(
        ItemId::ENERGY_CELL,
        "能量电池",
        "战斗中立即恢复 25 点 Energy。",
        ItemCategory::CONSUMABLE,
        ItemTier::BASIC,
        25,
        ItemUseTiming::BATTLE_ONLY,
        std::vector<ItemEffect>(1, ItemEffect(EffectType::RESTORE_ENERGY, 25))));

    // durationTurns = 3 表示使用的当前回合就算第 1 回合，之后再生效 2 回合。
    // 若战斗提前结束，剩余持续效果由 BattleSystem 全部清除。
    items_.push_back(Item(
        ItemId::NANO_MEDKIT,
        "纳米急救针",
        "战斗中使用，从当前回合开始，连续 3 回合每回合恢复 12 点 HP。",
        ItemCategory::CONSUMABLE,
        ItemTier::ADVANCED,
        90,
        ItemUseTiming::BATTLE_ONLY,
        std::vector<ItemEffect>(1, ItemEffect(EffectType::RESTORE_HP, 12, 3))));

    items_.push_back(Item(
        ItemId::SUPERCONDUCTING_CORE,
        "超导能量核心",
        "战斗中使用，从当前回合开始，连续 3 回合每回合恢复 10 点 Energy。",
        ItemCategory::CONSUMABLE,
        ItemTier::ADVANCED,
        100,
        ItemUseTiming::BATTLE_ONLY,
        std::vector<ItemEffect>(1, ItemEffect(EffectType::RESTORE_ENERGY, 10, 3))));

    // ------------------------------ 属性芯片 ------------------------------
    // 芯片不进入物品槽，Shop 购买成功后应当立即把 effects 交给 Player。
    items_.push_back(Item(
        ItemId::ATTACK_CHIP,
        "强化芯片",
        "购买后立即永久增加 3 点基础 ATK，不进入物品槽。",
        ItemCategory::DIRECT_UPGRADE,
        ItemTier::BASIC,
        150,
        ItemUseTiming::NOT_DIRECTLY_USABLE,
        std::vector<ItemEffect>(1, ItemEffect(EffectType::INCREASE_ATTACK, 3))));

    items_.push_back(Item(
        ItemId::HP_CHIP,
        "生命芯片",
        "购买后立即永久增加 20 点基础 maxHp，并同步增加当前 HP。",
        ItemCategory::DIRECT_UPGRADE,
        ItemTier::BASIC,
        150,
        ItemUseTiming::NOT_DIRECTLY_USABLE,
        std::vector<ItemEffect>(1, ItemEffect(EffectType::INCREASE_MAX_HP, 20))));

    // ------------------------------- 武器 -------------------------------
    items_.push_back(Item(
        ItemId::PULSE_BLADE,
        "脉冲短刃",
        "装备时提供 5 点 ATK。",
        ItemCategory::EQUIPMENT,
        ItemTier::BASIC,
        120,
        ItemUseTiming::NOT_DIRECTLY_USABLE,
        std::vector<ItemEffect>(),
        EquipmentData(EquipmentSlot::WEAPON, 5)));

    items_.push_back(Item(
        ItemId::ARC_CHAIN_BLADE,
        "电弧链刃",
        "装备时提供 6 点 ATK；攻击命中后追加 4 点伤害，追加伤害不再触发本被动。",
        ItemCategory::EQUIPMENT,
        ItemTier::ADVANCED,
        240,
        ItemUseTiming::NOT_DIRECTLY_USABLE,
        std::vector<ItemEffect>(),
        EquipmentData(
            EquipmentSlot::WEAPON,
            6,
            0,
            0,
            0,
            PassiveEffect(PassiveType::FOLLOW_UP_DAMAGE, 4))));

    items_.push_back(Item(
        ItemId::EXECUTION_RAIL_BLADE,
        "处决者磁轨刃",
        "装备时提供 9 点 ATK；敌人 HP 低于或等于 25% 时，造成的伤害提高 30%。",
        ItemCategory::EQUIPMENT,
        ItemTier::UNIQUE,
        420,
        ItemUseTiming::NOT_DIRECTLY_USABLE,
        std::vector<ItemEffect>(),
        EquipmentData(
            EquipmentSlot::WEAPON,
            9,
            0,
            0,
            0,
            PassiveEffect(PassiveType::EXECUTE_DAMAGE, 30, 25))));

    items_.push_back(Item(
        ItemId::OVERCLOCK_PULSE_RIFLE,
        "超频脉冲枪",
        "装备时提供 6 点 ATK；使用消耗 Energy 的技能时，伤害提高 20%。",
        ItemCategory::EQUIPMENT,
        ItemTier::ADVANCED,
        260,
        ItemUseTiming::NOT_DIRECTLY_USABLE,
        std::vector<ItemEffect>(),
        EquipmentData(
            EquipmentSlot::WEAPON,
            6,
            0,
            0,
            0,
            PassiveEffect(PassiveType::SKILL_DAMAGE_BONUS, 20))));

    // ------------------------------- 护盾 -------------------------------
    items_.push_back(Item(
        ItemId::COMPOSITE_SHIELD,
        "复合护盾",
        "装备后，每次受伤时减少 3 点伤害，最终伤害不低于 0。",
        ItemCategory::EQUIPMENT,
        ItemTier::BASIC,
        120,
        ItemUseTiming::NOT_DIRECTLY_USABLE,
        std::vector<ItemEffect>(),
        EquipmentData(EquipmentSlot::SHIELD, 0, 0, 0, 3)));

    items_.push_back(Item(
        ItemId::PHASE_DEFLECTOR,
        "相位偏转盾",
        "装备后，按比例减少 15% 的受到伤害。",
        ItemCategory::EQUIPMENT,
        ItemTier::ADVANCED,
        250,
        ItemUseTiming::NOT_DIRECTLY_USABLE,
        std::vector<ItemEffect>(),
        EquipmentData(
            EquipmentSlot::SHIELD,
            0,
            0,
            0,
            0,
            PassiveEffect(PassiveType::PERCENT_DAMAGE_REDUCTION, 15))));

    items_.push_back(Item(
        ItemId::MIRROR_SHIELD,
        "镜面反射盾",
        "装备后提供 2 点固定减伤，并反弹玩家最终承受伤害的 20%。",
        ItemCategory::EQUIPMENT,
        ItemTier::UNIQUE,
        400,
        ItemUseTiming::NOT_DIRECTLY_USABLE,
        std::vector<ItemEffect>(),
        EquipmentData(
            EquipmentSlot::SHIELD,
            0,
            0,
            0,
            2,
            PassiveEffect(PassiveType::REFLECT_DAMAGE, 20))));

    items_.push_back(Item(
        ItemId::AEGIS_ARMOR,
        "神盾装甲",
        "装备时提供 25 点 maxHp；每场战斗首次受到致命伤害时保留 1 点 HP。",
        ItemCategory::EQUIPMENT,
        ItemTier::UNIQUE,
        450,
        ItemUseTiming::NOT_DIRECTLY_USABLE,
        std::vector<ItemEffect>(),
        EquipmentData(
            EquipmentSlot::SHIELD,
            0,
            25,
            0,
            0,
            PassiveEffect(PassiveType::LETHAL_GUARD, 1, 0, true))));

    // ------------------------------- 配饰 -------------------------------
    items_.push_back(Item(
        ItemId::NEURAL_AMPLIFIER,
        "神经增幅器",
        "装备时提供 15 点 maxEnergy。",
        ItemCategory::EQUIPMENT,
        ItemTier::BASIC,
        130,
        ItemUseTiming::NOT_DIRECTLY_USABLE,
        std::vector<ItemEffect>(),
        EquipmentData(EquipmentSlot::ACCESSORY, 0, 0, 15)));

    items_.push_back(Item(
        ItemId::BERSERKER_IMPLANT,
        "狂战神经插件",
        "装备时提供 3 点 ATK；玩家 HP 低于或等于 30% 时，造成的伤害提高 25%。",
        ItemCategory::EQUIPMENT,
        ItemTier::ADVANCED,
        240,
        ItemUseTiming::NOT_DIRECTLY_USABLE,
        std::vector<ItemEffect>(),
        EquipmentData(
            EquipmentSlot::ACCESSORY,
            3,
            0,
            0,
            0,
            PassiveEffect(PassiveType::LOW_HP_DAMAGE_BONUS, 25, 30))));

    items_.push_back(Item(
        ItemId::ZERO_POINT_CAPACITOR,
        "零点电容",
        "装备时提供 20 点 maxEnergy；每场战斗第一次使用技能时少消耗 10 点 Energy。",
        ItemCategory::EQUIPMENT,
        ItemTier::ADVANCED,
        260,
        ItemUseTiming::NOT_DIRECTLY_USABLE,
        std::vector<ItemEffect>(),
        EquipmentData(
            EquipmentSlot::ACCESSORY,
            0,
            0,
            20,
            0,
            PassiveEffect(PassiveType::FIRST_SKILL_COST_REDUCTION, 10, 0, true))));
}

const std::vector<Item>& ItemCatalog::getAllItems() const
{
    return items_;
}

const Item* ItemCatalog::findById(ItemId id) const
{
    // find_if 直接表达“找到第一个 ID 相等的 Item”，比手写下标循环更贴合意图。
    const std::vector<Item>::const_iterator found = std::find_if(
        items_.begin(),
        items_.end(),
        [id](const Item& item) { return item.getId() == id; });

    return found == items_.end() ? 0 : &(*found);
}

std::vector<const Item*> ItemCatalog::getItemsByCategory(ItemCategory category) const
{
    std::vector<const Item*> result;

    for (std::vector<Item>::const_iterator it = items_.begin(); it != items_.end(); ++it)
    {
        if (it->getCategory() == category)
        {
            result.push_back(&(*it));
        }
    }

    return result;
}

std::vector<const Item*> ItemCatalog::getItemsByTier(ItemTier tier) const
{
    std::vector<const Item*> result;

    for (std::vector<Item>::const_iterator it = items_.begin(); it != items_.end(); ++it)
    {
        if (it->getTier() == tier)
        {
            result.push_back(&(*it));
        }
    }

    return result;
}

std::vector<const Item*> ItemCatalog::getEquipmentBySlot(EquipmentSlot slot) const
{
    std::vector<const Item*> result;

    // NONE 不是真实装备槽，直接返回空结果可避免调用方误用。
    if (slot == EquipmentSlot::NONE)
    {
        return result;
    }

    for (std::vector<Item>::const_iterator it = items_.begin(); it != items_.end(); ++it)
    {
        if (it->isEquipment() && it->getEquipmentData().slot == slot)
        {
            result.push_back(&(*it));
        }
    }

    return result;
}

bool ItemCatalog::isValid() const
{
    const int itemIdCount = static_cast<int>(ItemId::COUNT);

    // NONE 不是真实物品，因此完整目录应当正好有 COUNT - 1 个元素。
    if (items_.size() != static_cast<std::size_t>(itemIdCount - 1))
    {
        return false;
    }

    // seen[index] 记录对应 ItemId 是否已出现，同时可以发现重复和遗漏。
    std::vector<bool> seen(static_cast<std::size_t>(itemIdCount), false);

    for (std::vector<Item>::const_iterator it = items_.begin(); it != items_.end(); ++it)
    {
        if (!it->isValid())
        {
            return false;
        }

        const int numericId = static_cast<int>(it->getId());
        if (numericId <= static_cast<int>(ItemId::NONE) ||
            numericId >= itemIdCount ||
            seen[static_cast<std::size_t>(numericId)])
        {
            return false;
        }

        seen[static_cast<std::size_t>(numericId)] = true;
    }

    for (int numericId = 1; numericId < itemIdCount; ++numericId)
    {
        if (!seen[static_cast<std::size_t>(numericId)])
        {
            return false;
        }
    }

    return true;
}

// ============================ ConsumableSlot ============================
// ConsumableSlot 只保存一个固定消耗品槽的 ItemId 和数量。
// count_ 是 private，外部不能绕过边界检查直接写成负数。
ConsumableSlot::ConsumableSlot(ItemId id, int initialCount)
    : itemId_(id), count_(initialCount)
{
}

ItemId ConsumableSlot::getItemId() const
{
    return itemId_;
}

int ConsumableSlot::getCount() const
{
    return count_;
}

bool ConsumableSlot::add(int amount)
{
    // 三种失败情况都必须保持 count_ 不变：
    // 1. 增加量不是正数；
    // 2. 当前对象本身已经带有非法负数；
    // 3. count_ + amount 会超过 int 最大值并发生溢出。
    // 第 3 项改写成减法比较，避免在检查过程中先执行可能溢出的加法。
    if (amount <= 0 || count_ < 0 ||
        count_ > std::numeric_limits<int>::max() - amount)
    {
        return false;
    }

    count_ += amount;
    return true;
}

bool ConsumableSlot::consumeOne()
{
    // 数量为 0 或非法负数时都不能继续消耗，确保 count_ 永远不会因本函数变负。
    if (count_ <= 0)
    {
        return false;
    }

    --count_;
    return true;
}

bool ConsumableSlot::isEmpty() const
{
    return count_ == 0;
}

bool ConsumableSlot::isValid() const
{
    // 默认构造的 NONE/0 只适合作为空占位；正式消耗品槽必须换成真实 ItemId。
    return itemId_ != ItemId::NONE && count_ >= 0;
}

// =============================== ItemSlots ==============================
ItemSlots::ItemSlots()
    : consumableSlots_{{
          ConsumableSlot(ItemId::MEDKIT, 1),
          ConsumableSlot(ItemId::ENERGY_CELL, 1),
          ConsumableSlot(ItemId::NANO_MEDKIT, 0),
          ConsumableSlot(ItemId::SUPERCONDUCTING_CORE, 0)}},
      weapon_(ItemId::NONE),
      shield_(ItemId::NONE),
      accessory_(ItemId::NONE)
{
}

const std::array<ConsumableSlot, 4>& ItemSlots::getConsumableSlots() const
{
    return consumableSlots_;
}

const ConsumableSlot* ItemSlots::findConsumableSlot(ItemId id) const
{
    for (std::array<ConsumableSlot, 4>::const_iterator it = consumableSlots_.begin();
         it != consumableSlots_.end();
         ++it)
    {
        if (it->getItemId() == id)
        {
            return &(*it);
        }
    }

    return 0;
}

ConsumableSlot* ItemSlots::findMutableConsumableSlot(ItemId id)
{
    for (std::array<ConsumableSlot, 4>::iterator it = consumableSlots_.begin();
         it != consumableSlots_.end();
         ++it)
    {
        if (it->getItemId() == id)
        {
            return &(*it);
        }
    }

    return 0;
}

bool ItemSlots::addConsumable(ItemId id, int amount)
{
    ConsumableSlot* slot = findMutableConsumableSlot(id);
    return slot != 0 && slot->add(amount);
}

bool ItemSlots::consumeConsumable(ItemId id)
{
    ConsumableSlot* slot = findMutableConsumableSlot(id);
    return slot != 0 && slot->consumeOne();
}

ItemId ItemSlots::getEquippedItem(EquipmentSlot slot) const
{
    switch (slot)
    {
    case EquipmentSlot::WEAPON:
        return weapon_;
    case EquipmentSlot::SHIELD:
        return shield_;
    case EquipmentSlot::ACCESSORY:
        return accessory_;
    case EquipmentSlot::NONE:
    default:
        return ItemId::NONE;
    }
}

bool ItemSlots::replaceEquipment(const Item& equipment)
{
    if (!equipment.isValid() || !equipment.isEquipment())
    {
        return false;
    }

    ItemId* targetSlot = 0;
    switch (equipment.getEquipmentData().slot)
    {
    case EquipmentSlot::WEAPON:
        targetSlot = &weapon_;
        break;
    case EquipmentSlot::SHIELD:
        targetSlot = &shield_;
        break;
    case EquipmentSlot::ACCESSORY:
        targetSlot = &accessory_;
        break;
    case EquipmentSlot::NONE:
    default:
        return false;
    }

    if (*targetSlot == equipment.getId())
    {
        return false;
    }

    // 直接覆盖且不返回旧 ItemId，体现“替换后旧装备消失、不可撤销”的规则。
    *targetSlot = equipment.getId();
    return true;
}

bool ItemSlots::isValid() const
{
    static const ItemId expectedIds[4] = {
        ItemId::MEDKIT,
        ItemId::ENERGY_CELL,
        ItemId::NANO_MEDKIT,
        ItemId::SUPERCONDUCTING_CORE};

    for (std::size_t index = 0; index < consumableSlots_.size(); ++index)
    {
        if (!consumableSlots_[index].isValid() ||
            consumableSlots_[index].getItemId() != expectedIds[index])
        {
            return false;
        }
    }

    return true;
}
