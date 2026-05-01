#include "Weapon.h"
#include <math.h>

// 岩石武器实例组件
RockWeapon::RockWeapon()
{
    bullets = vector<Bullet>(2);
    Reset();
}

void RockWeapon::Update(POINT player_pos, bool facing_left, int player_width, int player_height, bool is_skill_active)
{
    // 利用圆极坐标算法进行轨道阵列周转控制推算
    double radius = 120;
    for (size_t i = 0; i < bullets.size(); i++)
    {
        double radian = GetTickCount() * 0.003 + (3.14159 * i);
        bullets[i].pos.x = player_pos.x + player_width / 2 + (int)(radius * cos(radian));
        bullets[i].pos.y = player_pos.y + player_height / 2 + (int)(radius * sin(radian));
    }
}

void RockWeapon::Draw(POINT player_pos, bool facing_left, int player_width, int player_height, bool is_skill_active)
{
    // 使用纯几何体渲染图元替换固定纹理资产
    for (const Bullet& b : bullets)
    {
        setlinecolor(RGB(80, 80, 80));
        setfillcolor(RGB(120, 120, 120));
        fillcircle(b.pos.x, b.pos.y, 20);

        setlinecolor(RGB(60, 60, 60));
        setfillcolor(RGB(90, 90, 90));
        fillcircle(b.pos.x - 6, b.pos.y + 6, 8);
        fillcircle(b.pos.x + 8, b.pos.y - 8, 6);
    }
}

void RockWeapon::Reset()
{
    for (size_t i = 0; i < bullets.size(); i++)
    {
        bullets[i].is_active = true;
    }
}


// 蜂蜜武器实例组件
HoneyWeapon::HoneyWeapon()
{
    bullets = vector<Bullet>(4);
    Reset();
}

void HoneyWeapon::Update(POINT player_pos, bool facing_left, int player_width, int player_height, bool is_skill_active)
{
    // 追加正弦波动参数模拟轨道的椭圆脉冲收缩
    double radius = 90 + 15 * sin(GetTickCount() * 0.005);
    for (size_t i = 0; i < bullets.size(); i++)
    {
        double radian = GetTickCount() * 0.006 + (1.57 * i);
        bullets[i].pos.x = player_pos.x + player_width / 2 + (int)(radius * cos(radian));
        bullets[i].pos.y = player_pos.y + player_height / 2 + (int)(radius * sin(radian));
    }
}

void HoneyWeapon::Draw(POINT player_pos, bool facing_left, int player_width, int player_height, bool is_skill_active)
{
    // 多边形拼合绘制模型本体
    for (const Bullet& b : bullets)
    {
        setlinecolor(RGB(200, 120, 0));
        setfillcolor(RGB(255, 180, 0));
        fillroundrect(b.pos.x - 12, b.pos.y - 8, b.pos.x + 12, b.pos.y + 12, 8, 8);

        setlinecolor(RGB(80, 40, 0));
        setfillcolor(RGB(100, 50, 10));
        fillroundrect(b.pos.x - 10, b.pos.y - 14, b.pos.x + 10, b.pos.y - 6, 3, 3);

        setlinecolor(RGB(200, 200, 200));
        setfillcolor(RGB(255, 255, 255));
        fillroundrect(b.pos.x - 6, b.pos.y - 2, b.pos.x + 6, b.pos.y + 6, 2, 2);
    }
}

void HoneyWeapon::Reset()
{
    for (size_t i = 0; i < bullets.size(); i++)
    {
        bullets[i].is_active = true;
    }
}


// 猎枪武器实例组件
GunWeapon::GunWeapon()
{
    bullets = vector<Bullet>(20);
    Reset();
}

void GunWeapon::Update(POINT player_pos, bool facing_left, int player_width, int player_height, bool is_skill_active)
{
    int fire_cd = 350;
    // 使用触发技能的状态属性应用增强发火间隔率计算
    if (is_skill_active)
    {
        fire_cd = 120;
    }

    static DWORD last_fire_time = 0;
    if (GetTickCount() - last_fire_time > fire_cd)
    {
        for (size_t i = 0; i < bullets.size(); i++)
        {
            if (!bullets[i].is_active)
            {
                bullets[i].is_active = true;

                // 校准枪管射口的生成点位参数
                bullets[i].real_x = player_pos.x + (facing_left ? 20 : player_width - 20);

                bullets[i].real_y = player_pos.y + player_height / 2 + 10;
                bullets[i].vx = facing_left ? -18.0 : 18.0;
                bullets[i].vy = 0;
                last_fire_time = GetTickCount();
                break;
            }
        }
    }

    // 更新发射列阵体系中的激活刚体的运动参数运算
    for (size_t i = 0; i < bullets.size(); i++)
    {
        if (bullets[i].is_active)
        {
            bullets[i].real_x += bullets[i].vx;
            bullets[i].real_y += bullets[i].vy;
            bullets[i].pos.x = (int)bullets[i].real_x;
            bullets[i].pos.y = (int)bullets[i].real_y;

            // 超出渲染视口的越界物自动触发回收管线
            if (bullets[i].pos.x < -100 || bullets[i].pos.x > WINDOW_WIDTH + 100)
            {
                bullets[i].is_active = false;
            }
        }
    }
}

void GunWeapon::Draw(POINT player_pos, bool facing_left, int player_width, int player_height, bool is_skill_active)
{
    for (size_t i = 0; i < bullets.size(); i++)
    {
        if (bullets[i].is_active)
        {
            // 通过定向判断分发调用的纹理内存区块
            if (bullets[i].vx < 0)
            {
                putimage_alpha(bullets[i].pos.x - 15, bullets[i].pos.y - 5, &img_bullet_left);
            }
            else
            {
                putimage_alpha(bullets[i].pos.x - 15, bullets[i].pos.y - 5, &img_bullet_right);
            }
        }
    }
}

void GunWeapon::Reset()
{
    for (size_t i = 0; i < bullets.size(); i++)
    {
        bullets[i].is_active = false;
    }
}