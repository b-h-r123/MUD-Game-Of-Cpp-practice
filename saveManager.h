#pragma once

#include<iostream>
#include<fstream>
#include<string>
#include "item.h"
#include "Player.h"

class SaveManager
{
	void saveGame(const std::string& filename, const std::string& data)
	{
		std::ofstream outFile(filename);
		if (outFile.is_open())
		{
			outFile << data;
			outFile.close();
			std::cout << "游戏已保存到 " << filename << std::endl;
		}
		else
		{
			std::cerr << "无法打开文件 " << filename << " 进行保存。" << std::endl;
		}
	}


private:
	Player player;
	Item item;
};
