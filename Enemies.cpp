#include "Enemies.h"

// 普通级杂兵敌对实体实例化
NormalEnemy::NormalEnemy()
{
    loadimage(&img_shadow, _T("img/shadow_enemy.png"), shadow_width, 20, true);
    anim_left = new Animation(atlas_enemy_left, 45);
    anim_right = new Animation(atlas_enemy_right, 45);
    hp = max_hp = 3;
    speed = 1.2;
    collision_damage = 15;
}

// 精英级机械敌对实体实例化
MachineEnemy::MachineEnemy()
{
    // 同步刷新派生类的碰撞体包围盒边界
    frame_width = 120;
    frame_height = 120;
    shadow_width = 90;

    loadimage(&img_shadow, _T("img/shadow_enemy.png"), shadow_width, 20, true);
    anim_left = new Animation(atlas_machine_left, 30);
    anim_right = new Animation(atlas_machine_right, 30);
    hp = max_hp = 6;
    speed = 2.0;
    collision_damage = 30;
}

// 史诗级首领敌对实体实例化
BossEnemy::BossEnemy()
{
    is_boss = true;

    frame_width = 200;
    frame_height = 200;
    shadow_width = 160;
    loadimage(&img_shadow, _T("img/shadow_enemy.png"), shadow_width, 30, true);
    anim_left = new Animation(atlas_boss_left, 80);
    anim_right = new Animation(atlas_boss_right, 80);

    hp = max_hp = 300;
    speed = 0.8;
    last_summon_time = GetTickCount();

    // 校准大型实体模型特有的阴影映射坐标位移参数
    shadow_offset_value = 25;
    shadow_offset_y = 60;

    collision_damage = 150;
}

void BossEnemy::Move(const Player& player, vector<Enemy*>& enemy_list)
{
    // 调用继承基类的默认追踪算法逻辑
    Enemy::Move(player, enemy_list);

    // 注入 Boss 独有的定时生成衍生实体行为逻辑
    DWORD current_time = GetTickCount();
    if (current_time - last_summon_time > 4000)
    {
        last_summon_time = current_time;
        for (int i = 0; i < 2; i++)
        {
            Enemy* minion = new NormalEnemy();
            // 在 Boss 周围一定随机范围内分配衍生实体出生坐标
            minion->SetPosition({ pos.x + (rand() % 100 - 50), pos.y + (rand() % 100 - 50) });
            enemy_list.push_back(minion);
        }
    }
}