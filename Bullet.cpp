#include "Bullet.h"
#include <cmath>

// 渲染子弹对象及关联的视觉特效
void Bullet::Draw() const
{
    // 获取当前系统时间作为动画波动计算的基准参考系
    DWORD current_time = GetTickCount();

    // 缩放波动效果计算
    // 通过正弦函数生成基于时间的循环周期波动参数 [-2.0, 2.0]
    double pulse_offset = sin(current_time / 100.0) * 2.0;

    // 结合基础半径计算当前渲染半径
    int current_radius = 12 + (int)pulse_offset;

    // 禁用边框线渲染，避免多层覆盖时出现轮廓伪影
    setlinestyle(PS_SOLID, 1);

    // 渲染分层光源效果

    // 图层 1：外部泛光层 (最大影响半径)
    setlinecolor(RGB(200, 100, 0));
    setfillcolor(RGB(180, 80, 0));
    fillcircle(pos.x, pos.y, current_radius + 6);

    // 图层 2：中层过渡光环 
    setlinecolor(RGB(255, 180, 20));
    setfillcolor(RGB(240, 150, 10));
    fillcircle(pos.x, pos.y, current_radius + 2);

    // 图层 3：内部核心高光层
    setlinecolor(RGB(255, 255, 255));
    setfillcolor(RGB(255, 255, 255));
    fillcircle(pos.x, pos.y, current_radius - 4);


    // 渲染环绕粒子附属特效
    // 根据系统时间差结合三角函数，实时计算粒子的运动轨迹坐标

    // 粒子单元 1 运动计算
    int p1_x = pos.x + (int)(cos(current_time / 50.0) * (current_radius + 10));
    int p1_y = pos.y + (int)(sin(current_time / 50.0) * (current_radius + 10));

    // 粒子单元 2 运动计算 (相位差设定为 180 度即 PI)
    int p2_x = pos.x + (int)(cos(current_time / 50.0 + 3.1415) * (current_radius + 10));
    int p2_y = pos.y + (int)(sin(current_time / 50.0 + 3.1415) * (current_radius + 10));

    // 执行粒子单元渲染
    setlinecolor(RGB(255, 255, 100));
    setfillcolor(RGB(255, 255, 100));
    fillcircle(p1_x, p1_y, 3);
    fillcircle(p2_x, p2_y, 3);
}