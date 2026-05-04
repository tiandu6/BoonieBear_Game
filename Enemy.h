#pragma once
#include <iostream>
#include <vector>
#include <easyx.h>
#include "Animation.h"
#include "common.h"
#include "Bullet.h"
#include "Player.h"
using namespace std;

// 敌方非玩家角色 (NPC) 实体基类：提供寻路、渲染、受击等通用行为框架
class Enemy
{
public:
    Enemy();
    virtual ~Enemy();

    // 碰撞检测系统：使用 const 修饰保证该方法只读不写
    virtual bool CheckBulletCollision(const Bullet& bullet) const;
    virtual bool CheckPlayerCollision(const Player& player) const;

    // AI 导航引擎：接收 Player 存活数组，实现动态测距与最短路径迫近算法
    virtual void Move(const vector<Player*>& players, vector<Enemy*>& enemy_list);

    // 渲染管线
    virtual void Draw(int delta);

    // 伤害计算模块：接收伤害值与伤害来源坐标（用于计算击退方向）
    virtual bool Hurt(int damage, const POINT& source_pos);

    // 生命周期查询
    bool CheckAlive() const; // 标记为 const

    // 空间变换
    void SetPosition(POINT p);
    POINT GetPosition() const;

    // 下发技能受击后的高亮闪烁材质表现时长
    void ApplySkillFlicker(int duration_ms);

    bool is_boss = false;           // 标识自身是否拥有 Boss 权重体系
    int collision_damage = 15;      // 对玩家造成的物理冲撞基础伤害

protected:
    int frame_width = 80;           // 实体逻辑碰撞盒宽度
    int frame_height = 80;          // 实体逻辑碰撞盒高度
    int shadow_width = 65;          // 投影渲染尺寸
    double speed = 1.2;             // 基础刚体移速

    int shadow_offset_value = 15;   // 投影 X 轴跟随偏移
    int shadow_offset_y = 15;       // 投影 Y 轴纵深偏移

    IMAGE* p_img_shadow = nullptr;  // 投影图层材质
    Animation* anim_left = nullptr; // 左向行走图集引擎
    Animation* anim_right = nullptr;// 右向行走图集引擎

    POINT pos = { 0,0 };            // 物理层渲染坐标（整型）
    double real_x = 0.0;            // 逻辑层高精坐标（浮点型，防止缓速截断误差）
    double real_y = 0.0;

    bool facing_left = false;       // 自身朝向流记录器
    bool alive = true;              // 生命周期锁

    int hp = 2;                     // 当前生命值
    int max_hp = 2;                 // 生命值上限
    DWORD last_hurt_time = 0;       // 受击无敌保护帧时间戳

    // 受击物理击退引擎变量
    DWORD knockback_end_time = 0;
    double knockback_vx = 0;
    double knockback_vy = 0;

    DWORD skill_flicker_end_time = 0; // 特殊技能（如落雷、震击）附加的异常状态表现时长
};