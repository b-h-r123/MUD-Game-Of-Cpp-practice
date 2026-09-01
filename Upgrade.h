#pragma once

#include "item.h"

// ============================== Upgrade 总览 ==============================
//
// Upgrade 负责两件事：
// 1. 保存玩家的等级和当前 EXP，并按照“当前等级 × 经验系数”处理升级；
// 2. 每升一级，自动应用一次强化芯片和一次生命芯片的永久效果。
//
// 当前没有可分配属性点、升级菜单或技能树。玩家不需要在 HP 和 ATK 之间
// 做选择；升级成功后两项都会增长。芯片的具体增量仍由 ItemCatalog 唯一定义，
// Upgrade 不重复写 +3 ATK、+20 maxHp 等平衡数字。
//
// Upgrade 不负责敌人掉落多少 EXP、商店扣金币、库存、输入界面和存档文件。
// Game / BattleSystem 只需在结算 EXP 时调用 addExperience()。
// ============================================================================

// 集中保存等级系统自身的平衡参数。
// 芯片效果不属于这里，仍在 ItemCatalog 中调整。
struct UpgradeConfig
{
    int maxLevel;           // 等级上限，默认 Lv10。
    int experiencePerLevel; // 下一级需求 = 当前等级 × 此系数，默认 100。

    UpgradeConfig(int levelLimit = 10, int experienceFactor = 100);

    // 两项必须为正，并且最高一级的需求不能超出 int 范围。
    bool isValid() const;
};

class Upgrade
{
public:
    // 新游戏默认从 Lv1、0 EXP 开始；其余参数主要供读取存档和测试使用。
    explicit Upgrade(
        const UpgradeConfig& config = UpgradeConfig(),
        int initialLevel = 1,
        int initialExperience = 0);

    const UpgradeConfig& getConfig() const;
    int getLevel() const;
    int getExperience() const;

    bool isMaxLevel() const;

    // 返回升下一级需要的 EXP；满级或当前进度非法时返回 0。
    int getRequiredExperienceForNextLevel() const;

    // 增加 EXP，并一次处理所有能够连续达到的等级。
    // 每升一级都会自动应用 ItemCatalog 中 ATTACK_CHIP 和 HP_CHIP 的效果。
    //
    // levelsGained 返回本次实际提升的等级数。非正 EXP、目录/进度/玩家状态
    // 非法或任一计算溢出时返回 false，并保证等级、EXP、HP、maxHp、ATK
    // 全部保持原值。
    bool addExperience(
        int amount,
        const ItemCatalog& catalog,
        int& currentHp,
        int& maxHp,
        int& attack,
        int& levelsGained);

    // 单独应用一枚升级芯片，供商店购买芯片时复用同一套效果执行逻辑。
    // 只接受合法、永久的 DIRECT_UPGRADE 物品；失败时属性全部不变。
    bool applyChip(
        const Item& chip,
        int& currentHp,
        int& maxHp,
        int& attack) const;

    // 检查配置、等级和剩余 EXP 是否自洽。
    bool isValid() const;

private:
    UpgradeConfig config_; // 当前玩家使用的等级参数快照。
    int level_;            // 当前等级，范围为 1～maxLevel。
    int experience_;       // 扣除升级消耗后保留的溢出 EXP。
};
