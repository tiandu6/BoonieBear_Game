#include "Enemies.h"

// ======================== 【常规杂鱼模型】 ========================
NormalEnemy::NormalEnemy()
{
    // 加载标准体积投影
    p_img_shadow = &img_shadow_normal;

    // 装载对应图集资源 (动画切帧周期较短，视觉呈现动作较快)
    anim_left = new Animation(atlas_enemy_left, 45);
    anim_right = new Animation(atlas_enemy_right, 45);

    // 数值注入
    hp = max_hp = 3;
    speed = 1.2;
    collision_damage = 15;
}

// ======================== 【机械装甲模型】 ========================
MachineEnemy::MachineEnemy()
{
    // 重载默认碰撞骨骼框架为 120 像素规格
    frame_width = 120;
    frame_height = 120;
    shadow_width = 90;

    p_img_shadow = &img_shadow_machine;

    // 重装甲动作滞后，延长动画播放延迟
    anim_left = new Animation(atlas_machine_left, 30);
    anim_right = new Animation(atlas_machine_right, 30);

    // 数值注入：血量与冲撞伤害倍增，移速拔高带来极大压迫感
    hp = max_hp = 6;
    speed = 2.0;
    collision_damage = 30;
}

// ======================== 【黑化首领模型】 ========================
BossEnemy::BossEnemy()
{
    is_boss = true; // 开启特权标识

    // 覆盖为超巨型 200 像素规格判定盒
    frame_width = 200;
    frame_height = 200;
    shadow_width = 160;

    p_img_shadow = &img_shadow_boss;
    anim_left = new Animation(atlas_boss_left, 80);
    anim_right = new Animation(atlas_boss_right, 80);

    // 终极数值注入：极高生命墙与致死级冲撞力量，但步履维艰
    hp = max_hp = 300;
    speed = 0.8;
    last_summon_time = GetTickCount();

    // 补正模型放大后与脚部阴影的坐标对齐基准
    shadow_offset_value = 25;
    shadow_offset_y = 60;
    collision_damage = 150;
}

// 拓展基类位移算法，在走位追踪同时并发执行召唤兵团操作
void BossEnemy::Move(const vector<Player*>& players, vector<Enemy*>& enemy_list)
{
    Enemy::Move(players, enemy_list); // 继承父类索敌追踪位移计算

    DWORD current_time = GetTickCount();

    // 技能冷却器：每 4000 毫秒（4秒）召唤一波援军
    if (current_time - last_summon_time > 4000)
    {
        last_summon_time = current_time;

        // 瞬间凭空刷新两只小怪至自身周围 [-50, +50] 像素的离散随机区间
        for (int i = 0; i < 2; i++)
        {
            Enemy* minion = new NormalEnemy();
            minion->SetPosition({ pos.x + (rand() % 100 - 50), pos.y + (rand() % 100 - 50) });
            enemy_list.push_back(minion);
        }
    }
}