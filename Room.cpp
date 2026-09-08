#include "Room.h"
#include <algorithm>

// ================================ Room =================================

Room::Room(
    RoomId id,
    const std::string& name,
    RoomType type,
    const std::vector<RoomId>& connections,
    RoomId unlockAfter,
    RoomState initialState)
    : id_(id),
      name_(name),
      type_(type),
      connections_(connections),
      unlockAfter_(unlockAfter),
      state_(initialState)
{
    // 构造阶段只建立一条房间记录，不触发战斗、奖励或连锁解锁
    // 所有房间先完整创建，之后再由 Game 根据流程修改 state_
}

RoomId Room::getId() const
{
    return id_;
}

const std::string& Room::getName() const
{
    return name_;
}

RoomType Room::getType() const
{
    return type_;
}

RoomState Room::getState() const
{
    return state_;
}

const std::vector<RoomId>& Room::getConnections() const
{
    // 地图界面可以读取这些连接来显示可选方向，但不能在外部改写拓扑结构
    return connections_;
}

RoomId Room::getUnlockPrerequisite() const
{
    return unlockAfter_;
}

bool Room::isConnectedTo(RoomId target) const
{
    // 这里只回答“地图上有没有路”，不负责判断目标是否已经解锁
    // 这种拆分让 Game 可以明确区分“没有道路”和“道路存在但暂未开放”
    return std::find(connections_.begin(), connections_.end(), target) !=
           connections_.end();
}

bool Room::canEnter() const
{
    // CLEARED 仍然可进入非常重要：玩家探索完支线后需要沿原路返回主线
    return state_ != RoomState::LOCKED;
}

bool Room::hasBattle() const
{
    // Game 只对这四种类型创建战斗；服务房和宝箱房走各自独立流程
    switch (type_)
    {
    case RoomType::MAIN_BATTLE:
    case RoomType::BOSS:
    case RoomType::OPTIONAL_BATTLE:
    case RoomType::ELITE_BATTLE:
        return true;

    case RoomType::SHOP:
    case RoomType::BLACK_MARKET:
    case RoomType::CHEST:
    default:
        return false;
    }
}

bool Room::isRepeatable() const
{
    // 服务房每次进入都可以再次打开，所以始终保持 AVAILABLE
    // “可再次进入”与“可重复触发内容”不同：CLEARED 战斗房可以再次进入，
    // 但 Game 看到 CLEARED 后不得重新创建敌人或重新结算奖励
    return type_ == RoomType::SHOP || type_ == RoomType::BLACK_MARKET;
}

void Room::unlock()
{
    // 只在确实未解锁时改变状态，使重复解锁成为安全的无副作用操作
    // 尤其不能把 CLEARED 改回 AVAILABLE，否则一次性奖励可能被重复领取
    if (state_ == RoomState::LOCKED)
    {
        state_ = RoomState::AVAILABLE;
    }
}

void Room::markCleared()
{
    // 完成状态只能在一次性内容“当前可触发”时写入
    // 战斗失败时 Game 不调用本函数，房间自然保持 AVAILABLE；
    // 即使 Game 错误地对服务房调用，本层检查也能避免商店/黑市被关闭
    if (state_ == RoomState::AVAILABLE && !isRepeatable())
    {
        state_ = RoomState::CLEARED;
    }
}

// ============================= 默认地图数据 =============================

std::vector<Room> createDefaultRooms()
{
    std::vector<Room> rooms;
    rooms.reserve(12);

    // 这里是整个项目唯一的地图配置源。房间名称目前使用中性名称，
    // 敌人、剧情和奖励仍由对应模块维护，不写进 Room 以避免接口耦合

    // 主线骨架：MAIN_1 -> MAIN_2 -> ... -> MAIN_6
    // 每个房间同时写回程连接，所以完成后的房间仍能作为返回路线

    rooms.push_back(Room(
        RoomId::MAIN_1,
        "主线房 1 / 开始",
        RoomType::MAIN_BATTLE,
        std::vector<RoomId>{RoomId::MAIN_2},
        RoomId::NONE,
        RoomState::AVAILABLE));

    rooms.push_back(Room(
        RoomId::MAIN_2,
        "主线房 2",
        RoomType::MAIN_BATTLE,
        std::vector<RoomId>{RoomId::MAIN_1, RoomId::MAIN_3},
        RoomId::MAIN_1));

    rooms.push_back(Room(
        RoomId::MAIN_3,
        "主线房 3",
        RoomType::MAIN_BATTLE,
        std::vector<RoomId>{RoomId::MAIN_2, RoomId::MAIN_4, RoomId::SHOP},
        RoomId::MAIN_2));

    rooms.push_back(Room(
        RoomId::MAIN_4,
        "主线房 4",
        RoomType::MAIN_BATTLE,
        std::vector<RoomId>{
            RoomId::MAIN_3,
            RoomId::MAIN_5,
            RoomId::EXTRA_BATTLE},
        RoomId::MAIN_3));

    rooms.push_back(Room(
        RoomId::MAIN_5,
        "主线房 5",
        RoomType::MAIN_BATTLE,
        std::vector<RoomId>{
            RoomId::MAIN_4,
            RoomId::MAIN_6,
            RoomId::ADVANCED_CHEST,
            RoomId::BLACK_MARKET},
        RoomId::MAIN_4));

    rooms.push_back(Room(
        RoomId::MAIN_6,
        "主线房 6 / Boss",
        RoomType::BOSS,
        std::vector<RoomId>{RoomId::MAIN_5},
        RoomId::MAIN_5));

    // MAIN_3 支线：普通商店前移为中前期补给点，并且可以重复进入。
    rooms.push_back(Room(
        RoomId::SHOP,
        "商店",
        RoomType::SHOP,
        std::vector<RoomId>{RoomId::MAIN_3},
        RoomId::MAIN_3));

    // MAIN_4 支线：附加怪房完成后继续开放精英怪房和普通宝箱房。
    rooms.push_back(Room(
        RoomId::EXTRA_BATTLE,
        "附加怪房",
        RoomType::OPTIONAL_BATTLE,
        std::vector<RoomId>{
            RoomId::MAIN_4,
            RoomId::ELITE_BATTLE,
            RoomId::CHEST},
        RoomId::MAIN_4));

    rooms.push_back(Room(
        RoomId::ELITE_BATTLE,
        "精英怪房",
        RoomType::ELITE_BATTLE,
        std::vector<RoomId>{RoomId::EXTRA_BATTLE},
        RoomId::EXTRA_BATTLE));

    rooms.push_back(Room(
        RoomId::CHEST,
        "宝箱房",
        RoomType::CHEST,
        std::vector<RoomId>{RoomId::EXTRA_BATTLE},
        RoomId::EXTRA_BATTLE));

    // MAIN_5 支线：高级宝箱和黑市不影响 MAIN_6 的解锁，玩家可以完全跳过。
    rooms.push_back(Room(
        RoomId::ADVANCED_CHEST,
        "高级宝箱房",
        RoomType::CHEST,
        std::vector<RoomId>{RoomId::MAIN_5},
        RoomId::MAIN_5));

    rooms.push_back(Room(
        RoomId::BLACK_MARKET,
        "黑市",
        RoomType::BLACK_MARKET,
        std::vector<RoomId>{RoomId::MAIN_5},
        RoomId::MAIN_5));

    return rooms;
}

// =============================== 安全查找 ===============================

Room* findRoomById(std::vector<Room>& rooms, RoomId id)
{
    // 可写版本主要供 Game 在通关后执行 unlock() 或 markCleared()。
    for (std::vector<Room>::iterator it = rooms.begin(); it != rooms.end(); ++it)
    {
        if (it->getId() == id)
        {
            return &(*it);
        }
    }

    return 0;
}

const Room* findRoomById(const std::vector<Room>& rooms, RoomId id)
{
    // 只读版本供地图界面、状态展示和存档检查使用。
    for (std::vector<Room>::const_iterator it = rooms.begin(); it != rooms.end(); ++it)
    {
        if (it->getId() == id)
        {
            return &(*it);
        }
    }

    return 0;
}
