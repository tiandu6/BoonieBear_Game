#include "Enemy.h"

Enemy::Enemy()
{
    // 基类仅负责通用的随机出生点逻辑
    enum class SpawnEdge { Up = 0, Down, Left, Right };
    SpawnEdge edge = (SpawnEdge)(rand() % 4);
    switch (edge) {
    case SpawnEdge::Up: pos.x = rand() % WINDOW_WIDTH; pos.y = -150; break;
    case SpawnEdge::Down: pos.x = rand() % WINDOW_WIDTH; pos.y = WINDOW_HEIGHT; break;
    case SpawnEdge::Left: pos.x = -150; pos.y = rand() % WINDOW_HEIGHT; break;
    case SpawnEdge::Right: pos.x = WINDOW_WIDTH; pos.y = rand() % WINDOW_HEIGHT; break;
    }
}

Enemy::~Enemy()
{
    // 释放动画资源
    delete anim_left;
    delete anim_right;
}

// 检测子弹是否击中敌人
bool Enemy::CheckBulletCollision(const Bullet& bullet)
{
    // 判断子弹坐标是否在敌人帧范围内
    bool is_overlap_x = bullet.pos.x >= pos.x && bullet.pos.x <= pos.x + frame_width;
    bool is_overlap_y = bullet.pos.y >= pos.y && bullet.pos.y <= pos.y + frame_height;
    return is_overlap_x && is_overlap_y;
}

// 检测玩家与敌人的碰撞（带碰撞盒收缩）
bool Enemy::CheckPlayerCollision(const Player& player)
{
    // 碰撞盒收缩值，避免碰撞判定过宽
    int shrink = 15;

    // 计算敌人碰撞盒边界（收缩后）
    int enemy_left = pos.x + shrink;
    int enemy_right = pos.x + frame_width - shrink;
    int enemy_top = pos.y + shrink;
    int enemy_bottom = pos.y + frame_height - shrink;

    // 计算玩家碰撞盒边界（收缩后）
    const POINT& player_pos = player.GetPosition();
    int player_left = player_pos.x + shrink;
    int player_right = player_pos.x + player.FRAME_WIDTH - shrink;
    int player_top = player_pos.y + shrink;
    int player_bottom = player_pos.y + player.FRAME_HEIGHT - shrink;

    // 轴对齐碰撞检测：x、y轴均重叠则判定碰撞
    bool is_overlap_x = (enemy_left < player_right) && (enemy_right > player_left);
    bool is_overlap_y = (enemy_top < player_bottom) && (enemy_bottom > player_top);

    return is_overlap_x && is_overlap_y;
}

// 敌人朝向玩家移动的逻辑
void Enemy::Move(const Player& player, vector<Enemy*>& enemy_list)
{
    // 如果正处于击退状态，执行滑行逻辑，不进行常规AI追踪
    if (GetTickCount() < knockback_end_time)
    {
        pos.x += (int)knockback_vx;
        pos.y += (int)knockback_vy;

        // 防止被击退到屏幕外面去
        if (pos.x < 0) pos.x = 0;
        if (pos.x > WINDOW_WIDTH - frame_width) pos.x = WINDOW_WIDTH - frame_width;
        if (pos.y < 0) pos.y = 0;
        if (pos.y > WINDOW_HEIGHT - frame_height) pos.y = WINDOW_HEIGHT - frame_height;

        return; // 直接返回，不执行后面的追踪逻辑
    }

    // 计算敌人到玩家的方向向量
    const POINT& player_pos = player.GetPosition();
    int dir_x = player_pos.x - pos.x;
    int dir_y = player_pos.y - pos.y;

    // 向量归一化
    double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
    if (len_dir != 0)
    {
        double normalized_x = dir_x / len_dir;
        double normalized_y = dir_y / len_dir;
        pos.x += (int)(speed * normalized_x);
        pos.y += (int)(speed * normalized_y);
    }

    // 更新敌人朝向
    if (dir_x < 0) facing_left = true;
    else if (dir_x > 0) facing_left = false;
}

// 绘制敌人（含阴影、动画）
void Enemy::Draw(int delta)
{
    // 绘制阴影
    int pos_shadow_x = pos.x + (frame_width / 2 - shadow_width / 2);

    // 根据敌人朝向动态调整阴影X轴偏移
    if (facing_left) {
        pos_shadow_x += shadow_offset_value; // 面朝左冲刺，身体前倾，影子相对偏右
    }
    else {
        pos_shadow_x -= shadow_offset_value; // 面朝右冲刺，身体前倾，影子相对偏左
    }

    // 使用我们刚加的 shadow_offset_y 变量来计算高度，而不是写死的 15
    int pos_shadow_y = pos.y + frame_height - shadow_offset_y;
    putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);

    // 根据朝向播放对应动画
    if (facing_left)
        anim_left->Play(pos.x, pos.y, delta);
    else
        anim_right->Play(pos.x, pos.y, delta);

    // 区分 Boss 和普通敌人的血条表现
    if (is_boss)
    {
        // Boss 专属：超长血条 + 具体数字
        int bar_width = 140;
        int bar_height = 16;
        int bar_x = pos.x + (frame_width - bar_width) / 2;
        int bar_y = pos.y - 30; // 头顶偏上

        // 绘制带圆角的底槽
        setlinecolor(RGB(20, 20, 20));
        setfillcolor(RGB(60, 20, 20));
        fillroundrect(bar_x, bar_y, bar_x + bar_width, bar_y + bar_height, 4, 4);

        if (hp > 0) {
            int fill_width = (int)((double)hp / max_hp * bar_width);
            setfillcolor(RGB(255, 50, 50));
            solidroundrect(bar_x + 1, bar_y + 1, bar_x + fill_width - 1, bar_y + bar_height - 1, 4, 4);
        }

        // 绘制具体数字 (例如 300 / 300)
        TCHAR hp_txt[32];
        _stprintf_s(hp_txt, _T("%d / %d"), hp, max_hp);
        settextstyle(16, 0, _T("Arial"), 0, 0, FW_BOLD, false, false, false);
        setbkmode(TRANSPARENT);
        int tw = textwidth(hp_txt);
        int th = textheight(hp_txt);
        int tx = bar_x + (bar_width - tw) / 2;
        int ty = bar_y + (bar_height - th) / 2;

        settextcolor(RGB(20, 20, 20)); // 阴影
        outtextxy(tx + 1, ty + 1, hp_txt);
        settextcolor(RGB(255, 255, 255)); // 白字
        outtextxy(tx, ty, hp_txt);
    }
    else
    {
        // 杂兵的小血条 (保留你原有的朴素血条代码)
        int bar_width = 40;
        int bar_height = 5;
        int bar_x = pos.x + (frame_width - bar_width) / 2;
        int bar_y = pos.y - 10;
        setlinecolor(RGB(20, 20, 20));
        setfillcolor(RGB(80, 20, 20));
        fillrectangle(bar_x, bar_y, bar_x + bar_width, bar_y + bar_height);
        if (hp > 0) {
            int fill_width = (int)((double)hp / max_hp * bar_width);
            setfillcolor(RGB(255, 60, 60));
            solidrectangle(bar_x + 1, bar_y + 1, bar_x + fill_width - 1, bar_y + bar_height - 1);
        }
    }

    DWORD current_time = GetTickCount();
    if (current_time - last_hurt_time < 150)
    {
        // 进度值 0.0 ~ 1.0，用于计算冲击波扩大的圈数
        double progress = (current_time - last_hurt_time) / 150.0;
        int center_x = pos.x + frame_width / 2;
        int center_y = pos.y + frame_height / 2;

        // 1. 敌人本体受击发红蒙版 (半径随体型自适应)
        setlinecolor(RGB(255, 50, 50));
        setfillcolor(RGB(200, 50, 50));
        fillcircle(center_x, center_y, frame_width / 3); // 【修改】自适应半径

        // 2. 绘制向外扩散的刀光/冲击波特效 (扩散范围随体型自适应)
        setlinecolor(RGB(255, 150, 50));
        setlinestyle(PS_SOLID, 3);
        circle(center_x, center_y, (int)(progress * frame_width * 0.7)); // 【修改】自适应冲击波
        setlinestyle(PS_SOLID, 1);
    }
}

// 敌人受击
bool Enemy::Hurt(int damage, const POINT& source_pos)
{
    // 给敌人0.3秒的受击冷却
    DWORD current_time = GetTickCount();
    if (current_time - last_hurt_time > 300)
    {
        hp -= damage;
        last_hurt_time = current_time;

        // 计算击退方向（从子弹中心指向敌人中心）
        int center_x = pos.x + frame_width / 2;
        int center_y = pos.y + frame_height / 2;
        double dir_x = center_x - source_pos.x;
        double dir_y = center_y - source_pos.y;

        // 向量归一化
        double len = sqrt(dir_x * dir_x + dir_y * dir_y);
        if (len != 0)
        {
            // 击退速度设定为正常移速的 4 倍
            knockback_vx = (dir_x / len) * (speed * 4.0);
            knockback_vy = (dir_y / len) * (speed * 4.0);
        }

        // 击退效果持续 150 毫秒（这期间敌人不受自己控制，往后滑行）
        knockback_end_time = current_time + 150;

        if (hp <= 0) alive = false;
        return true;
    }
    return false;
}

// 返回敌人存活状态
bool Enemy::CheckAlive()
{
    return alive;
}

void Enemy::SetPosition(POINT p) 
{ 
    pos = p; 
}