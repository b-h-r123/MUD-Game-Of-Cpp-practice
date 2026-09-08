#include "Player.h"

#include <iostream>

Player::Player()
	: BattleSystem(100, 100, 20, 5, 5), Exp(0), Gold(100), Atp(0) {}

void Player::setPlayerName(const std::string& newName)
{
	name = newName;
}

const std::string& Player::getPlayerName() const
{
	return name;
}

unsigned int Player::getLevel() const
{
	return level;
}

int Player::getExp() const
{
	return Exp;
}

int Player::getGold() const
{
	return Gold;
}

int Player::getAtp() const
{
	return Atp;
}

void Player::addExp(int val)
{
	Exp += val;
	checkLevelUp();
}

void Player::addGold(int val)
{
	Gold += val;
}

void Player::addAtp(int val)
{
	Atp += val;
}

void Player::spendAtp(int val)
{
	Atp -= val;
	if (Atp < 0) Atp = 0;
}

void Player::checkLevelUp()
{
	int expThreshold = static_cast<int>(level) * 100; // 升级所需经验值
	if (Exp >= expThreshold)
	{
		Exp -= expThreshold;
		level++;
		std::cout << "恭喜！你已升级到等级 " << level << "！\n";

		Atp += 1; // 每次升级增加 1 点属性点
		std::cout << "你获得了1点属性点！当前属性点: " << Atp << "\n";
		std::cout << "请选择如何分配属性点\n";
		std::cout << "1. 攻击力+5\n";
		std::cout << "2. 最大生命值+20\n";

		int choice = 0;
		std::cin >> choice;
		if (std::cin.fail())
		{
			std::cin.clear();
			std::cin.ignore(1024, '\n');
			choice = 0;
		}
		switch (choice)
		{
		case 1:
			// b_atk 等成员在 battleSystem 中是 protected，子类可直接访问。
			b_atk += 5;
			spendAtp(1);
			break;
		case 2:
			b_Hp += 20;
			b_MHp += 20;
			spendAtp(1);
			break;
		default:
			std::cout << "无效选择，属性点保留。\n";
			break;
		}

		// 升级后经验可能仍够再升一级，继续检测。
		if (Exp >= static_cast<int>(level) * 100)
		{
			checkLevelUp();
		}
	}
}

void Player::restoreToFull()
{
	b_Hp = b_MHp;
	b_Energy = b_Menergy;
}
