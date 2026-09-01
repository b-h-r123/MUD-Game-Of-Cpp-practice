#include "Shop.h"

#include <cassert>
#include <iostream>
#include <limits>

namespace
{
// 测试辅助函数：返回某件商品当前所在槽位，未找到时返回 size_t 最大值。
std::size_t findOfferIndex(const Shop& shop, ItemId id)
{
    const std::vector<ShopOffer>& offers = shop.getOffers();
    for (std::size_t index = 0; index < offers.size(); ++index)
    {
        if (offers[index].itemId == id)
        {
            return index;
        }
    }

    return std::numeric_limits<std::size_t>::max();
}

bool hasDuplicateOffers(const Shop& shop)
{
    const std::vector<ShopOffer>& offers = shop.getOffers();
    for (std::size_t left = 0; left < offers.size(); ++left)
    {
        for (std::size_t right = left + 1U; right < offers.size(); ++right)
        {
            if (offers[left].itemId == offers[right].itemId)
            {
                return true;
            }
        }
    }

    return false;
}
} // namespace

int main()
{
    const ItemCatalog catalog;

    // -------------------------- 默认配置 --------------------------
    ShopConfig config;
    assert(config.isValid());
    assert(config.normalShop.slotCount == 4U);
    assert(config.blackMarket.slotCount == 6U);
    assert(config.normalShop.poolWeights.basicSupply == 50);
    assert(config.normalShop.poolWeights.advancedItem == 35);
    assert(config.normalShop.poolWeights.equipment == 24);
    assert(config.normalShop.poolWeights.advancedEquipment == 1);
    assert(config.blackMarket.poolWeights.basicSupply == 20);
    assert(config.blackMarket.poolWeights.advancedItem == 40);
    assert(config.blackMarket.poolWeights.equipment == 10);
    assert(config.blackMarket.poolWeights.advancedEquipment == 30);
    assert(config.consumableInitialStock == 10);
    assert(config.directUpgradeInitialStock == 3);
    assert(config.equipmentInitialStock == 1);

    // 所有可调数字都会先经过边界校验。
    ShopConfig invalidConfig = config;
    invalidConfig.normalShop.slotCount = 0U;
    assert(!invalidConfig.isValid());
    invalidConfig = config;
    invalidConfig.normalShop.poolWeights.advancedEquipment = -1;
    assert(!invalidConfig.isValid());
    invalidConfig = config;
    invalidConfig.consumableInitialStock = 0;
    invalidConfig.directUpgradeInitialStock = 0;
    invalidConfig.equipmentInitialStock = 0;
    assert(!invalidConfig.isValid());

    // -------------------------- 普通商店 --------------------------
    Shop normalShop(ShopType::NORMAL, catalog, config, 20260901U);
    assert(normalShop.isValid());
    assert(normalShop.getRemainingStock(ItemId::MEDKIT) == 10);
    assert(normalShop.getRemainingStock(ItemId::ATTACK_CHIP) == 3);
    assert(normalShop.getRemainingStock(ItemId::PULSE_BLADE) == 1);
    assert(normalShop.enter());
    assert(normalShop.getOffers().size() == 4U);
    assert(!hasDuplicateOffers(normalShop));
    assert(normalShop.isValid());

    // 普通商店价格保持 Catalog 原价。
    for (std::vector<ShopOffer>::const_iterator it = normalShop.getOffers().begin();
         it != normalShop.getOffers().end();
         ++it)
    {
        const Item* item = catalog.findById(it->itemId);
        assert(item != 0);
        assert(it->price == item->getPrice());
    }

    // 同一个 seed 和配置应产生相同首次货架，便于复现随机 bug。
    Shop reproducedShop(ShopType::NORMAL, catalog, config, 20260901U);
    assert(reproducedShop.enter());
    assert(reproducedShop.getOffers().size() == normalShop.getOffers().size());
    for (std::size_t index = 0; index < normalShop.getOffers().size(); ++index)
    {
        assert(reproducedShop.getOffers()[index].itemId == normalShop.getOffers()[index].itemId);
        assert(reproducedShop.getOffers()[index].price == normalShop.getOffers()[index].price);
    }

    // ----------------------------- 黑市 -----------------------------
    Shop blackMarket(ShopType::BLACK_MARKET, catalog, config, 42U);
    assert(blackMarket.enter());
    assert(blackMarket.getOffers().size() == 6U);
    assert(!hasDuplicateOffers(blackMarket));
    assert(blackMarket.isValid());

    // 黑市价格为基础价的 115%，不整除时向上取整。
    for (std::vector<ShopOffer>::const_iterator it = blackMarket.getOffers().begin();
         it != blackMarket.getOffers().end();
         ++it)
    {
        const Item* item = catalog.findById(it->itemId);
        const int expectedPrice = (item->getPrice() * 115 + 99) / 100;
        assert(it->price == expectedPrice);
    }

    // 四个商品池权重可以像 config 超参数一样独立调整。
    // 只保留普通装备池后，货架应当只刷新 BASIC 装备。
    ShopConfig equipmentOnlyConfig = config;
    equipmentOnlyConfig.normalShop.poolWeights = ShopPoolWeightConfig(0, 0, 100, 0);
    Shop equipmentOnlyShop(ShopType::NORMAL, catalog, equipmentOnlyConfig, 99U);
    assert(equipmentOnlyShop.enter());
    assert(equipmentOnlyShop.getOffers().size() == 3U);
    for (std::vector<ShopOffer>::const_iterator it = equipmentOnlyShop.getOffers().begin();
         it != equipmentOnlyShop.getOffers().end();
         ++it)
    {
        const Item* item = catalog.findById(it->itemId);
        assert(item->isEquipment());
        assert(item->getTier() == ItemTier::BASIC);
    }

    // -------------------------- 库存与购买确认 --------------------------
    // 把普通商店设为 7 槽，且只开启常规补给和普通装备池，
    // 可以一次看到 4 件常规补给 + 3 件 BASIC 装备。
    ShopConfig stockTestConfig = config;
    stockTestConfig.normalShop.slotCount = 7U;
    stockTestConfig.normalShop.poolWeights = ShopPoolWeightConfig(100, 0, 100, 0);
    stockTestConfig.refreshOnEveryEntry = false;

    Shop stockTestShop(ShopType::NORMAL, catalog, stockTestConfig, 7U);
    assert(stockTestShop.enter());
    assert(stockTestShop.getOffers().size() == 7U);

    // 消耗品购买后库存从 10 变成 9，因为尚有库存，它仍保留在货架上。
    std::size_t medkitIndex = findOfferIndex(stockTestShop, ItemId::MEDKIT);
    assert(medkitIndex != std::numeric_limits<std::size_t>::max());
    assert(stockTestShop.commitPurchase(medkitIndex));
    assert(stockTestShop.getRemainingStock(ItemId::MEDKIT) == 9);
    assert(findOfferIndex(stockTestShop, ItemId::MEDKIT) !=
           std::numeric_limits<std::size_t>::max());

    // 装备默认只有 1 件，购买后库存归零并从当前货架移除。
    std::size_t weaponIndex = findOfferIndex(stockTestShop, ItemId::PULSE_BLADE);
    assert(weaponIndex != std::numeric_limits<std::size_t>::max());
    assert(stockTestShop.commitPurchase(weaponIndex));
    assert(stockTestShop.getRemainingStock(ItemId::PULSE_BLADE) == 0);
    assert(findOfferIndex(stockTestShop, ItemId::PULSE_BLADE) ==
           std::numeric_limits<std::size_t>::max());
    assert(stockTestShop.isValid());

    // 非法槽位不得修改任何库存。
    const int hpChipStockBefore = stockTestShop.getRemainingStock(ItemId::HP_CHIP);
    assert(!stockTestShop.commitPurchase(999U));
    assert(stockTestShop.getRemainingStock(ItemId::HP_CHIP) == hpChipStockBefore);

    // refreshOnEveryEntry == false 时，重复进入不会擅自改变货架。
    const std::vector<ShopOffer> offersBeforeReentry = stockTestShop.getOffers();
    assert(stockTestShop.enter());
    assert(stockTestShop.getOffers().size() == offersBeforeReentry.size());
    for (std::size_t index = 0; index < offersBeforeReentry.size(); ++index)
    {
        assert(stockTestShop.getOffers()[index].itemId == offersBeforeReentry[index].itemId);
        assert(stockTestShop.getOffers()[index].price == offersBeforeReentry[index].price);
    }

    // resetStock 恢复初始数量并清空货架，供新游戏初始化使用。
    stockTestShop.resetStock();
    assert(stockTestShop.getRemainingStock(ItemId::MEDKIT) == 10);
    assert(stockTestShop.getRemainingStock(ItemId::PULSE_BLADE) == 1);
    assert(stockTestShop.getOffers().empty());
    assert(stockTestShop.isValid());

    std::cout << "Shop tests passed.\n";
    return 0;
}
