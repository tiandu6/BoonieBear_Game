#pragma once
#include <iostream>
#include <vector>
#include <easyx.h>
#include "Bullet.h"
#include "common.h"
using namespace std;

// 武器基类接口定义 (策略模式)
class Weapon
{
public:
    virtual ~Weapon()
    {
    }

    // 更新与渲染调用链均需要获取宿主玩家的状态快照
    virtual void Update(POINT player_pos, bool facing_left, int player_width, int player_height, bool is_skill_active) = 0;
    virtual void Draw(POINT player_pos, bool facing_left, int player_width, int player_height, bool is_skill_active) = 0;

    // 初始化清空或重启弹药库状态配置
    virtual void Reset() = 0;

    // 获取并暴露发射的物理实体包围盒集合
    vector<Bullet>& GetBullets()
    {
        return bullets;
    }

protected:
    // 持有当前激活发射或待命的弹道结构集合
    vector<Bullet> bullets;
};

// 派生类：近战系岩石钝器组件
class RockWeapon : public Weapon
{
public:
    RockWeapon();
    void Update(POINT player_pos, bool facing_left, int player_width, int player_height, bool is_skill_active) override;
    void Draw(POINT player_pos, bool facing_left, int player_width, int player_height, bool is_skill_active) override;
    void Reset() override;
};

// 派生类：投掷系粘性蜂蜜组件
class HoneyWeapon : public Weapon
{
public:
    HoneyWeapon();
    void Update(POINT player_pos, bool facing_left, int player_width, int player_height, bool is_skill_active) override;
    void Draw(POINT player_pos, bool facing_left, int player_width, int player_height, bool is_skill_active) override;
    void Reset() override;
};

// 派生类：射击系高频猎枪组件
class GunWeapon : public Weapon
{
public:
    GunWeapon();
    void Update(POINT player_pos, bool facing_left, int player_width, int player_height, bool is_skill_active) override;
    void Draw(POINT player_pos, bool facing_left, int player_width, int player_height, bool is_skill_active) override;
    void Reset() override;
};