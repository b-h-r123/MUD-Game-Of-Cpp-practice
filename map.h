#pragma once
#include <string>

struct Map
{
    std::string name, desc, tag;
    int north, south, west, east, mapX, mapY;
};

void initRooms(Map rooms[]);
std::string buildFrame(const Map rooms[], int p);
void move();


