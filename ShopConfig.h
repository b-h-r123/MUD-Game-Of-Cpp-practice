#pragma once

#include "item.h"

#include <cstddef>

// ============================ ShopConfig 总览 ============================
//
// 这个文件只放“平衡性参数”，不执行随机刷新、购买或输入输出。
// 使用方式类似训练模型时先创建 config，再把 config 交给 Shop：
//
// ShopConfig config;
// config.normalShop.slotCount = 4;
// config.normalShop.poolWeights.advancedItem = 35;
// config.normalShop.poolWeights.advancedEquipment = 1;
// config.blackMarket.pricePercent = 115;
// Shop normalShop(ShopType::NORMAL, catalog, config);
//
// 这样试玩后调整槽位、权重、库存和价格倍率时，不需要改 Shop 算法。
// ==========================================================================

enum class ShopType
{
    NORMAL,       // 4 槽普通商店，基础物品权重更高。
    BLACK_MARKET  // 6 槽黑市，高级和稀有物品权重更高。
};

// 商店随机刷新直接使用四个商品池，它们与玩家看到的商品定位一致。
enum class ShopPool
{
    NONE,               // 非法或未归类物品。
    BASIC_SUPPLY,       // 医疗包、能量电池、强化芯片、生命芯片。
    ADVANCED_ITEM,      // 纳米急救针、超导能量核心。
    EQUIPMENT,          // BASIC 武器、护盾和配饰。
    ADVANCED_EQUIPMENT  // ADVANCED 与 UNIQUE 装备。
};

// 四个数字是相对权重，不要求加起来等于 100。
// 例如 50 / 35 / 24 / 1 会被自动按总和 110 归一化。
struct ShopPoolWeightConfig
{
    int basicSupply;       // 常规补给和芯片的刷新权重。
    int advancedItem;      // 两种高级消耗品的刷新权重。
    int equipment;         // BASIC 装备的刷新权重。
    int advancedEquipment; // ADVANCED / UNIQUE 装备的刷新权重。

    ShopPoolWeightConfig(
        int basicSupplyWeight = 0,
        int advancedItemWeight = 0,
        int equipmentWeight = 0,
        int advancedEquipmentWeight = 0);

    int getWeight(ShopPool pool) const;
    bool isValid() const;
};

// 普通商店和黑市各自拥有一组模式参数。
struct ShopModeConfig
{
    std::size_t slotCount;       // 进入商店时展示的商品槽位数。
    ShopPoolWeightConfig poolWeights; // 四个商品池的相对刷新权重。
    int pricePercent;            // 售价相对 ItemCatalog 基础价的百分比，100 表示原价。

    ShopModeConfig(
        std::size_t slots = 0,
        const ShopPoolWeightConfig& weights = ShopPoolWeightConfig(),
        int priceMultiplierPercent = 100);

    bool isValid() const;
};

// ShopConfig 是 Shop 所有可调平衡参数的唯一入口。
// 字段故意保持 public，便于在创建 Shop 前像修改 Python config 一样直接调整。
struct ShopConfig
{
    ShopModeConfig normalShop;       // 普通商店的槽位、权重和价格。
    ShopModeConfig blackMarket;      // 黑市的槽位、权重和价格。

    int consumableInitialStock;      // 每种消耗品的初始可购买数量。
    int directUpgradeInitialStock;   // 每种属性芯片的初始可购买数量。
    int equipmentInitialStock;       // 每件装备的初始库存，默认为 1。

    bool refreshOnEveryEntry;        // true：每次重新进入都刷新货架。
    bool allowDuplicateOffers;       // true：同一次刷新允许多个槽位出现同一物品。

    // 默认数值集中定义在 ShopConfig.cpp 的构造函数中。
    ShopConfig();

    const ShopModeConfig& getMode(ShopType type) const;

    // 根据物品类别返回初始库存。这里只读 Item 分类，不修改物品。
    int getInitialStock(const Item& item) const;

    // 防止零槽位、负库存、非法权重或非正价格倍率进入 Shop。
    bool isValid() const;
};
