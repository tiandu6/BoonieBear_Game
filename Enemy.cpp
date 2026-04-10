#include "Enemy.h"

Enemy::Enemy()
{
    // 加载敌人阴影图片，指定宽高并启用透明
    loadimage(&img_shadow, _T("img/shadow_enemy.png"), SHADOW_WIDTH, 20, true);
    // 创建左右方向动画，帧间隔45ms
    anim_left = new Animation(atlas_enemy_left, 45);
    anim_right = new Animation(atlas_enemy_right, 45);

    // 敌人出生的边缘枚举
    enum class SpawnEdge
    {
        Up = 0,
        Down,
        Left,
        Right
    };

    // 随机选择出生的屏幕边缘
    SpawnEdge edge = (SpawnEdge)(rand() % 4);
    switch (edge)
    {
    case SpawnEdge::Up:
        pos.x = rand() % WINDOW_WIDTH;
        pos.y = -FRAME_HEIGHT;
        break;
    case SpawnEdge::Down:
        pos.x = rand() % WINDOW_WIDTH;
        pos.y = WINDOW_HEIGHT;
        break;
    case SpawnEdge::Left:
        pos.x = -FRAME_WIDTH;
        pos.y = rand() % WINDOW_HEIGHT;
        break;
    case SpawnEdge::Right:
        pos.x = WINDOW_WIDTH;
        pos.y = rand() % WINDOW_HEIGHT;
        break;
    default:
        break;
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
    bool is_overlap_x = bullet.pos.x >= pos.x && bullet.pos.x <= pos.x + FRAME_WIDTH;
    bool is_overlap_y = bullet.pos.y >= pos.y && bullet.pos.y <= pos.y + FRAME_HEIGHT;
    return is_overlap_x && is_overlap_y;
}

// 检测玩家与敌人的碰撞（带碰撞盒收缩）
bool Enemy::CheckPlayerCollision(const Player& player)
{
    // 碰撞盒收缩值，避免碰撞判定过宽
    int shrink = 15;

    // 计算敌人碰撞盒边界（收缩后）
    int enemy_left = pos.x + shrink;
    int enemy_right = pos.x + FRAME_WIDTH - shrink;
    int enemy_top = pos.y + shrink;
    int enemy_bottom = pos.y + FRAME_HEIGHT - shrink;

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
void Enemy::Move(const Player& player)
{
    // 计算敌人到玩家的方向向量
    const POINT& player_pos = player.GetPosition();
    int dir_x = player_pos.x - pos.x;
    int dir_y = player_pos.y - pos.y;

    // 向量归一化，避免斜向移动速度过快
    double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
    if (len_dir != 0)
    {
        double normalized_x = dir_x / len_dir;
        double normalized_y = dir_y / len_dir;
        pos.x += (int)(SPEED * normalized_x);
        pos.y += (int)(SPEED * normalized_y);
    }

    // 更新敌人朝向
    if (dir_x < 0)
        facing_left = true;
    else if (dir_x > 0)
        facing_left = false;
}

// 绘制敌人（含阴影、动画）
void Enemy::Draw(int delta)
{
    // 计算阴影绘制坐标（居中显示）
    int pos_shadow_x = pos.x + (FRAME_WIDTH / 2 - SHADOW_WIDTH / 2);
    int pos_shadow_y = pos.y + FRAME_HEIGHT - 15;
    // 绘制阴影
    putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);

    // 根据朝向播放对应动画，delta为帧间隔时间
    if (facing_left)
        anim_left->Play(pos.x, pos.y, delta);
    else
        anim_right->Play(pos.x, pos.y, delta);
}

// 敌人受击：直接标记为死亡
void Enemy::Hurt()
{
    alive = false;
}

// 返回敌人存活状态
bool Enemy::CheckAlive()
{
    return alive;
}