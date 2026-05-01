#pragma once
#include <iostream>
#include <easyx.h>
using namespace std;

// 弹射物实体数据类
class Bullet
{
public:
    Bullet() = default;
    ~Bullet() = default;

    // 渲染弹射物数据模型
    void Draw() const;

    POINT pos = { 0, 0 };   // 整型坐标数据缓存

    // 运动学与状态属性
    bool is_active = false; // 当前生命周期激活状态
    double real_x = 0;      // 浮点型内部X坐标 (避免低速运动时的整型截断误差)
    double real_y = 0;      // 浮点型内部Y坐标 
    double vx = 0;          // X 轴分量速度
    double vy = 0;          // Y 轴分量速度

private:
    const int RADIUS = 10;  // 弹射物碰撞半径基准值
};