//TODO：实现函数
#include<iostream>
#include<string>
#include"Player.h"


void Player::inputPlayername() 
{ 
	std::cout << "请输入玩家姓名: ";
	std::getline(std::cin, name);
}

//检验升级
void Player::checkLevelUp() 
{
	int expThreshold = level * 100; //升级所需经验值
	if (Exp >= expThreshold) 
	{
		Exp -= expThreshold; //扣除升级所需经验
		level++; //升级
		std::cout << "恭喜！你已升级到等级 " << level << "！\n";
		//升级后可以增加属性点或其他奖励
		Atp += 1; //每次升级增加1点属性点
		std::cout << "你获得了1点属性点！当前属性点: " << Atp << "\n";
		std::cout << "请选择如何分配属性点\n";
		int choice=0;
		std::cout << "1. 攻击力+5\n";
		std::cout << "2. 最大生命值+20\n";
		std::cin >> choice;
		switch (choice)
		{
		case 1:
			b_atk += 5;
			Atp--;
			break;
		case 2:
			b_Hp += 20;
			b_MHp += 20;
			Atp--;
			break;
		default:
			break;
		}

	}
}

void Player::addExp(int val)
{
	Exp += val;
	checkLevelUp(); //加经验顺便触发升级检测，符合游戏逻辑
}

void Player::addGold(int val)
{
	Gold += val;
}

Player::Player() :BattleSystem(100, 100, 20,5,5), Exp(0), Gold(100), Atp(0) 
{
	inputPlayername();
};