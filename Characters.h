#pragma once
#include "Player.h"

// 聚合角色派生类群，继承基类通用状态流

// 角色：熊大（近战控制系重装坦克）
class XiongDa : public Player
{
public:
    XiongDa();
    void UpdateAttacks() override;
    void DrawAttacks() override;
    void UseSkill(vector<Enemy*>& enemy_list) override; // 重载包含击退判定的群控逻辑
};

// 角色：熊二（高频位移系战地医疗）
class XiongEr : public Player
{
public:
    XiongEr();
    void UpdateAttacks() override;
    void DrawAttacks() override;
    void UseSkill(vector<Enemy*>& enemy_list) override; // 重载基于已损百分比计算的急救逻辑
};

// 角色：光头强（高输出脆皮狂战士）
class GuangtouQiang : public Player
{
public:
    GuangtouQiang();
    void UpdateAttacks() override; // 植入持续性状态监视器的重载版本
    void DrawAttacks() override;
    void UseSkill(vector<Enemy*>& enemy_list) override; // 重载通过时间戳进行自身面板强化的逻辑
};