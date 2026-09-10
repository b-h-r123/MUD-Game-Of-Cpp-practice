#include "Shop.h"

#include <algorithm>
#include <limits>

ShopOffer::ShopOffer(ItemId id, int actualPrice)
    : itemId(id), price(actualPrice)
{
}

bool ShopOffer::isValid() const
{
    const int numericId = static_cast<int>(itemId);
    return numericId > static_cast<int>(ItemId::NONE) &&
           numericId < static_cast<int>(ItemId::COUNT) &&
           price >= 0;
}

Shop::StockEntry::StockEntry(ItemId id, int initialStock)
    : itemId(id), remaining(initialStock)
{
}

Shop::Shop(
    ShopType type,
    const ItemCatalog& catalog,
    const ShopConfig& config)
    : Shop(type, catalog, config, std::random_device()())
{
}

Shop::Shop(
    ShopType type,
    const ItemCatalog& catalog,
    const ShopConfig& config,
    unsigned int randomSeed)
    : type_(type),
      catalog_(catalog),
      config_(config),
      randomEngine_(randomSeed),
      hasEntered_(false)
{
    resetStock();
}

ShopType Shop::getType() const
{
    return type_;
}

const ShopConfig& Shop::getConfig() const
{
    return config_;
}

bool Shop::enter()
{
    // 首次进入时不管配置如何都要生成货架。
    // 重复进入时，只有 refreshOnEveryEntry == true 才更换当前商品。
    if (!hasEntered_ || config_.refreshOnEveryEntry)
    {
        if (!refresh())
        {
            return false;
        }
    }

    hasEntered_ = true;
    return true;
}

bool Shop::refresh()
{
    // 如果目录或配置本身非法，不能用新的半成品覆盖旧货架。
    if (!catalog_.isValid() || !config_.isValid())
    {
        return false;
    }

    const ShopModeConfig& mode = config_.getMode(type_);
    std::vector<const Item*> candidates = buildCandidates();
    std::vector<ShopOffer> refreshedOffers;
    refreshedOffers.reserve(mode.slotCount);

    while (refreshedOffers.size() < mode.slotCount && !candidates.empty())
    {
        const Item* selected = chooseCandidate(candidates);
        if (selected == 0)
        {
            // 当前剩余候选项的权重全为 0，说明本次可刷新项已经耗尽。
            break;
        }

        const int actualPrice = calculatePrice(*selected);
        if (actualPrice < 0)
        {
            return false;
        }

        refreshedOffers.push_back(ShopOffer(selected->getId(), actualPrice));

        if (!config_.allowDuplicateOffers)
        {
            // 默认不允许同一货架重复出现同一件物品。
            candidates.erase(
                std::remove(candidates.begin(), candidates.end(), selected),
                candidates.end());
        }
    }

    // 即使所有库存都已卖完，“刷新出空货架”仍然是一次合法结果。
    offers_ = refreshedOffers;
    return true;
}

const std::vector<ShopOffer>& Shop::getOffers() const
{
    return offers_;
}

const ShopOffer* Shop::getOffer(std::size_t slotIndex) const
{
    return slotIndex < offers_.size() ? &offers_[slotIndex] : 0;
}

int Shop::getRemainingStock(ItemId id) const
{
    const StockEntry* entry = findStock(id);
    return entry == 0 ? 0 : entry->remaining;
}

bool Shop::commitPurchase(std::size_t slotIndex)
{
    if (slotIndex >= offers_.size())
    {
        return false;
    }

    const ItemId purchasedId = offers_[slotIndex].itemId;
    StockEntry* entry = findMutableStock(purchasedId);
    if (entry == 0 || entry->remaining <= 0)
    {
        return false;
    }

    --entry->remaining;

    if (entry->remaining == 0)
    {
        // 当允许重复商品时，同一 ItemId 可能占据多个槽位，
        // 因此库存归零后要一次移除全部同类槽位。
        offers_.erase(
            std::remove_if(
                offers_.begin(),
                offers_.end(),
                [purchasedId](const ShopOffer& offer) {
                    return offer.itemId == purchasedId;
                }),
            offers_.end());
    }

    return true;
}

void Shop::resetStock()
{
    std::vector<StockEntry> newStock;
    const std::vector<Item>& items = catalog_.getAllItems();
    newStock.reserve(items.size());

    for (std::vector<Item>::const_iterator it = items.begin(); it != items.end(); ++it)
    {
        newStock.push_back(StockEntry(it->getId(), config_.getInitialStock(*it)));
    }

    stock_ = newStock;
    offers_.clear();
    hasEntered_ = false;
}

bool Shop::isValid() const
{
    if (!catalog_.isValid() || !config_.isValid() ||
        stock_.size() != catalog_.getAllItems().size())
    {
        return false;
    }

    // 库存必须与 Catalog 一一对应，且不能超过配置给定的初始上限。
    for (std::vector<StockEntry>::const_iterator it = stock_.begin(); it != stock_.end(); ++it)
    {
        const Item* item = catalog_.findById(it->itemId);
        if (item == 0 || it->remaining < 0 ||
            it->remaining > config_.getInitialStock(*item))
        {
            return false;
        }
    }

    const ShopModeConfig& mode = config_.getMode(type_);
    if (offers_.size() > mode.slotCount)
    {
        return false;
    }

    for (std::size_t index = 0; index < offers_.size(); ++index)
    {
        const ShopOffer& offer = offers_[index];
        const Item* item = catalog_.findById(offer.itemId);
        if (!offer.isValid() || item == 0 || getRemainingStock(offer.itemId) <= 0 ||
            mode.poolWeights.getWeight(classifyItem(*item)) <= 0 ||
            offer.price != calculatePrice(*item))
        {
            return false;
        }

        if (!config_.allowDuplicateOffers)
        {
            for (std::size_t previous = 0; previous < index; ++previous)
            {
                if (offers_[previous].itemId == offer.itemId)
                {
                    return false;
                }
            }
        }
    }

    return true;
}

const Shop::StockEntry* Shop::findStock(ItemId id) const
{
    for (std::vector<StockEntry>::const_iterator it = stock_.begin(); it != stock_.end(); ++it)
    {
        if (it->itemId == id)
        {
            return &(*it);
        }
    }

    return 0;
}

Shop::StockEntry* Shop::findMutableStock(ItemId id)
{
    for (std::vector<StockEntry>::iterator it = stock_.begin(); it != stock_.end(); ++it)
    {
        if (it->itemId == id)
        {
            return &(*it);
        }
    }

    return 0;
}

std::vector<const Item*> Shop::buildCandidates() const
{
    std::vector<const Item*> candidates;
    const std::vector<Item>& items = catalog_.getAllItems();
    const ShopModeConfig& mode = config_.getMode(type_);

    for (std::vector<Item>::const_iterator it = items.begin(); it != items.end(); ++it)
    {
        if (getRemainingStock(it->getId()) > 0 &&
            mode.poolWeights.getWeight(classifyItem(*it)) > 0)
        {
            candidates.push_back(&(*it));
        }
    }

    return candidates;
}

ShopPool Shop::classifyItem(const Item& item) const
{
    // 两种芯片与 BASIC 消耗品共同归入“常规补给”池。
    if (item.isDirectUpgrade() ||
        (item.isConsumable() && item.getTier() == ItemTier::BASIC))
    {
        return ShopPool::BASIC_SUPPLY;
    }

    // 当前两种 ADVANCED 消耗品就是全部“高级道具”。
    if (item.isConsumable())
    {
        return ShopPool::ADVANCED_ITEM;
    }

    // BASIC 装备属于普通装备；ADVANCED 和 UNIQUE 都进入高级装备池。
    if (item.isEquipment())
    {
        return item.getTier() == ItemTier::BASIC
                   ? ShopPool::EQUIPMENT
                   : ShopPool::ADVANCED_EQUIPMENT;
    }

    return ShopPool::NONE;
}

const Item* Shop::chooseCandidate(const std::vector<const Item*>& candidates)
{
    std::vector<const Item*> basicSupplies;
    std::vector<const Item*> advancedItems;
    std::vector<const Item*> equipment;
    std::vector<const Item*> advancedEquipment;

    for (std::vector<const Item*>::const_iterator it = candidates.begin(); it != candidates.end(); ++it)
    {
        switch (classifyItem(**it))
        {
        case ShopPool::BASIC_SUPPLY:
            basicSupplies.push_back(*it);
            break;
        case ShopPool::ADVANCED_ITEM:
            advancedItems.push_back(*it);
            break;
        case ShopPool::EQUIPMENT:
            equipment.push_back(*it);
            break;
        case ShopPool::ADVANCED_EQUIPMENT:
            advancedEquipment.push_back(*it);
            break;
        case ShopPool::NONE:
            break;
        }
    }

    const ShopPoolWeightConfig& weights = config_.getMode(type_).poolWeights;
    const long long basicSupplyWeight = basicSupplies.empty() ? 0LL : weights.basicSupply;
    const long long advancedItemWeight = advancedItems.empty() ? 0LL : weights.advancedItem;
    const long long equipmentWeight = equipment.empty() ? 0LL : weights.equipment;
    const long long advancedEquipmentWeight =
        advancedEquipment.empty() ? 0LL : weights.advancedEquipment;
    const long long totalWeight =
        basicSupplyWeight + advancedItemWeight +
        equipmentWeight + advancedEquipmentWeight;

    if (totalWeight <= 0)
    {
        return 0;
    }

    std::uniform_int_distribution<long long> poolRoll(1LL, totalWeight);
    const long long rolled = poolRoll(randomEngine_);

    const std::vector<const Item*>* selectedPool = 0;
    if (rolled <= basicSupplyWeight)
    {
        selectedPool = &basicSupplies;
    }
    else if (rolled <= basicSupplyWeight + advancedItemWeight)
    {
        selectedPool = &advancedItems;
    }
    else if (rolled <= basicSupplyWeight + advancedItemWeight + equipmentWeight)
    {
        selectedPool = &equipment;
    }
    else
    {
        selectedPool = &advancedEquipment;
    }

    std::uniform_int_distribution<std::size_t> itemRoll(0U, selectedPool->size() - 1U);
    return (*selectedPool)[itemRoll(randomEngine_)];
}

int Shop::calculatePrice(const Item& item) const
{
    const long long basePrice = item.getPrice();
    const long long pricePercent = config_.getMode(type_).pricePercent;

    // +99 后再整除 100，实现正整数价格的向上取整。
    // 使用 long long 计算，避免 int 乘法在校验前先溢出。
    const long long adjustedPrice = (basePrice * pricePercent + 99LL) / 100LL;
    if (adjustedPrice < 0 || adjustedPrice > std::numeric_limits<int>::max())
    {
        return -1;
    }

    return static_cast<int>(adjustedPrice);
}
