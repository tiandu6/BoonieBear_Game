#pragma once
#include <iostream>
#include <easyx.h>
#include "Animation.h"
#include "common.h"
using namespace std;

// 玩家实体类
class Player
{
public:
    // 玩家帧尺寸
    const int FRAME_WIDTH = 80;
    const int FRAME_HEIGHT = 80;

    Player();
    ~Player();
    // 处理玩家输入事件（键鼠）
    void ProcessEvent(const ExMessage& msg);
    // 玩家移动逻辑
    void Move();
    // 绘制玩家（含动画、阴影）
    void Draw(int delta);
    // 获取玩家当前坐标
    const POINT& GetPosition() const;

private:
    // 玩家阴影宽度、移动速度
    const int SHADOW_WIDTH = 80;
    const int SPEED = 2;

    IMAGE img_shadow;          // 玩家阴影图片
    Animation* anim_left;      // 向左的动画
    Animation* anim_right;     // 向右的动画
    POINT pos = { 500,500 };   // 玩家初始坐标

    // 玩家移动状态（上下左右）
    bool is_move_up = false;
    bool is_move_down = false;
    bool is_move_left = false;
    bool is_move_right = false;
};