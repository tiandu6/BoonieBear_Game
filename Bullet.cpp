#include "Bullet.h"

// 绘制子弹：橙色边框+深红色填充的圆形
void Bullet::Draw() const
{
    setlinecolor(RGB(255, 155, 50));
    setfillcolor(RGB(200, 75, 10));
    fillcircle(pos.x, pos.y, RADIUS);
}