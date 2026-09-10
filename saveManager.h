#pragma once

#include <string>
#include <vector>

class Player;
class Room;

// SaveManager 只负责把玩家状态与房间进度写入/读出文本存档，
// 不参与战斗、商店和输入。存档格式为两行：
//   第一行：Player::serialize() 的一行文本
//   第二行：12 个房间状态（0=LOCKED 1=AVAILABLE 2=CLEARED）
class SaveManager
{
public:
	bool save(const std::string& filename, const Player& player, const std::vector<Room>& rooms) const;
	bool load(const std::string& filename, Player& player, std::vector<Room>& rooms) const;
};
