#include "Bullet.h"
#include <cmath>

// 执行单体发光弹射物的复合层级渲染及衍生粒子运算
void Bullet::Draw() const
{
    // 获取系统的毫秒时间戳作为周期波动的核心基准变量
    DWORD current_time = GetTickCount();

    // 构建呼吸缩放态：通过正弦函数剥离出 [-2.0, 2.0] 区间的平滑波动振幅
    double pulse_offset = sin(current_time / 100.0) * 2.0;
    // 注入基础半径，生成本帧的动态渲染尺寸
    int current_radius = 12 + (int)pulse_offset;

    // 清除图形边框线，防止透明遮罩叠加时引发暗色边缘伪影
    setlinestyle(PS_SOLID, 1);

    // ================= 分层泛光渲染管线 =================
    // 底层：范围最广的虚化泛光晕圈
    setlinecolor(RGB(200, 100, 0));
    setfillcolor(RGB(180, 80, 0));
    fillcircle(pos.x, pos.y, current_radius + 6);

    // 中层：色彩渐变的次级光环过渡
    setlinecolor(RGB(255, 180, 20));
    setfillcolor(RGB(240, 150, 10));
    fillcircle(pos.x, pos.y, current_radius + 2);

    // 表层：发白的核心高光聚焦点
    setlinecolor(RGB(255, 255, 255));
    setfillcolor(RGB(255, 255, 255));
    fillcircle(pos.x, pos.y, current_radius - 4);


    // ================= 附着卫星粒子渲染管线 =================
    // 基于引力场公式模拟弹射物周边的能量离子绕转效果

    // 粒子单元 1：提取 0 弧度相位的轨迹分量
    int p1_x = pos.x + (int)(cos(current_time / 50.0) * (current_radius + 10));
    int p1_y = pos.y + (int)(sin(current_time / 50.0) * (current_radius + 10));

    // 粒子单元 2：提取 PI (3.1415) 对称弧度相位的轨迹分量，实现双子星对偶旋转
    int p2_x = pos.x + (int)(cos(current_time / 50.0 + 3.1415) * (current_radius + 10));
    int p2_y = pos.y + (int)(sin(current_time / 50.0 + 3.1415) * (current_radius + 10));

    // 压入黄色高光渲染双子星
    setlinecolor(RGB(255, 255, 100));
    setfillcolor(RGB(255, 255, 100));
    fillcircle(p1_x, p1_y, 3);
    fillcircle(p2_x, p2_y, 3);
}