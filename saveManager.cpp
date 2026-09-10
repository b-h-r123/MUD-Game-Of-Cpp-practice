#include "saveManager.h"

#include <fstream>
#include <sstream>

#include "Player.h"
#include "Room.h"

bool SaveManager::save(const std::string& filename, const Player& player, const std::vector<Room>& rooms) const
{
	std::ofstream out(filename);
	if (!out.is_open())
	{
		return false;
	}

	out << player.serialize() << "\n";
	for (std::size_t i = 0; i < rooms.size(); ++i)
	{
		out << static_cast<int>(rooms[i].getState()) << ' ';
	}
	out << "\n";

	return out.good();
}

bool SaveManager::load(const std::string& filename, Player& player, std::vector<Room>& rooms) const
{
	std::ifstream in(filename);
	if (!in.is_open())
	{
		return false;
	}

	std::string playerLine;
	std::string roomLine;
	if (!std::getline(in, playerLine) || !std::getline(in, roomLine))
	{
		return false;
	}

	if (!player.deserialize(playerLine))
	{
		return false;
	}

	std::istringstream roomStream(roomLine);
	int stateValue = 0;
	std::size_t index = 0;
	while (roomStream >> stateValue && index < rooms.size())
	{
		if (stateValue < 0 || stateValue > 2)
		{
			return false;
		}
		rooms[index].setState(static_cast<RoomState>(stateValue));
		++index;
	}

	return true;
}
