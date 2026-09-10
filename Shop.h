#pragma once

#include "ShopConfig.h"

#include <cstddef>
#include <random>
#include <vector>

// ShopOffer 是玩家当前看到的一个货架槽位。
// 它只保存稳定的 ItemId 和本商店实际售价，物品详细信息通过 ItemCatalog 查询。
struct ShopOffer
{
    ItemId itemId; // 该槽位显示的物品。
    int price;     // 已应用普通商店/黑市价格倍率后的单价。

    ShopOffer(ItemId id = ItemId::NONE, int actualPrice = 0);
    bool isValid() const;
};

// Shop 只负责三件事：
// 1. 根据 ShopConfig 和 ItemCatalog 刷新货架；
// 2. 保存每种商品的剩余库存；
// 3. 在外部已完成“扣金币 + 交付物品”后确认消耗一件库存。
//
// Shop 不直接依赖 Player，因为 Player 的金币和属性接口尚未由团队确定。
// 后续由 Game 协调 Player 购买成功后，再调用 commitPurchase()。
class Shop
{
public:
    // 正常游戏使用的构造函数：自动使用随机种子。
    Shop(
        ShopType type,
        const ItemCatalog& catalog,
        const ShopConfig& config = ShopConfig());

    // 可重现构造函数：测试时传入固定 seed，保证每次刷新序列一致。
    Shop(
        ShopType type,
        const ItemCatalog& catalog,
        const ShopConfig& config,
        unsigned int randomSeed);

    ShopType getType() const;
    const ShopConfig& getConfig() const;

    // 进入商店。首次进入必定生成货架；后续是否刷新由 refreshOnEveryEntry 决定。
    bool enter();

    // 手动刷新当前货架。构建成功后才替换旧货架，失败时不留下半成品。
    bool refresh();

    const std::vector<ShopOffer>& getOffers() const;
    const ShopOffer* getOffer(std::size_t slotIndex) const;

    // 查询某件物品在当前 Shop 实例中的剩余数量。
    // NONE、COUNT 或不存在的 ItemId 返回 0。
    int getRemainingStock(ItemId id) const;

    // 仅当 Player 侧的扣金币与发放物品已经整体成功时调用。
    // 成功时库存减 1；库存归零时移除货架上该物品的所有槽位。
    bool commitPurchase(std::size_t slotIndex);

    // 新游戏或测试时恢复默认库存，并清空当前货架。
    void resetStock();

    // 检查配置、库存和当前货架是否仍然自洽。
    bool isValid() const;

private:
    struct StockEntry
    {
        ItemId itemId;
        int remaining;

        StockEntry(ItemId id, int initialStock);
    };

    const StockEntry* findStock(ItemId id) const;
    StockEntry* findMutableStock(ItemId id);

    // 从当前仍有库存且本商店权重大于 0 的物品中构造候选池。
    std::vector<const Item*> buildCandidates() const;

    // 根据 Item 的类别和稀有度，将其归入已确认的四个商品池之一。
    ShopPool classifyItem(const Item& item) const;

    // 先按四个商品池的相对权重选一池，再在池内等概率选具体物品。
    const Item* chooseCandidate(const std::vector<const Item*>& candidates);

    // 将 ItemCatalog 基础价乘以当前商店的百分比，并向上取整。
    int calculatePrice(const Item& item) const;

    ShopType type_;
    const ItemCatalog& catalog_;       // 只读借用目录，其生命期必须长于 Shop。
    ShopConfig config_;               // 构造时复制配置，避免外部中途修改造成状态混乱。
    std::vector<StockEntry> stock_;   // 每种物品的持久库存。
    std::vector<ShopOffer> offers_;   // 当前一次进入时看到的货架。
    std::mt19937 randomEngine_;       // C++ STL 伪随机数引擎。
    bool hasEntered_;                // 区分首次进入和重复进入。
};
