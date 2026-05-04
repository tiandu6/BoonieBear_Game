#include "Weapon.h"
#include <math.h>

// ======================== 【熊大：岩石武器实例组件】 ========================
RockWeapon::RockWeapon()
{
    bullets = vector<Bullet>(2); // 标配两颗护体环绕岩石
    Reset();
}

void RockWeapon::Update(POINT player_pos, bool facing_left, int player_width, int player_height, bool is_skill_active)
{
    // 利用圆的极坐标参数方程 (x = r*cosθ, y = r*sinθ) 求解环绕阵列轨道
    double radius = 120; // 设定环绕半径

    for (size_t i = 0; i < bullets.size(); i++)
    {
        // GetTickCount() 充当无穷递增的极角，制造自旋推移
        // 3.14159 * i 让两颗岩石相位刚好相差 180 度，实现对位平衡
        double radian = GetTickCount() * 0.003 + (3.14159 * i);
        bullets[i].pos.x = player_pos.x + player_width / 2 + (int)(radius * cos(radian));
        bullets[i].pos.y = player_pos.y + player_height / 2 + (int)(radius * sin(radian));
    }
}

void RockWeapon::Draw(POINT player_pos, bool facing_left, int player_width, int player_height, bool is_skill_active)
{
    // 使用纯几何体圆环渲染叠加图元，替换繁重的固定纹理资产
    for (const Bullet& b : bullets)
    {
        // 渲染岩石底漆大骨架
        setlinecolor(RGB(80, 80, 80));
        setfillcolor(RGB(120, 120, 120));
        fillcircle(b.pos.x, b.pos.y, 20);

        // 渲染表面随机的坑洼立体阴影
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
        bullets[i].is_active = true; // 环绕型近战武器默认长久存在
    }
}

// ======================== 【熊二：蜂蜜武器实例组件】 ========================
HoneyWeapon::HoneyWeapon()
{
    bullets = vector<Bullet>(4); // 四颗高密度的蜂蜜罐
    Reset();
}

void HoneyWeapon::Update(POINT player_pos, bool facing_left, int player_width, int player_height, bool is_skill_active)
{
    // 在极坐标半径中追加正弦波动参数 (sin)，模拟出犹如心脏跳动般的椭圆脉冲收缩轨道
    double radius = 90 + 15 * sin(GetTickCount() * 0.005);

    for (size_t i = 0; i < bullets.size(); i++)
    {
        // 1.57 (即 PI/2) 的相位差，让四个罐子呈 90 度十字形阵列分布
        double radian = GetTickCount() * 0.006 + (1.57 * i);
        bullets[i].pos.x = player_pos.x + player_width / 2 + (int)(radius * cos(radian));
        bullets[i].pos.y = player_pos.y + player_height / 2 + (int)(radius * sin(radian));
    }
}

void HoneyWeapon::Draw(POINT player_pos, bool facing_left, int player_width, int player_height, bool is_skill_active)
{
    // 多边形拼合绘制蜂蜜罐模型本体
    for (const Bullet& b : bullets)
    {
        // 罐身基座
        setlinecolor(RGB(200, 120, 0));
        setfillcolor(RGB(255, 180, 0));
        fillroundrect(b.pos.x - 12, b.pos.y - 8, b.pos.x + 12, b.pos.y + 12, 8, 8);

        // 顶层深色封口木塞
        setlinecolor(RGB(80, 40, 0));
        setfillcolor(RGB(100, 50, 10));
        fillroundrect(b.pos.x - 10, b.pos.y - 14, b.pos.x + 10, b.pos.y - 6, 3, 3);

        // 罐体表面的玻璃白斑高光点
        setlinecolor(RGB(200, 200, 200));
        setfillcolor(RGB(255, 255, 255));
        fillroundrect(b.pos.x - 6, b.pos.y - 2, b.pos.x + 6, b.pos.y + 6, 2, 2);
    }
}

void HoneyWeapon::Reset()
{
    for (size_t i = 0; i < bullets.size(); i++)
    {
        bullets[i].is_active = true; // 环绕型近战武器默认长久存在
    }
}

// ======================== 【光头强：猎枪武器实例组件】 ========================
GunWeapon::GunWeapon()
{
    bullets = vector<Bullet>(20); // 给枪械建立含有 20 发弹药余量的对象复用池
    Reset();
}

void GunWeapon::Update(POINT player_pos, bool facing_left, int player_width, int player_height, bool is_skill_active)
{
    int fire_cd = 350; // 默认枪械射击节拍 (350ms)

    // 接受大招属性状态：如果火力全开 Buff 存续，则暴力将冷却降低至 120ms
    if (is_skill_active)
    {
        fire_cd = 120;
    }

    // ================= 弹药发射管理与对象池获取 =================
    if (GetTickCount() - last_fire_time > (DWORD)fire_cd)
    {
        // 遍历弹药库，寻找一颗空闲的弹药实例执行借出分配
        for (size_t i = 0; i < bullets.size(); i++)
        {
            if (!bullets[i].is_active)
            {
                bullets[i].is_active = true; // 唤醒弹药

                // 依据人物朝向锁定膛口出弹坐标原点
                bullets[i].real_x = player_pos.x + (facing_left ? 20 : player_width - 20);
                bullets[i].real_y = player_pos.y + player_height / 2 + 10;

                // 赋予高速 X 轴初速度矢量
                bullets[i].vx = facing_left ? -18.0 : 18.0;
                bullets[i].vy = 0;

                last_fire_time = GetTickCount(); // 重置后坐力扳机计时
                break;
            }
        }
    }

    // ================= 弹药在途物理与边界销毁推进 =================
    for (size_t i = 0; i < bullets.size(); i++)
    {
        if (bullets[i].is_active)
        {
            // 给弹药实体追加物理动量运算
            bullets[i].real_x += bullets[i].vx;
            bullets[i].real_y += bullets[i].vy;
            bullets[i].pos.x = (int)bullets[i].real_x;
            bullets[i].pos.y = (int)bullets[i].real_y;

            // 越野垃圾回收 (Garbage Collection) 管线
            // 子弹一旦飞离视口超过 100 像素，直接封存沉睡归还给对象池
            if (bullets[i].pos.x < -100 || bullets[i].pos.x > WINDOW_WIDTH + 100)
            {
                bullets[i].is_active = false;
            }
        }
    }
}

void GunWeapon::Draw(POINT player_pos, bool facing_left, int player_width, int player_height, bool is_skill_active)
{
    // 渲染在途的所有出膛弹药实例
    for (size_t i = 0; i < bullets.size(); i++)
    {
        if (bullets[i].is_active)
        {
            // 通过识别动量物理分量的正负轴方向，精确分发对应的贴图材质以防倒飞伪影
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
    // 枪械弹药在触发器未叩响前均处于休眠沉睡态
    for (size_t i = 0; i < bullets.size(); i++)
    {
        bullets[i].is_active = false;
    }
}