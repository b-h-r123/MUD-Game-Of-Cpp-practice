//TODO: 实现Game类的成员函数

#include <iostream>
#include<string>
#include<vector>
#include<Windows.h>
#include<cstdlib>
#include<conio.h>

#include "Game.h"

static void specialPrint(const std::string& str, int delay = 50)
{
	for (char ch : str)
	{
		std::cout << ch << std::flush;
		Sleep(delay);
	}
}

Game::Game() {};

void Game::start()
{
	//房间初始化
	while (gaming)
	{
		mainMenu();
	}
	std::cout << "\n信息终端已关闭，再见，潜入员。" << std::endl;
}

void Game::mainMenu()
{
	const char* option[] = { "开始新游戏", "加载旧游戏", "退出游戏" };
	int select = 0;

	while (true)
	{
		system("cls");
		std::cout << "霓虹回响\n\n";
		for (int i = 0; i < 3; i++)
		{
			if (i == select)
				std::cout << "-> ";
			else
				std::cout << "   ";
			std::cout << option[i] << std::endl;
		}
		
		int key = _getch();
		if (key == 224 || key == 0)
			key = _getch();		//第二次读取

		if (key == 72)		//上键
			select = (select + 2) % 3;
		else if (key == 80)		//下键
			select = (select + 1) % 3;
		else if (key == 13)
		{
			switch (select)
			{
			case 0: 
				startNewGame();
				break;
			case 1: 
				loadingOldGame();
				break;
			case 2:
				gaming = 0;
				break;
			}
			break;
		}
	}
}

void Game::startNewGame()
{
	specialPrint("开启新游戏将会导致之前的存档数据丢失，是否继续？\n");
	int YN = 0;	// 0=是的 1=否
	while (true)
	{
		system("cls");
		std::cout << "\n开启新游戏将会导致之前的存档数据丢失，是否继续？\n";
		if (YN == 0)
		{
			std::cout << "-> 是的\n";
			std::cout << "   否\n";
		}
		else
		{
			std::cout << "   是的\n";
			std::cout << "-> 否\n";
		}
		int key = _getch();
		if(key == 224 || key == 0)
			key = _getch();		//第二次读取
		if(key == 72 || key == 80)		//上下键
			YN = (YN + 1) % 2;
		else if(key == 13)
		{
			if(YN == 0)
			{
				std::cout << "开始新游戏...\n";
				break;
			}
			else
			{
				std::cout << "\n正在返回主菜单...\n";
				Sleep(1500);
				return;
			}
		}

	}
	//CXZ来写过场动画
	chooseRoom();
}

void Game::chooseRoom()
{
	//正在思考怎么画房间选择界面
}

void Game::loadingOldGame()
{

	std::cout << "LodingOldGame";
}
