#pragma once
#include <iostream>
#include <vector>
#include <easyx.h>
#include "Animation.h"
#include "common.h"
#include "Bullet.h"
#include "Player.h"
using namespace std;

// 敌人虚基类
class Enemy
{
public:
    Enemy();
    virtual ~Enemy(); // 必须是虚析构函数，防止子类内存泄漏！

    virtual bool CheckBulletCollision(const Bullet& bullet);
    virtual bool CheckPlayerCollision(const Player& player);

    // 【修改】：加入 vector 引用，为了让 Boss 可以在移动时召唤小弟塞进列表
    virtual void Move(const Player& player, vector<Enemy*>& enemy_list);
    virtual void Draw(int delta);
    virtual bool Hurt(int damage, const POINT& source_pos);

    bool CheckAlive();
    void SetPosition(POINT p); // 暴露坐标设置接口

    bool is_boss = false; // 标记是否为 Boss

protected:
    // 将原来写死的 const 变成 protected 变量，让子类继承并修改
    int frame_width = 80;
    int frame_height = 80;
    int shadow_width = 65;
    double speed = 2.0;

    // 敌人通用的阴影偏移量，默认值为 15
    int shadow_offset_value = 15;
    int shadow_offset_y = 15;     // Y 轴向上偏移量（默认 15）

    IMAGE img_shadow;
    Animation* anim_left = nullptr;
    Animation* anim_right = nullptr;
    POINT pos = { 0,0 };
    bool facing_left = false;
    bool alive = true;

    int hp = 2;
    int max_hp = 2;
    DWORD last_hurt_time = 0;

    DWORD knockback_end_time = 0;
    double knockback_vx = 0;
    double knockback_vy = 0;
};