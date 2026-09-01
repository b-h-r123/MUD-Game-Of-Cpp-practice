#pragma once

#include <array>
#include <string>
#include <vector>

// ============================== Item 模块总览 ==============================
//
// 本模块只描述“物品是什么”和“物品携带哪些效果数据”，不直接修改 Player，
// 也不负责商店交易、玩家输入和战斗回合。之后由 Player / Shop / BattleSystem
// 读取这些数据并执行各自职责。这样可以避免 Item 和其他模块相互强耦合。
//
// 当前物品分为三类：
// 1. CONSUMABLE：进入四个固定消耗品槽()，使用成功后数量减 1；
// 2. DIRECT_UPGRADE：芯片，购买后立即加强基础属性，不进入任何物品槽；
// 3. EQUIPMENT：直接占用武器、护盾或配饰槽，替换时旧装备消失。
// ============================================================================

// ItemId 是物品的唯一编号，程序内部应使用它判断物品，而不是比较中文名称。
// 例如存档和商店库存都应保存 ItemId。装备池确定后，只需在末尾继续追加编号，
// 不需要修改 Item 类的整体结构。
enum class ItemId
{
    NONE = 0,              // 空值：表示槽位中没有物品，不能作为真实商品使用。
    MEDKIT,                // 医疗包：立即恢复 HP，可在战斗内外使用。
    ENERGY_CELL,           // 能量电池：战斗中立即恢复 Energy。
    NANO_MEDKIT,           // 纳米急救针：战斗中提供持续 HP 恢复。
    SUPERCONDUCTING_CORE,  // 超导能量核心：战斗中提供持续 Energy 恢复。
    ATTACK_CHIP,           // 强化芯片：购买后立即永久增加基础 ATK。
    HP_CHIP,               // 生命芯片：购买后立即永久增加 maxHp 和当前 HP。

    // 武器：一件基础武器，加上三件分别代表输出类特殊被动的装备。
    PULSE_BLADE,             // 脉冲短刃：只提供稳定攻击加成。
    ARC_CHAIN_BLADE,         // 电弧链刃：攻击命中后追加一次伤害。
    EXECUTION_RAIL_BLADE,    // 处决者磁轨刃：对低生命敌人提高伤害。
    OVERCLOCK_PULSE_RIFLE,   // 超频脉冲枪：提高消耗 Energy 的技能伤害。

    // 护盾：一件基础护盾，加上三件分别代表防御类特殊被动的装备。
    COMPOSITE_SHIELD,        // 复合护盾：每次受伤时提供固定减伤。
    PHASE_DEFLECTOR,         // 相位偏转盾：按比例减少玩家受到的伤害。
    MIRROR_SHIELD,           // 镜面反射盾：反弹部分最终承受伤害。
    AEGIS_ARMOR,             // 神盾装甲：每场战斗首次致命伤害时保留生命。

    // 配饰：一件基础配饰，加上两件分别代表状态类特殊被动的装备。
    NEURAL_AMPLIFIER,        // 神经增幅器：提高最大 Energy。
    BERSERKER_IMPLANT,       // 狂战神经插件：低 HP 时提高输出。
    ZERO_POINT_CAPACITOR,    // 零点电容：每场战斗首次技能减少 Energy 消耗。

    COUNT // 只用于统计和边界校验，不是真实物品。新道具必须添加在 COUNT 之前。
};

// ItemCategory: 决定“获得物品后应该交给哪个流程处理”。
enum class ItemCategory
{
    CONSUMABLE,       // 可叠加消耗品：增加对应固定消耗品槽的数量。
    DIRECT_UPGRADE,   // 即时升级芯片：直接修改 Player 基础属性。
    EQUIPMENT         // 装备：直接安装到一个固定装备槽。
};

// 装备槽类型: NONE 供非装备物品以及空槽使用。
enum class EquipmentSlot
{
    NONE,       // 不属于任何装备槽。
    WEAPON,     // 武器槽：主要影响玩家造成的伤害。
    SHIELD,     // 护盾槽：主要负责减伤和反弹。
    ACCESSORY   // 配饰槽：主要提供属性加成或其他被动。
};

// 物品级别: 只用于配置、商店刷新权重和界面展示，不直接决定实际数值。
enum class ItemTier
{
    BASIC,      // 基础物品，通常更常见、价格更低。
    ADVANCED,   // 高级物品，通常效果更强、刷新概率更低。
    UNIQUE      // 稀有或唯一物品，通常库存极少。
};

// 直接使用时机由物品定义本身携带，避免 UI 或 BattleSystem
// 通过 ItemId 写一大串特例判断。芯片和装备都不是“主动使用”道具。
enum class ItemUseTiming
{
    NOT_DIRECTLY_USABLE, // 由购买或装备替换流程处理，不出现在“使用道具”菜单。
    ANYTIME,             // 战斗内外都可以使用，当前只有医疗包。
    BATTLE_ONLY          // 只能在战斗中使用。
};

// EffectType 描述“一个主动效果会改变什么”。
// 消耗品和购买后立即生效的芯片使用 ItemEffect 保存这些数据。
enum class EffectType
{
    RESTORE_HP,          // 恢复当前 HP，但最终不能超过 maxHp。
    RESTORE_ENERGY,      // 恢复当前 Energy，但最终不能超过 maxEnergy。
    INCREASE_ATTACK,     // 永久增加 Player 的基础 ATK。
    INCREASE_MAX_HP,     // 永久增加基础 maxHp，并按规则同步增加当前 HP。
    INCREASE_MAX_ENERGY, // 增加基础 maxEnergy；具体物品尚未确定。
    BONUS_DAMAGE,        // 对敌人追加伤害，具体结算由 BattleSystem 负责。
    DAMAGE_REDUCTION,    // 减少玩家受到的伤害。
    REFLECT_DAMAGE       // 将部分最终承受伤害反弹给攻击者。
};

// PassiveType 表示装备需要 BattleSystem 在特定战斗节点处理的特殊被动。
// 这里保存“被动的种类”，但不在 Item 模块中直接执行伤害结算。
enum class PassiveType
{
    NONE,                       // 没有特殊被动，只使用普通属性加成。
    FOLLOW_UP_DAMAGE,           // 成功攻击后追加一次伤害，不递归触发自身。
    EXECUTE_DAMAGE,             // 敌人 HP 低于阈值时增加伤害。
    SKILL_DAMAGE_BONUS,         // 使用消耗 Energy 的技能时增加伤害。
    PERCENT_DAMAGE_REDUCTION,   // 按比例降低玩家受到的伤害。
    REFLECT_DAMAGE,             // 按比例反弹玩家最终承受的伤害。
    LETHAL_GUARD,               // 每场战斗首次致命伤害时保留玩家生命。
    LOW_HP_DAMAGE_BONUS,        // 玩家 HP 低于阈值时增加输出。
    FIRST_SKILL_COST_REDUCTION  // 每场战斗第一次使用技能时减少能量消耗。
};

// durationTurns 为 0 时，效果立即生效或永久生效。
// durationTurns 大于 0 时，使用物品的当前回合计为第一个生效回合。
struct ItemEffect
{
    EffectType type;   // 效果类型，决定修改 HP、Energy、ATK 等哪项数据。
    int value;         // 每次生效的数值；必须大于 0，具体大小后续平衡。
    int durationTurns; // 持续回合数；0 表示立即或永久，正数表示临时效果。

    // 构造一条物品效果。duration 默认是 0，适用于医疗包和芯片等效果。
    ItemEffect(EffectType effectType, int effectValue, int duration = 0);

    // 检查配置是否合法：效果值必须为正，持续回合不能为负数。
    bool isValid() const;

    // 返回该效果是否需要在战斗中跨多个回合保存。
    bool isTemporary() const;
};

// PassiveEffect 保存一条装备特殊被动的配置。
// value 和 thresholdPercent 的具体含义由 type 决定，例如反弹比例或 HP 阈值。
struct PassiveEffect
{
    PassiveType type;     // 被动种类；NONE 表示没有特殊被动。
    int value;            // 被动强度，例如追加伤害值或减伤百分比。
    int thresholdPercent; // 触发阈值，范围为 0～100；不需要阈值时使用 0。
    bool oncePerBattle;   // true 表示每场战斗最多触发一次。

    PassiveEffect(
        PassiveType passiveType = PassiveType::NONE,
        int effectValue = 0,
        int threshold = 0,
        bool once = false);

    bool isValid() const;  // 检查强度和百分比范围是否合法。
    bool isActive() const; // 是否真正配置了一个被动，而不是 NONE。
};

// 装备加成由当前已穿戴的装备动态提供，
// 不能将这些加成永久写入玩家的基础属性。
struct EquipmentData
{
    EquipmentSlot slot;       // 安装位置；真实装备不能为 NONE。
    int attackBonus;          // 装备期间提供的攻击加成，不写入基础 ATK。
    int maxHpBonus;           // 装备期间提供的最大生命加成。
    int maxEnergyBonus;       // 装备期间提供的最大能量加成。
    int flatDamageReduction;  // 每次受伤前直接减去的固定伤害值。
    PassiveEffect passive;    // 最多一条特殊被动；NONE 表示没有特殊被动。

    EquipmentData(
        EquipmentSlot equipmentSlot = EquipmentSlot::NONE,
        int attack = 0,
        int maxHp = 0,
        int maxEnergy = 0,
        int damageReduction = 0,
        const PassiveEffect& passiveEffect = PassiveEffect());

    // 所有普通数值必须非负，并且被动配置也必须合法。
    bool isValid() const;

    // 判断装备是否至少有一项有效加成，防止创建“什么都不做”的装备。
    bool hasAnyBonus() const;

    // 判断该结构是否为非装备物品应使用的默认空装备数据。
    bool isEmpty() const;
};

// Item 是物品的只读“定义对象”。
// 它保存名称、价格、效果等配置，但不保存玩家拥有的数量。
class Item
{
public:
    // effects 用于消耗品和即时升级芯片；equipment 只用于装备。
    // 构造后可调用 isValid() 检查类别和数据组合是否合法。
    Item(
        ItemId id,
        const std::string& name,
        const std::string& description,
        ItemCategory category,
        ItemTier tier,
        int price,
        ItemUseTiming useTiming,
        const std::vector<ItemEffect>& effects = std::vector<ItemEffect>(),
        const EquipmentData& equipment = EquipmentData());

    ItemId getId() const;                          // 返回程序内部唯一编号。
    const std::string& getName() const;             // 返回界面显示名称。
    const std::string& getDescription() const;      // 返回物品说明文本。
    ItemCategory getCategory() const;               // 返回获得后的处理方式。
    ItemTier getTier() const;                       // 返回物品层级。
    int getPrice() const;                           // 返回当前配置价格。
    ItemUseTiming getUseTiming() const;             // 返回直接使用的时机限制。
    const std::vector<ItemEffect>& getEffects() const; // 只读访问主动效果列表。
    const EquipmentData& getEquipmentData() const; // 只读访问装备数据。

    bool isConsumable() const;    // 是否应进入固定消耗品槽。
    bool isDirectUpgrade() const; // 是否购买后立即升级且不进入物品槽。
    bool isEquipment() const;     // 是否应安装到固定装备槽。
    bool canUseInBattle() const;  // 是否允许在战斗中主动使用。
    bool canUseOutsideBattle() const; // 是否允许在非战斗状态主动使用。

    // 检查整个 Item 配置是否自洽；只检查配置，不执行物品效果。
    bool isValid() const;

private:
    ItemId id_;                         // 唯一编号，NONE 只表示空值。
    std::string name_;                  // 显示名称，与逻辑判断无关。
    std::string description_;           // 给玩家阅读的效果说明。
    ItemCategory category_;             // 获得物品后的处理路径。
    ItemTier tier_;                     // 商店刷新和展示使用的层级。
    int price_;                         // 商品价格，允许 0，但不能为负数。
    ItemUseTiming useTiming_;           // 消耗品的使用时机；芯片和装备为不可直接使用。
    std::vector<ItemEffect> effects_;   // 消耗品或芯片的一条或多条效果。
    EquipmentData equipment_;           // 装备专用数据；非装备必须保持为空。
};

// ItemCatalog 是项目中唯一的“道具定义表”。
//
// Item 不保存玩家数量，Catalog 也不保存商店库存；它只负责把 ItemId
// 映射到名称、价格和效果。因此 Player、Shop、SaveManager 可以共用同一套配置，
// 而不需要在多个模块中重复写魔法数字。
class ItemCatalog
{
public:
    // 构造时创建全部已确定物品。当前数值是集中的首轮平衡参数，
    // 之后调整平衡时只需修改 item.cpp 中的目录定义。
    ItemCatalog();

    // 返回完整目录的只读引用，供 UI 或测试统计。
    const std::vector<Item>& getAllItems() const;

    // 按唯一编号查找。NONE、COUNT 或不存在的编号都返回空指针。
    // 返回指针的有效期与当前 ItemCatalog 对象一致。
    const Item* findById(ItemId id) const;

    // 以分类或稀有度筛选物品，Shop 可以在返回的候选项中随机刷新。
    std::vector<const Item*> getItemsByCategory(ItemCategory category) const;
    std::vector<const Item*> getItemsByTier(ItemTier tier) const;

    // 只返回指定装备槽的装备；NONE 会返回空 vector。
    std::vector<const Item*> getEquipmentBySlot(EquipmentSlot slot) const;

    // 验证每件物品配置合法、ItemId 无重复，且 COUNT 前没有遗漏定义。
    bool isValid() const;

private:
    // 物品数量很小且需要按顺序展示，vector 比额外引入 map 更直接。
    // 线性查找在当前规模下成本可忽略，也更容易讲清楚。
    std::vector<Item> items_;
};

// ConsumableSlot 表示七格物品槽中的一个“消耗品格”。
// 四个消耗品格的 ItemId 固定，玩家只能增加数量或使用一个，不能把格子换成别的物品。
class ConsumableSlot
{
public:
    // initialCount 允许为 0；负数会使对象处于非法状态，isValid() 返回 false。
    explicit ConsumableSlot(ItemId id = ItemId::NONE, int initialCount = 0);

    ItemId getItemId() const; // 这一固定槽对应哪一种消耗品。
    int getCount() const;     // 当前持有数量；外部只能读取，不能直接改写。

    // 增加数量。amount 必须为正且不能发生 int 溢出；失败时数量不变。
    bool add(int amount);

    // 成功时消耗 1 个；数量为 0 时返回 false，避免出现负数。
    bool consumeOne();

    bool isEmpty() const; // 数量是否恰好为 0。
    bool isValid() const; // ItemId 不能为 NONE，数量不能为负数。

private:
    ItemId itemId_; // 固定槽的道具类型，创建后不能被外部替换。
    int count_;     // 该道具的可用数量，只能通过 add/consumeOne 修改。
};

// ItemSlots 是 Player 最终持有的固定七格物品槽数据：
//
// 0. 医疗包          1. 能量电池
// 2. 纳米急救针      3. 超导能量核心
// 4. 武器            5. 护盾            6. 配饰
//
// 芯片属于 DIRECT_UPGRADE，购买后立即生效，因此不会出现在这里。
class ItemSlots
{
public:
    // 初始状态：医疗包和能量电池各 1 个，另外两个消耗品为 0，三个装备槽为空。
    ItemSlots();

    // UI 和 SaveManager 可以只读遍历前四个固定消耗品槽。
    const std::array<ConsumableSlot, 4>& getConsumableSlots() const;

    // 按 ItemId 查找固定消耗品槽；芯片、装备或未知编号返回空指针。
    const ConsumableSlot* findConsumableSlot(ItemId id) const;

    // 只允许修改前四格中已经固定存在的消耗品；失败时所有槽位保持不变。
    bool addConsumable(ItemId id, int amount);
    bool consumeConsumable(ItemId id);

    // 读取当前武器、护盾或配饰。槽位为空或传入 NONE 时返回 ItemId::NONE。
    ItemId getEquippedItem(EquipmentSlot slot) const;

    // 使用一个合法装备直接覆盖对应装备槽。旧装备不会保存，也没有撤销接口。
    // 传入非装备、非法配置或与当前完全相同的装备时返回 false。
    bool replaceEquipment(const Item& equipment);

    // 检查前四个消耗品槽的种类、顺序和数量是否仍然合法。
    bool isValid() const;

private:
    // 只有 ItemSlots 自己可以取得可写指针；外部只能调用公开的 const 查询版本。
    ConsumableSlot* findMutableConsumableSlot(ItemId id);

    std::array<ConsumableSlot, 4> consumableSlots_; // 固定的前四格消耗品。
    ItemId weapon_;                                // 第五格：当前武器。
    ItemId shield_;                                // 第六格：当前护盾。
    ItemId accessory_;                             // 第七格：当前配饰。
};
