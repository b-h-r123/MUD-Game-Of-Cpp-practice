#pragma once

#include <string>
#include <vector>
// ============================== Room 模块概览 ==============================
//
// Room 是一个“房间数据与状态对象”，负责回答下面几个问题：
// 1. 这是哪个房间，属于哪一种房间？
// 2. 它和哪些房间存在物理连接？
// 3. 它目前是未解锁、可触发内容，还是已经完成？
// 4. 它是否包含战斗，是否可以重复触发内容？
//
// Room 不读取键盘输入、不修改 Player、不执行战斗，也不直接发放宝箱奖励。
// 这些流程由 Game 统一协调。这样可以避免 Room 同时依赖 Player、Enemy、
// BattleSystem 和 Shop，保持类职责简单，也方便单独测试地图数据和状态转换。
//
// Room 提供的是“地图规则和进度状态”，Game 才是“游戏流程协调者”。
// ===========================================================================

// RoomId 是房间在程序内部的稳定身份。
// 逻辑判断、地图连接和存档都应该使用 RoomId，而不是比较中文房间名称。
enum class RoomId
{
    NONE = 0,       // 空值：只用于表示“没有解锁前置”，不是实际房间。
    MAIN_1,         // 主线房 1：新游戏的起点。
    MAIN_2,         // 主线房 2。
    MAIN_3,         // 主线房 3。
    MAIN_4,         // 主线房 4：通关后开放附加怪支线。
    MAIN_5,         // 主线房 5：通关后开放 Boss、黑市和高级宝箱。
    MAIN_6,         // 主线房 6：Boss 房。
    SHOP,           // 普通商店；主线房 3 通关后开放，可重复进入。
    EXTRA_BATTLE,   // 附加怪房，完成后开放精英怪房和普通宝箱房。
    ELITE_BATTLE,   // 精英怪房。
    CHEST,          // 普通宝箱房。
    ADVANCED_CHEST, // 高级宝箱房。
    BLACK_MARKET    // 黑市，可重复进入。
};

// RoomType 表示 Game 进入房间后应该把流程交给哪个模块。
// 例如 MAIN_BATTLE 交给 BattleSystem，SHOP 交给商店界面。
// 普通宝箱和高级宝箱的领取逻辑相同，因此共用 CHEST 类型，身份由 RoomId 区分。
enum class RoomType
{
    MAIN_BATTLE,     // 普通主线战斗。
    BOSS,            // 最终 Boss 战斗。
    SHOP,            // 普通商店服务。
    BLACK_MARKET,    // 黑市服务。
    CHEST,           // 一次性宝箱奖励。
    OPTIONAL_BATTLE, // 普通可选战斗。
    ELITE_BATTLE     // 精英可选战斗。
};

// RoomState 只描述房间内容的进度，不描述“玩家当前站在哪里”。
// 当前所在房间应由 Game 单独保存为 currentRoomId，RoomState 只描述“这个房间的内容是否已经完成”。
enum class RoomState
{
    LOCKED,    // 尚未满足解锁条件，玩家不能进入。
    AVAILABLE, // 已解锁；一次性内容尚未完成，或该房间本来就可重复使用。
    CLEARED    // 一次性内容已经完成，但房间仍可再次进入并作为通路经过。
};

class Room

{
public:
    // 构造一个房间的数据记录。前五项组成地图固定配置，initialState 是新游戏状态。
    // connections 保存物理邻居；unlockAfter 保存进度前置，二者含义不同。
    Room(
        RoomId id,
        const std::string& name,
        RoomType type,
        const std::vector<RoomId>& connections,
        RoomId unlockAfter,
        RoomState initialState = RoomState::LOCKED);

    // 以下 getter 都只读取数据，不会改变房间状态。
    RoomId getId() const;
    const std::string& getName() const;                 // 返回界面显示名称。
    RoomType getType() const;                           // 返回房间内容类型。
    RoomState getState() const;                         // 返回房间解锁状态。
    const std::vector<RoomId>& getConnections() const; // 只读访问相邻房间表。
    RoomId getUnlockPrerequisite() const;               // 返回单一解锁前置。

    // 查询 target 是否出现在 connections_ 中。
    // 这里只判断物理连接，不判断目标房间是否已经解锁；移动时 Game 需要同时检查：
    // currentRoom.isConnectedTo(targetId) && targetRoom.canEnter()。
    bool isConnectedTo(RoomId target) const;

    // AVAILABLE 和 CLEARED 都允许进入；只有 LOCKED 会被拒绝。
    bool canEnter() const;

    // 判断 Game 是否需要为这个房间调度 BattleSystem。
    bool hasBattle() const;

    // 只有商店和黑市是可重复触发的服务；战斗和宝箱都是一次性内容。
    bool isRepeatable() const;

    // 受控状态转换：只允许 LOCKED -> AVAILABLE。
    // 如果房间已经解锁或已经完成，重复调用不会改变状态。
    void unlock();

    // 受控状态转换：只允许一次性房间从 AVAILABLE -> CLEARED。
    // 对 LOCKED、已经 CLEARED、商店或黑市调用时都不会改变状态。
    void markCleared();

private:
    // 固定配置：创建默认地图后不应在运行期间改变。
    RoomId id_;                       // 房间身份，默认地图中必须唯一。
    std::string name_;                // 给玩家阅读的名称，不参与逻辑判断。
    RoomType type_;                   // Game 据此选择战斗、商店或宝箱流程。
    std::vector<RoomId> connections_; // 固定双向邻接表，只在构造时写入。
    RoomId unlockAfter_;              // 单一解锁前置；NONE 表示初始无前置。

    // 运行状态：战斗胜利、领取宝箱或读取存档时才可能发生变化。
    RoomState state_;                 // 唯一会在运行过程中变化的 Room 数据。
};

// 创建《霓虹回响》已经确认的默认地图。
// 地图数据只集中在 Room.cpp 这一处，Game、UI 和 SaveManager 不应各自复制一份。
// 返回的 vector 恰好包含 12 个 Room；MAIN_1 初始 AVAILABLE，其余初始 LOCKED。
std::vector<Room> createDefaultRooms();

// Game、UI 和 SaveManager 统一通过 RoomId 查找房间，避免各模块自行维护下标映射。
// 找不到时返回空指针，使非法输入或损坏的存档不会直接造成数组越界。
Room* findRoomById(std::vector<Room>& rooms, RoomId id);
const Room* findRoomById(const std::vector<Room>& rooms, RoomId id);

