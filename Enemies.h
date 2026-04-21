#pragma once
#include "Enemy.h"

// 1. 普通敌人（原伐木工）
class NormalEnemy : public Enemy
{
public:
    NormalEnemy();
};

// 2. 精英敌人：机器伐木工（高移速）
class MachineEnemy : public Enemy
{
public:
    MachineEnemy();
};

// 3. BOSS：李老板（高血量，定时召唤小弟）
class BossEnemy : public Enemy
{
public:
    BossEnemy();
    // 【考点】：利用虚函数重写（Override），实现 Boss 专属行为
    void Move(const Player& player, vector<Enemy*>& enemy_list) override;
private:
    DWORD last_summon_time = 0; // 上次召唤时间
};