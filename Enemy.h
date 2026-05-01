#pragma once
#include <iostream>
#include <vector>
#include <easyx.h>
#include "Animation.h"
#include "common.h"
#include "Bullet.h"
#include "Player.h"
using namespace std;

// 通用敌对实体基类定义
class Enemy
{
public:
    Enemy();
    virtual ~Enemy();

    virtual bool CheckBulletCollision(const Bullet& bullet);
    virtual bool CheckPlayerCollision(const Player& player);

    // AI导航追随计算机制实现
    virtual void Move(const Player& player, vector<Enemy*>& enemy_list);
    virtual void Draw(int delta);
    virtual bool Hurt(int damage, const POINT& source_pos);

    bool CheckAlive();
    void SetPosition(POINT p);

    POINT GetPosition() const;

    // 应用受击控制渲染层视觉闪烁
    void ApplySkillFlicker(int duration_ms);

    bool is_boss = false;

    // 默认接触基础碰撞伤害阈值定义
    int collision_damage = 15;

protected:
    int frame_width = 80;
    int frame_height = 80;
    int shadow_width = 65;
    double speed = 1.2;

    int shadow_offset_value = 15;
    int shadow_offset_y = 15;

    IMAGE img_shadow;
    Animation* anim_left = nullptr;
    Animation* anim_right = nullptr;
    POINT pos = { 0,0 };
    double real_x = 0.0;
    double real_y = 0.0;

    bool facing_left = false;
    bool alive = true;

    int hp = 2;
    int max_hp = 2;
    DWORD last_hurt_time = 0;

    DWORD knockback_end_time = 0;
    double knockback_vx = 0;
    double knockback_vy = 0;

    DWORD skill_flicker_end_time = 0;
};