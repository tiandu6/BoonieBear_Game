#pragma once
#include "Player.h"

// 继承自 Player 的各独立游戏角色派生类声明

class XiongDa : public Player
{
public:
    XiongDa();
    void UpdateAttacks() override;
    void DrawAttacks() override;
    void UseSkill(vector<Enemy*>& enemy_list) override;
};

class XiongEr : public Player
{
public:
    XiongEr();
    void UpdateAttacks() override;
    void DrawAttacks() override;
    void UseSkill(vector<Enemy*>& enemy_list) override;
};

class GuangtouQiang : public Player
{
public:
    GuangtouQiang();
    void UpdateAttacks() override;
    void DrawAttacks() override;
    void UseSkill(vector<Enemy*>& enemy_list) override;
};