#pragma once
#include <string>
#include "battleSystem.h"

// Player 继承 BattleSystem，因此拥有血量/攻击/能量与全部 debuff 接口。
// Player 自身再维护玩家档案：名字、等级、经验、金币、属性点。
// 注意：构造时不读取键盘输入（避免在 Game 构造阶段就阻塞），
// 名字由 Game 在开局时通过 setPlayerName 显式写入。
class Player : public BattleSystem
{
public:
	Player();

	void setPlayerName(const std::string& newName);
	const std::string& getPlayerName() const;

	unsigned int getLevel() const;
	int getExp() const;
	int getGold() const;
	int getAtp() const;

	void addExp(int val);
	void addGold(int val);
	void addAtp(int val);
	void spendAtp(int val);
	void checkLevelUp();

	// 战间整备：把血量/能量回满（回城或章节结算时调用）。
	void restoreToFull();

	~Player() = default;

private:
	std::string name;
	unsigned int level = 1;
	int Exp = 0;      //经验
	int Gold = 0;     //金钱
	int Atp = 0;      //属性点（attribute point）
};
