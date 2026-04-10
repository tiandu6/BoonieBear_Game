#pragma once
#include <iostream>
#include <easyx.h>
#include "Animation.h"
#include "common.h"
#include "Bullet.h"
#include "Player.h"
using namespace std;

// 敌人实体类
class Enemy
{
public:
    Enemy();
    ~Enemy();
    // 检测子弹与敌人的碰撞
    bool CheckBulletCollision(const Bullet& bullet);
    // 检测玩家与敌人的碰撞
    bool CheckPlayerCollision(const Player& player);
    // 敌人朝向玩家移动
    void Move(const Player& player);
    // 绘制敌人（含动画、阴影）
    void Draw(int delta);
    // 敌人受击处理
    void Hurt();
    // 判断敌人是否存活
    bool CheckAlive();

private:
    // 敌人帧尺寸、阴影宽度、移动速度
    const int FRAME_WIDTH = 80;
    const int FRAME_HEIGHT = 80;
    const int SHADOW_WIDTH = 65;
    const int SPEED = 2;

    IMAGE img_shadow;          // 敌人阴影图片
    Animation* anim_left;      // 向左的动画
    Animation* anim_right;     // 向右的动画
    POINT pos = { 0,0 };       // 敌人当前坐标
    bool facing_left = false;  // 敌人朝向：是否向左
    bool alive = true;         // 存活状态
};