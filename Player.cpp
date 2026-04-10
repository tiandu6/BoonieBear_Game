#include "Player.h"

Player::Player()
{
    // 加载玩家阴影图片，指定宽高并启用透明
    loadimage(&img_shadow, _T("img/shadow_player.png"), SHADOW_WIDTH, 25, true);
    // 创建左右方向动画，帧间隔100ms
    anim_left = new Animation(atlas_player_left, 100);
    anim_right = new Animation(atlas_player_right, 100);
}

Player::~Player()
{
    // 释放动画资源
    delete anim_left;
    delete anim_right;
}

// 处理玩家输入事件
void Player::ProcessEvent(const ExMessage& msg)
{
    if (msg.message == WM_KEYDOWN)
    {
        // 按下方向键，标记对应移动状态
        switch (msg.vkcode)
        {
        case VK_UP:
            is_move_up = true;
            break;
        case VK_DOWN:
            is_move_down = true;
            break;
        case VK_LEFT:
            is_move_left = true;
            break;
        case VK_RIGHT:
            is_move_right = true;
            break;
        }
    }
    else if (msg.message == WM_KEYUP)
    {
        // 松开方向键，取消对应移动状态
        switch (msg.vkcode)
        {
        case VK_UP:
            is_move_up = false;
            break;
        case VK_DOWN:
            is_move_down = false;
            break;
        case VK_LEFT:
            is_move_left = false;
            break;
        case VK_RIGHT:
            is_move_right = false;
            break;
        }
    }
}

// 玩家移动逻辑（含边界限制）
void Player::Move()
{
    // 计算移动方向向量（x：右-左，y：下-上）
    int dir_x = is_move_right - is_move_left;
    int dir_y = is_move_down - is_move_up;

    // 向量归一化，避免斜向移动速度过快
    double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
    if (len_dir != 0)
    {
        double normalized_x = dir_x / len_dir;
        double normalized_y = dir_y / len_dir;
        pos.x += (int)(SPEED * normalized_x);
        pos.y += (int)(SPEED * normalized_y);
    }

    // 边界限制：防止玩家移出屏幕
    if (pos.x < 0)
        pos.x = 0;
    if (pos.y < 0)
        pos.y = 0;
    if (pos.x + FRAME_WIDTH > WINDOW_WIDTH)
        pos.x = WINDOW_WIDTH - FRAME_WIDTH;
    if (pos.y + FRAME_HEIGHT > WINDOW_HEIGHT)
        pos.y = WINDOW_HEIGHT - FRAME_HEIGHT;
}

// 绘制玩家（阴影+动画）
void Player::Draw(int delta)
{
    // 计算阴影绘制坐标（居中显示）
    int pos_shadow_x = pos.x + (FRAME_WIDTH / 2 - SHADOW_WIDTH / 2);
    int pos_shadow_y = pos.y + 100;
    // 绘制阴影
    putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);

    // 判断玩家朝向（根据移动方向）
    static bool facing_left = false;
    int dir_x = is_move_right - is_move_left;
    if (dir_x < 0)
        facing_left = true;
    else if (dir_x > 0)
        facing_left = false;

    // 动画绘制偏移（适配150x150的原图尺寸）
    int draw_x = pos.x - 35;
    int draw_y = pos.y - 35;

    // 根据朝向播放对应动画
    if (facing_left)
        anim_left->Play(draw_x, draw_y, delta);
    else
        anim_right->Play(draw_x, draw_y, delta);
}

// 获取玩家当前坐标（只读）
const POINT& Player::GetPosition() const
{
    return pos;
}
