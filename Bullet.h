#pragma once
#include <iostream>
#include <easyx.h>
using namespace std;

// 子弹类
class Bullet
{
public:
    Bullet() = default;
    ~Bullet() = default;
    // 绘制子弹（圆形）
    void Draw() const;

    POINT pos = { 0,0 };  // 子弹当前坐标
private:
    const int RADIUS = 10;// 子弹半径
};