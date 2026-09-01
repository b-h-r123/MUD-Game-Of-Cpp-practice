#include "Room.h"

#include <cassert>
#include <iostream>
#include <set>
#include <vector>

namespace
{
void unlockRoomsAfter(std::vector<Room>& rooms, RoomId completedRoom)
{
    for (std::vector<Room>::iterator it = rooms.begin(); it != rooms.end(); ++it)
    {
        if (it->getUnlockPrerequisite() == completedRoom)
        {
            it->unlock();
        }
    }
}

void requireBidirectionalEdge(const std::vector<Room>& rooms, RoomId firstId, RoomId secondId)
{
    const Room* first = findRoomById(rooms, firstId);
    const Room* second = findRoomById(rooms, secondId);
    assert(first != 0 && second != 0);
    assert(first->isConnectedTo(secondId));
    assert(second->isConnectedTo(firstId));
}
} // namespace

int main()
{
    std::vector<Room> rooms = createDefaultRooms();
    assert(rooms.size() == 12U);

    // -------------------------- 整体结构 --------------------------
    std::set<RoomId> ids;
    std::size_t directedConnectionCount = 0U;
    std::size_t battleRoomCount = 0U;
    std::size_t repeatableRoomCount = 0U;

    for (std::vector<Room>::const_iterator it = rooms.begin(); it != rooms.end(); ++it)
    {
        assert(it->getId() != RoomId::NONE);
        assert(!it->getName().empty());
        assert(ids.insert(it->getId()).second);

        directedConnectionCount += it->getConnections().size();
        battleRoomCount += it->hasBattle() ? 1U : 0U;
        repeatableRoomCount += it->isRepeatable() ? 1U : 0U;

        for (std::vector<RoomId>::const_iterator neighbor = it->getConnections().begin();
             neighbor != it->getConnections().end();
             ++neighbor)
        {
            const Room* other = findRoomById(rooms, *neighbor);
            assert(other != 0);
            assert(*neighbor != it->getId());
            assert(other->isConnectedTo(it->getId()));
        }
    }

    assert(ids.size() == 12U);
    assert(directedConnectionCount == 22U); // 11 条双向边。
    assert(battleRoomCount == 8U);          // 6 个主线战斗 + 附加怪 + 精英怪。
    assert(repeatableRoomCount == 2U);      // 普通商店和黑市。

    // -------------------------- 已确认拓扑 --------------------------
    requireBidirectionalEdge(rooms, RoomId::MAIN_1, RoomId::MAIN_2);
    requireBidirectionalEdge(rooms, RoomId::MAIN_2, RoomId::MAIN_3);
    requireBidirectionalEdge(rooms, RoomId::MAIN_3, RoomId::MAIN_4);
    requireBidirectionalEdge(rooms, RoomId::MAIN_4, RoomId::MAIN_5);
    requireBidirectionalEdge(rooms, RoomId::MAIN_5, RoomId::MAIN_6);

    // 普通商店现在连接主线房 3，用于把第一次补给/刷装备节点前移。
    requireBidirectionalEdge(rooms, RoomId::MAIN_3, RoomId::SHOP);
    assert(!findRoomById(rooms, RoomId::MAIN_4)->isConnectedTo(RoomId::SHOP));

    requireBidirectionalEdge(rooms, RoomId::MAIN_4, RoomId::EXTRA_BATTLE);
    requireBidirectionalEdge(rooms, RoomId::EXTRA_BATTLE, RoomId::ELITE_BATTLE);
    requireBidirectionalEdge(rooms, RoomId::EXTRA_BATTLE, RoomId::CHEST);
    requireBidirectionalEdge(rooms, RoomId::MAIN_5, RoomId::ADVANCED_CHEST);
    requireBidirectionalEdge(rooms, RoomId::MAIN_5, RoomId::BLACK_MARKET);

    // -------------------------- 解锁节奏 --------------------------
    Room* main1 = findRoomById(rooms, RoomId::MAIN_1);
    Room* main2 = findRoomById(rooms, RoomId::MAIN_2);
    Room* main3 = findRoomById(rooms, RoomId::MAIN_3);
    Room* main4 = findRoomById(rooms, RoomId::MAIN_4);
    Room* main5 = findRoomById(rooms, RoomId::MAIN_5);
    Room* shop = findRoomById(rooms, RoomId::SHOP);
    Room* extraBattle = findRoomById(rooms, RoomId::EXTRA_BATTLE);

    assert(main1 != 0 && main2 != 0 && main3 != 0 && main4 != 0 &&
           main5 != 0 && shop != 0 && extraBattle != 0);
    assert(main1->canEnter());
    assert(!shop->canEnter());

    main1->markCleared();
    unlockRoomsAfter(rooms, RoomId::MAIN_1);
    main2->markCleared();
    unlockRoomsAfter(rooms, RoomId::MAIN_2);

    // 到达主线房 3 但尚未通关时，商店仍然不可进入。
    assert(main3->canEnter());
    assert(!shop->canEnter());

    main3->markCleared();
    unlockRoomsAfter(rooms, RoomId::MAIN_3);
    assert(main4->canEnter());
    assert(shop->canEnter());
    assert(shop->getState() == RoomState::AVAILABLE);
    assert(!extraBattle->canEnter());

    // 商店是可重复服务，调用 markCleared() 也不应变成 CLEARED。
    shop->markCleared();
    assert(shop->getState() == RoomState::AVAILABLE);

    main4->markCleared();
    unlockRoomsAfter(rooms, RoomId::MAIN_4);
    assert(main5->canEnter());
    assert(extraBattle->canEnter());

    std::cout << "Room tests passed.\n";
    return 0;
}
