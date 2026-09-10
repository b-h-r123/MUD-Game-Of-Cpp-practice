#pragma once
#include "skill.h"

// 返回 [min, max] 闭区间的随机整数。
int getRandomInt(int min, int max);

// BattleSystem 是战斗属性的载体，也是 Player / Enemy 的公共基类。
// 它持有血量、攻击、能量与各种 debuff 状态，并提供一套回合制战斗流程。
// 战斗的具体参与者通过继承获得这些属性和接口，battle() 在内部
// 用 dynamic_cast 区分我方（Player）与敌方（Enemy）。
class BattleSystem
{
protected:
	int b_Hp;
	int b_MHp;
	int b_atk;
	int b_Energy;   //当前能量
	int b_Menergy;  //能量上限
	// debuff 状态
	bool m_poisoned = false;
	int m_poisonTurn = 0;
	bool m_shocked = false;
	int m_shockTurn = 0;
	// 防御标记：下一次受伤伤害减半
	bool m_defending = false;
public:
	BattleSystem(int Hp, int MHp, int atk, int Energy, int Menergy);
	// 获取实时属性。虚函数：Player 会叠加装备加成后返回有效值。
	virtual int getHp() const;
	virtual int getMHp() const;
	virtual int getAtk() const;
	virtual int getEnergy() const;
	virtual int getMEnergy() const;

	// 回合制战斗主流程。player/enemy 需分别为 Player / Enemy 的实例。
	// 玩家胜利返回 true，玩家失败返回 false。
	bool battle(BattleSystem& player, BattleSystem& enemy);

	// 技能攻击
	void useSkill(Skill& skill, BattleSystem* caster, BattleSystem* target);
	// 受到伤害
	void takedamage(int atk);
	// 回血（不超过上限）
	void heal(int val);
	// 判断是否结束战斗（HP<=0）
	bool isBattleOver() const;

	// ==== Debuff 接口 ====
	void applyPoison(int turn);   //施加中毒
	void applyShock(int turn);    //施加电击
	void applyDefend();           //开启防御

	// 回合开始执行状态结算（中毒扣血等）
	void processStatusStartTurn();

	// 计算【输出】伤害时调用：电击减伤逻辑，返回修正后伤害
	virtual int calcDamageOutput(int rawDmg);

	// 受到伤害前调用：处理防御减伤。虚函数供 Player 叠加装备减伤。
	virtual int calcDamageReceive(int rawDmg);

	// 清除全部战斗状态
	void clearAllStatus();

	virtual ~BattleSystem() = default;
};
