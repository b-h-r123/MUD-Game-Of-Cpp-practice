#pragma once
#include<iostream>
#include<string>
#include"skill.h"

//随机数生成
int getRandomInt(int min, int max);


class BattleSystem
{
protected:
	int b_Hp;
	int b_MHp;
	int b_atk;
	int b_Energy;//蓝量
	int b_Menergy;//蓝量上限
	//debuff状态
	bool m_poisoned = false;
	int m_poisonTurn = 0;
	bool m_shocked = false;
	int m_shockTurn = 0;
	//敌人防御标记，策划要求防御：下次受伤伤害减半
	bool m_defending = false; 
public:
	//初步构造函数，初始化战斗系统的基本属性
	BattleSystem(int Hp, int MHp, int atk, int Energy, int Menergy);
	//获取实时属性
	int getHp() const;
	int getMHp() const;
	int getAtk() const;

	//战斗过程
	void battle(BattleSystem& player, BattleSystem& enemy);
	
	//技能攻击
	void useSkill(Skill& skill, BattleSystem* caster, BattleSystem* target);
	//受到伤害
	void takedamage(int atk);
	//回血
	void heal(int val);
	//判断是否结束战斗
	bool isBattleOver() const;

	//====Debuff接口====
	void applyPoison(int turn);   //施加中毒
	void applyShock(int turn);    //施加电击
	void applyDefend();           //敌人开启防御

	//【回合开始执行状态】中毒在这里扣血
	void processStatusStartTurn();

	//【计算输出伤害时调用】电击减伤逻辑，返回修正后的伤害
	int calcDamageOutput(int rawDmg);

	//受到伤害前调用：处理敌人防御减伤
	int calcDamageReceive(int rawDmg);

	//清除全部状态
	void clearAllStatus();

	virtual ~BattleSystem() = default;
};
