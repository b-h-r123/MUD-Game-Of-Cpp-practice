#pragma once
#include <string>
#include <vector>
#include "Room.h"

struct Map
{
    std::string name, desc, tag;
    int north, south, west, east, mapX, mapY;
};

void initRooms(Map rooms[]);
std::string buildFrame(const Map rooms[], int p);
void move(std::vector<Room>& rooms);


