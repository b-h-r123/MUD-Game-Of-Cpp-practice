#include "Upgrade.h"

#include <limits>

namespace
{
// Upgrade 接口目前接收 Player 的三个基础属性引用。
// 在真正计算前统一校验，避免不同入口采用不同边界规则。
bool isPlayerBaseStateValid(int currentHp, int maxHp, int attack)
{
    return currentHp >= 0 && maxHp > 0 && currentHp <= maxHp && attack >= 0;
}
} // namespace

UpgradeConfig::UpgradeConfig(int levelLimit, int experienceFactor)
    : maxLevel(levelLimit),
      experiencePerLevel(experienceFactor)
{
}

bool UpgradeConfig::isValid() const
{
    if (maxLevel <= 0 || experiencePerLevel <= 0)
    {
        return false;
    }

    // 使用 long long 验证乘法，避免检查表达式本身先发生 int 溢出。
    const long long highestRequirement =
        static_cast<long long>(maxLevel) * experiencePerLevel;
    return highestRequirement <= std::numeric_limits<int>::max();
}

Upgrade::Upgrade(
    const UpgradeConfig& config,
    int initialLevel,
    int initialExperience)
    : config_(config),
      level_(initialLevel),
      experience_(initialExperience)
{
}

const UpgradeConfig& Upgrade::getConfig() const
{
    return config_;
}

int Upgrade::getLevel() const
{
    return level_;
}

int Upgrade::getExperience() const
{
    return experience_;
}

bool Upgrade::isMaxLevel() const
{
    return config_.isValid() && level_ == config_.maxLevel;
}

int Upgrade::getRequiredExperienceForNextLevel() const
{
    if (!isValid() || isMaxLevel())
    {
        return 0;
    }

    return level_ * config_.experiencePerLevel;
}

bool Upgrade::addExperience(
    int amount,
    const ItemCatalog& catalog,
    int& currentHp,
    int& maxHp,
    int& attack,
    int& levelsGained)
{
    levelsGained = 0;
    if (amount <= 0 || !isValid() || !catalog.isValid() ||
        !isPlayerBaseStateValid(currentHp, maxHp, attack))
    {
        return false;
    }

    // 每级固定使用这两件“规则物品”的效果，但增量来自目录定义。
    // 如果目录缺项或类别被错误修改，整个经验结算必须失败。
    const Item* attackChip = catalog.findById(ItemId::ATTACK_CHIP);
    const Item* hpChip = catalog.findById(ItemId::HP_CHIP);
    if (attackChip == 0 || hpChip == 0)
    {
        return false;
    }

    // 等级、EXP 和玩家属性全部先复制到临时变量。
    // 即使连续升级的最后一级发生溢出，前面各级也不会被部分提交。
    int nextLevel = level_;
    long long nextExperience = static_cast<long long>(experience_) + amount;
    int nextCurrentHp = currentHp;
    int nextMaxHp = maxHp;
    int nextAttack = attack;
    int gained = 0;

    while (nextLevel < config_.maxLevel)
    {
        const long long requiredExperience =
            static_cast<long long>(nextLevel) * config_.experiencePerLevel;
        if (nextExperience < requiredExperience)
        {
            break;
        }

        // 先在临时属性上执行两枚芯片，再确认本级升级。
        // applyChip() 自身也是原子操作；外层临时副本进一步保证连续升级原子性。
        if (!applyChip(*attackChip, nextCurrentHp, nextMaxHp, nextAttack) ||
            !applyChip(*hpChip, nextCurrentHp, nextMaxHp, nextAttack))
        {
            return false;
        }

        nextExperience -= requiredExperience;
        ++nextLevel;
        ++gained;
    }

    if (nextExperience > std::numeric_limits<int>::max())
    {
        return false;
    }

    // 到这里，经验、每一级芯片效果以及整数边界都已验证，再统一提交。
    level_ = nextLevel;
    experience_ = static_cast<int>(nextExperience);
    currentHp = nextCurrentHp;
    maxHp = nextMaxHp;
    attack = nextAttack;
    levelsGained = gained;
    return true;
}

bool Upgrade::applyChip(
    const Item& chip,
    int& currentHp,
    int& maxHp,
    int& attack) const
{
    if (!chip.isValid() || !chip.isDirectUpgrade() ||
        !isPlayerBaseStateValid(currentHp, maxHp, attack))
    {
        return false;
    }

    // 所有效果先应用到 long long 临时值，防止半更新和 int 加法溢出。
    long long nextCurrentHp = currentHp;
    long long nextMaxHp = maxHp;
    long long nextAttack = attack;

    const std::vector<ItemEffect>& effects = chip.getEffects();
    for (std::vector<ItemEffect>::const_iterator it = effects.begin();
         it != effects.end();
         ++it)
    {
        // 升级芯片只能修改永久基础属性，不能携带战斗持续时间。
        if (!it->isValid() || it->isTemporary())
        {
            return false;
        }

        switch (it->type)
        {
        case EffectType::INCREASE_ATTACK:
            nextAttack += it->value;
            break;

        case EffectType::INCREASE_MAX_HP:
            // 生命上限和当前生命同步增加，而不是把玩家直接恢复至满血。
            nextCurrentHp += it->value;
            nextMaxHp += it->value;
            break;

        default:
            return false;
        }
    }

    const long long intMaximum = std::numeric_limits<int>::max();
    if (nextCurrentHp > intMaximum || nextMaxHp > intMaximum ||
        nextAttack > intMaximum)
    {
        return false;
    }

    currentHp = static_cast<int>(nextCurrentHp);
    maxHp = static_cast<int>(nextMaxHp);
    attack = static_cast<int>(nextAttack);
    return true;
}

bool Upgrade::isValid() const
{
    if (!config_.isValid() || level_ < 1 || level_ > config_.maxLevel ||
        experience_ < 0)
    {
        return false;
    }

    // 未满级时，剩余 EXP 必须不足以再次升级；否则存档状态不自洽。
    if (level_ < config_.maxLevel)
    {
        const long long requiredExperience =
            static_cast<long long>(level_) * config_.experiencePerLevel;
        return experience_ < requiredExperience;
    }

    // 满级后不再消耗经验，但允许保存之后继续获得的非负 EXP。
    return true;
}
