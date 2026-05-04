#pragma once
#include "Enemy.h"

// 敌方具体怪物类别聚合声明

// 基础型近战伐木工：中等属性，常规行为
class NormalEnemy : public Enemy
{
public:
    NormalEnemy();
};

// 进阶型机械载具怪：高血量高伤害，拥有更大的视觉碰撞体积
class MachineEnemy : public Enemy
{
public:
    MachineEnemy();
};

// 游戏最终首领：黑化光头强，具备超大体积以及极高抗击退质量，并附带召唤随从的特殊机制
class BossEnemy : public Enemy
{
public:
    BossEnemy();

    // 添加 override 关键字，指明覆盖重写基类的寻路算法
    void Move(const vector<Player*>& players, vector<Enemy*>& enemy_list) override;

private:
    DWORD last_summon_time = 0; // Boss 独占的随从招募时间锁
};