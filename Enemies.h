#pragma once
#include "Enemy.h"

// 各梯队敌对实体的派生类声明

// 普通敌对实体
class NormalEnemy : public Enemy
{
public:
    NormalEnemy();
};

// 强化精英敌对实体
class MachineEnemy : public Enemy
{
public:
    MachineEnemy();
};

// 首领级别敌对实体
class BossEnemy : public Enemy
{
public:
    BossEnemy();
    // 覆盖基类方法以实现独占的运动及召唤系统
    void Move(const Player& player, vector<Enemy*>& enemy_list) override;
private:
    DWORD last_summon_time = 0; // 控制衍生召唤的时间锁
};