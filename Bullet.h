#pragma once
#include <iostream>
#include <easyx.h>
using namespace std;

// 高频弹射物实体数据模型定义
class Bullet
{
public:
    Bullet() = default;
    ~Bullet() = default;

    // 发起针对自身的渲染调用请求
    void Draw() const; //  标记为 const 确保渲染逻辑不会意外修改弹道数据

    POINT pos = { 0, 0 };   // 引擎渲染层使用的整型坐标锚点

    // 物理层运动学与生命周期状态机变量
    bool is_active = false; // 当前实例是否存活于对象池中
    double real_x = 0;      // 物理层绝对 X 坐标 (采用双精度浮点以规避斜向低速运动时的整型截断误差)
    double real_y = 0;      // 物理层绝对 Y 坐标 
    double vx = 0;          // X 轴刚体动量 (Velocity X)
    double vy = 0;          // Y 轴刚体动量 (Velocity Y)

private:
    const int RADIUS = 10;  // 用于 AABB 或球形包围盒的物理碰撞半径基准值
};