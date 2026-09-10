#include "Upgrade.h"

#include <cassert>
#include <iostream>
#include <limits>

int main()
{
    const ItemCatalog catalog;
    assert(catalog.isValid());

    const Item* attackChip = catalog.findById(ItemId::ATTACK_CHIP);
    const Item* hpChip = catalog.findById(ItemId::HP_CHIP);
    const Item* medkit = catalog.findById(ItemId::MEDKIT);
    assert(attackChip != 0 && hpChip != 0 && medkit != 0);

    // -------------------------- 默认进度和经验公式 --------------------------
    const UpgradeConfig defaultConfig;
    assert(defaultConfig.isValid());
    assert(defaultConfig.maxLevel == 10);
    assert(defaultConfig.experiencePerLevel == 100);

    Upgrade progress;
    assert(progress.isValid());
    assert(progress.getLevel() == 1);
    assert(progress.getExperience() == 0);
    assert(progress.getRequiredExperienceForNextLevel() == 100);
    assert(!progress.isMaxLevel());

    int currentHp = 60;
    int maxHp = 100;
    int attack = 10;
    int levelsGained = -1;

    // 非正经验不得修改任何进度或属性。
    assert(!progress.addExperience(
        0, catalog, currentHp, maxHp, attack, levelsGained));
    assert(levelsGained == 0);
    assert(progress.getLevel() == 1 && progress.getExperience() == 0);
    assert(currentHp == 60 && maxHp == 100 && attack == 10);

    // 未达到阈值时只积累 EXP，不提前应用芯片效果。
    assert(progress.addExperience(
        99, catalog, currentHp, maxHp, attack, levelsGained));
    assert(levelsGained == 0);
    assert(progress.getLevel() == 1 && progress.getExperience() == 99);
    assert(currentHp == 60 && maxHp == 100 && attack == 10);

    // Lv1 -> Lv2：同时获得一次强化芯片和生命芯片效果。
    assert(progress.addExperience(
        2, catalog, currentHp, maxHp, attack, levelsGained));
    assert(levelsGained == 1);
    assert(progress.getLevel() == 2);
    assert(progress.getExperience() == 1); // 99 + 2 - 100。
    assert(currentHp == 80);
    assert(maxHp == 120);
    assert(attack == 13);
    assert(progress.getRequiredExperienceForNextLevel() == 200);

    // -------------------------- 一次结算连续升级 --------------------------
    Upgrade continuousProgress;
    currentHp = 40;
    maxHp = 100;
    attack = 10;
    assert(continuousProgress.addExperience(
        650, catalog, currentHp, maxHp, attack, levelsGained));
    assert(levelsGained == 3);
    assert(continuousProgress.getLevel() == 4);
    assert(continuousProgress.getExperience() == 50); // 650 - 100 - 200 - 300。
    assert(currentHp == 100); // 三次生命芯片效果，共 +60。
    assert(maxHp == 160);
    assert(attack == 19);     // 三次强化芯片效果，共 +9。

    // -------------------------- Lv10 上限 --------------------------
    Upgrade maxLevelProgress;
    currentHp = 100;
    maxHp = 100;
    attack = 10;
    assert(maxLevelProgress.addExperience(
        4500, catalog, currentHp, maxHp, attack, levelsGained));
    assert(levelsGained == 9);
    assert(maxLevelProgress.getLevel() == 10);
    assert(maxLevelProgress.getExperience() == 0);
    assert(maxLevelProgress.isMaxLevel());
    assert(maxLevelProgress.getRequiredExperienceForNextLevel() == 0);
    assert(currentHp == 280 && maxHp == 280); // 9 × 20 HP。
    assert(attack == 37);                     // 10 + 9 × 3 ATK。

    // 满级后继续获得 EXP 不再增加等级，也不得再次应用芯片效果。
    assert(maxLevelProgress.addExperience(
        123, catalog, currentHp, maxHp, attack, levelsGained));
    assert(levelsGained == 0);
    assert(maxLevelProgress.getExperience() == 123);
    assert(currentHp == 280 && maxHp == 280 && attack == 37);

    // -------------------------- 商店芯片复用同一效果执行器 --------------------------
    Upgrade chipExecutor;
    currentHp = 60;
    maxHp = 100;
    attack = 10;
    assert(chipExecutor.applyChip(*attackChip, currentHp, maxHp, attack));
    assert(currentHp == 60 && maxHp == 100 && attack == 13);
    assert(chipExecutor.applyChip(*hpChip, currentHp, maxHp, attack));
    assert(currentHp == 80 && maxHp == 120 && attack == 13);

    // 消耗品不能伪装成升级芯片进入永久属性流程。
    assert(!chipExecutor.applyChip(*medkit, currentHp, maxHp, attack));
    assert(currentHp == 80 && maxHp == 120 && attack == 13);

    // -------------------------- 非法玩家状态必须零修改 --------------------------
    Upgrade invalidPlayerProgress;
    currentHp = 101;
    maxHp = 100;
    attack = 10;
    assert(!invalidPlayerProgress.addExperience(
        100, catalog, currentHp, maxHp, attack, levelsGained));
    assert(levelsGained == 0);
    assert(invalidPlayerProgress.getLevel() == 1);
    assert(invalidPlayerProgress.getExperience() == 0);
    assert(currentHp == 101 && maxHp == 100 && attack == 10);

    // -------------------------- 溢出时完整回滚 --------------------------
    Upgrade attackOverflowProgress;
    currentHp = 100;
    maxHp = 100;
    attack = std::numeric_limits<int>::max() - 2;
    assert(!attackOverflowProgress.addExperience(
        100, catalog, currentHp, maxHp, attack, levelsGained));
    assert(attackOverflowProgress.getLevel() == 1);
    assert(attackOverflowProgress.getExperience() == 0);
    assert(currentHp == 100 && maxHp == 100);
    assert(attack == std::numeric_limits<int>::max() - 2);

    Upgrade hpOverflowProgress;
    currentHp = std::numeric_limits<int>::max() - 10;
    maxHp = std::numeric_limits<int>::max() - 10;
    attack = 10;
    assert(!hpOverflowProgress.addExperience(
        100, catalog, currentHp, maxHp, attack, levelsGained));
    assert(hpOverflowProgress.getLevel() == 1);
    assert(hpOverflowProgress.getExperience() == 0);
    assert(currentHp == std::numeric_limits<int>::max() - 10);
    assert(maxHp == std::numeric_limits<int>::max() - 10);
    assert(attack == 10);

    // EXP 本身溢出时也不能改变满级进度或属性。
    Upgrade expOverflowProgress(
        defaultConfig,
        10,
        std::numeric_limits<int>::max());
    currentHp = 100;
    maxHp = 100;
    attack = 10;
    assert(expOverflowProgress.isValid());
    assert(!expOverflowProgress.addExperience(
        1, catalog, currentHp, maxHp, attack, levelsGained));
    assert(expOverflowProgress.getExperience() == std::numeric_limits<int>::max());
    assert(currentHp == 100 && maxHp == 100 && attack == 10);

    // -------------------------- 非法配置或存档状态 --------------------------
    assert(!UpgradeConfig(0, 100).isValid());
    assert(!UpgradeConfig(10, 0).isValid());
    assert(!UpgradeConfig(std::numeric_limits<int>::max(), 2).isValid());

    // Lv1 已有 100 EXP 表示本应升级却未结算，因此该存档状态非法。
    Upgrade inconsistentProgress(defaultConfig, 1, 100);
    assert(!inconsistentProgress.isValid());
    currentHp = 100;
    maxHp = 100;
    attack = 10;
    assert(!inconsistentProgress.addExperience(
        1, catalog, currentHp, maxHp, attack, levelsGained));
    assert(currentHp == 100 && maxHp == 100 && attack == 10);

    std::cout << "Upgrade and EXP tests passed.\n";
    return 0;
}
