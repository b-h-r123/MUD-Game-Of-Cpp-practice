#include "ShopConfig.h"

// ========================== 默认平衡参数区 ==========================
//
// 试玩时如果发现物品太贵、稀有装备太少或货架过小，
// 优先只修改 ShopConfig::ShopConfig() 中的这组数字。
// Shop 随机算法、ItemCatalog 和 UI 都不需要跟着改。
// ============================================================================

ShopPoolWeightConfig::ShopPoolWeightConfig(
    int basicSupplyWeight,
    int advancedItemWeight,
    int equipmentWeight,
    int advancedEquipmentWeight)
    : basicSupply(basicSupplyWeight),
      advancedItem(advancedItemWeight),
      equipment(equipmentWeight),
      advancedEquipment(advancedEquipmentWeight)
{
}

int ShopPoolWeightConfig::getWeight(ShopPool pool) const
{
    switch (pool)
    {
    case ShopPool::BASIC_SUPPLY:
        return basicSupply;
    case ShopPool::ADVANCED_ITEM:
        return advancedItem;
    case ShopPool::EQUIPMENT:
        return equipment;
    case ShopPool::ADVANCED_EQUIPMENT:
        return advancedEquipment;
    case ShopPool::NONE:
    default:
        return 0;
    }
}

bool ShopPoolWeightConfig::isValid() const
{
    return basicSupply >= 0 && advancedItem >= 0 &&
           equipment >= 0 && advancedEquipment >= 0 &&
           (basicSupply > 0 || advancedItem > 0 ||
            equipment > 0 || advancedEquipment > 0);
}

ShopModeConfig::ShopModeConfig(
    std::size_t slots,
    const ShopPoolWeightConfig& weights,
    int priceMultiplierPercent)
    : slotCount(slots),
      poolWeights(weights),
      pricePercent(priceMultiplierPercent)
{
}

bool ShopModeConfig::isValid() const
{
    return slotCount > 0U && poolWeights.isValid() && pricePercent > 0;
}

ShopConfig::ShopConfig()
    // 普通商店：4 槽，权重 50 / 35 / 24 / 1，按 ItemCatalog 原价售卖。
    : normalShop(
          4U,
          ShopPoolWeightConfig(50, 35, 24, 1),
          100),
      // 黑市：6 槽，权重 20 / 40 / 10 / 30，售价为基础价的 115%。
      blackMarket(
          6U,
          ShopPoolWeightConfig(20, 40, 10, 30),
          115),
      // 数量限制延续已确认规则：消耗品 10，芯片 3，装备 1。
      consumableInitialStock(10),
      directUpgradeInitialStock(3),
      equipmentInitialStock(1),
      refreshOnEveryEntry(true),
      allowDuplicateOffers(false)
{
}

const ShopModeConfig& ShopConfig::getMode(ShopType type) const
{
    return type == ShopType::BLACK_MARKET ? blackMarket : normalShop;
}

int ShopConfig::getInitialStock(const Item& item) const
{
    if (item.isConsumable())
    {
        return consumableInitialStock;
    }

    if (item.isDirectUpgrade())
    {
        return directUpgradeInitialStock;
    }

    if (item.isEquipment())
    {
        return equipmentInitialStock;
    }

    return 0;
}

bool ShopConfig::isValid() const
{
    // 某个分类的库存可以设为 0，相当于临时从商店池中关闭该分类。
    // 但三个分类不能全部为 0，否则商店永远无货可刷。
    const bool stocksAreNonNegative =
        consumableInitialStock >= 0 &&
        directUpgradeInitialStock >= 0 &&
        equipmentInitialStock >= 0;
    const bool hasAnyStock =
        consumableInitialStock > 0 ||
        directUpgradeInitialStock > 0 ||
        equipmentInitialStock > 0;

    return normalShop.isValid() && blackMarket.isValid() &&
           stocksAreNonNegative && hasAnyStock;
}
