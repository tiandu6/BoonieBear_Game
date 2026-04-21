#include "Enemies.h"

// ================= 普通敌人 =================
NormalEnemy::NormalEnemy()
{
    loadimage(&img_shadow, _T("img/shadow_enemy.png"), shadow_width, 20, true);
    anim_left = new Animation(atlas_enemy_left, 45);
    anim_right = new Animation(atlas_enemy_right, 45);
    hp = max_hp = 3;
    speed = 2.0; // 正常移速
}

// ================= 机器伐木工 =================
MachineEnemy::MachineEnemy()
{
    // 同步设置底层逻辑中的碰撞盒与宽高
    frame_width = 120;
    frame_height = 120;
    shadow_width = 90;  // 变大了，影子也得跟着加宽一点
    
    loadimage(&img_shadow, _T("img/shadow_enemy.png"), shadow_width, 20, true);
    // 假设你有名为 Machine 的图片
    anim_left = new Animation(atlas_machine_left, 30); // 腿抡得冒烟
    anim_right = new Animation(atlas_machine_right, 30);
    hp = max_hp = 6;  // 精英血量
    speed = 3;      // 极快移速！玩家压力的主要来源
}

// ================= BOSS 李老板 =================
BossEnemy::BossEnemy()
{
    is_boss = true;     // 我是大 Boss！

    frame_width = 200;  // Boss 体型巨大
    frame_height = 200;
    shadow_width = 160;
    loadimage(&img_shadow, _T("img/shadow_enemy.png"), shadow_width, 30, true);
    anim_left = new Animation(atlas_boss_left, 80);
    anim_right = new Animation(atlas_boss_right, 80);

    hp = max_hp = 300; // 史诗级血量
    speed = 1.2;       // Boss 走得慢，但压迫感强
    last_summon_time = GetTickCount();

    // 将 X 轴左右偏移量从 45 减小到 25（解决偏移过大的问题）
    shadow_offset_value = 25;
    // 将 Y 轴向上偏移量从默认的 15 加大到 60（把影子往上拽，解决影子偏下的问题）
    shadow_offset_y = 60;
}

void BossEnemy::Move(const Player& player, vector<Enemy*>& enemy_list)
{
    // 1. 先执行父类的常规追踪逻辑
    Enemy::Move(player, enemy_list);

    // 2. Boss 专属技能：每隔 4 秒，强行召唤 2 个小弟！
    DWORD current_time = GetTickCount();
    if (current_time - last_summon_time > 4000)
    {
        last_summon_time = current_time;
        for (int i = 0; i < 2; i++)
        {
            Enemy* minion = new NormalEnemy(); // 多态构造
            // 小弟从老板脚边刷出来
            minion->SetPosition({ pos.x + (rand() % 100 - 50), pos.y + (rand() % 100 - 50) });
            enemy_list.push_back(minion);
        }
    }
}