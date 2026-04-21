#include "Bullet.h"

// 绘制子弹特效
void Bullet::Draw() const
{
    // 1. 获取当前时间，用于制作所有动态特效的时间轴
    DWORD current_time = GetTickCount();

    // 2. 呼吸脉冲缩放 (Pulsing Effect)
    // 利用 sin 函数生成 -2 到 +2 之间的周期性波动，让子弹看起来在“呼吸”
    double pulse_offset = sin(current_time / 100.0) * 2.0;

    // 假设基础半径为 12（你可以根据实际碰撞大小调整这个基础值）
    int current_radius = 12 + (int)pulse_offset;

    // 为了防止重叠绘制时出现难看的边线，先取消线框
    setlinestyle(PS_SOLID, 1);

    // ================== 多层光晕渲染 ==================

    // 第一层：最外层的微弱光晕 (蜂蜜深橙色，范围最大)
    setlinecolor(RGB(200, 100, 0));
    setfillcolor(RGB(180, 80, 0));
    fillcircle(pos.x, pos.y, current_radius + 6);

    // 第二层：中层高亮光环 (明黄偏橙，主色调)
    setlinecolor(RGB(255, 180, 20));
    setfillcolor(RGB(240, 150, 10));
    fillcircle(pos.x, pos.y, current_radius + 2);

    // 第三层：内核高光 (纯白色，体现能量的高热感)
    setlinecolor(RGB(255, 255, 255));
    setfillcolor(RGB(255, 255, 255));
    fillcircle(pos.x, pos.y, current_radius - 4);


    // ================== 动态环绕粒子特效 ==================
    // 利用 cos 和 sin 让两个小粒子围绕子弹核心高速旋转！
    // 旋转速度通过 current_time / 50.0 控制

    // 粒子 1
    int p1_x = pos.x + (int)(cos(current_time / 50.0) * (current_radius + 10));
    int p1_y = pos.y + (int)(sin(current_time / 50.0) * (current_radius + 10));

    // 粒子 2 (跟粒子1相差 180度，即 PI，这里用 3.1415)
    int p2_x = pos.x + (int)(cos(current_time / 50.0 + 3.1415) * (current_radius + 10));
    int p2_y = pos.y + (int)(sin(current_time / 50.0 + 3.1415) * (current_radius + 10));

    // 画出这两个金色的伴星小粒子
    setlinecolor(RGB(255, 255, 100));
    setfillcolor(RGB(255, 255, 100));
    fillcircle(p1_x, p1_y, 3);
    fillcircle(p2_x, p2_y, 3);
}