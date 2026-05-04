#pragma once
#include <iostream>
#include <vector>
#include <easyx.h>
#include "Bullet.h"
#include "common.h"
using namespace std;

// 武器抽象基类 (采用策略模式，便于运行时动态替换攻击形态)
class Weapon
{
public:
    virtual ~Weapon() {}

    // 物理位置更新与图像渲染流的纯虚接口
    // （均需要获取宿主玩家的实时坐标、朝向和 Buff 状态快照进行运算反馈）
    virtual void Update(POINT player_pos, bool facing_left, int player_width, int player_height, bool is_skill_active) = 0;
    virtual void Draw(POINT player_pos, bool facing_left, int player_width, int player_height, bool is_skill_active) = 0;

    // 初始化清空或重启弹药库状态配置
    virtual void Reset() = 0;

    // 获取并暴露发射的物理实体包围盒集合，供外部碰撞系统索取使用
    vector<Bullet>& GetBullets()
    {
        return bullets;
    }

protected:
    // 持有当前激活发射或待命的子弹/法球实体结构集合
    vector<Bullet> bullets;
};

// ================= 各独立派生武器装具 =================

// 派生类：近战系岩石钝器组件（熊大专属）
class RockWeapon : public Weapon
{
public:
    RockWeapon();
    void Update(POINT player_pos, bool facing_left, int player_width, int player_height, bool is_skill_active) override;
    void Draw(POINT player_pos, bool facing_left, int player_width, int player_height, bool is_skill_active) override;
    void Reset() override;
};

// 派生类：投掷系粘性蜂蜜组件（熊二专属）
class HoneyWeapon : public Weapon
{
public:
    HoneyWeapon();
    void Update(POINT player_pos, bool facing_left, int player_width, int player_height, bool is_skill_active) override;
    void Draw(POINT player_pos, bool facing_left, int player_width, int player_height, bool is_skill_active) override;
    void Reset() override;
};

// 派生类：射击系高频猎枪组件（光头强专属）
class GunWeapon : public Weapon
{
public:
    GunWeapon();
    void Update(POINT player_pos, bool facing_left, int player_width, int player_height, bool is_skill_active) override;
    void Draw(POINT player_pos, bool facing_left, int player_width, int player_height, bool is_skill_active) override;
    void Reset() override;

private:
    DWORD last_fire_time = 0; // 【独立化】：保证每个光头强实例拥有独立的开火计时器，互不干扰
};