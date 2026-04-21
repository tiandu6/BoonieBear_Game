#include "Player.h"

Player::Player()
{
    // 基类只负责加载通用的阴影，不再加载具体的角色动画
    loadimage(&img_shadow, _T("img/shadow_player.png"), SHADOW_WIDTH, 25, true);
}

Player::~Player()
{
    // 虚析构函数释放动画资源
    if (anim_left) delete anim_left;
    if (anim_right) delete anim_right;
}

// 处理玩家输入事件（支持方向键和 WASD）
void Player::ProcessEvent(const ExMessage& msg)
{
    if (msg.message == WM_KEYDOWN)
    {
        switch (msg.vkcode)
        {
        case VK_UP:
        case 'W':
            is_move_up = true;
            break;
        case VK_DOWN:
        case 'S':
            is_move_down = true;
            break;
        case VK_LEFT:
        case 'A':
            is_move_left = true;
            break;
        case VK_RIGHT:
        case 'D':
            is_move_right = true;
            break;
        }
    }
    else if (msg.message == WM_KEYUP)
    {
        switch (msg.vkcode)
        {
        case VK_UP:
        case 'W':
            is_move_up = false;
            break;
        case VK_DOWN:
        case 'S':
            is_move_down = false;
            break;
        case VK_LEFT:
        case 'A':
            is_move_left = false;
            break;
        case VK_RIGHT:
        case 'D':
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
        pos.x += (int)(speed * normalized_x);
        pos.y += (int)(speed * normalized_y);
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

// 绘制玩家（阴影+动画+死亡特效）
void Player::Draw(int delta)
{
    // 使用类成员变量 facing_left 记录朝向
    if (delta > 0 && !is_dead)
    {
        int dir_x = is_move_right - is_move_left;
        if (dir_x < 0) facing_left = true;
        else if (dir_x > 0) facing_left = false;
    }

    // 【修改 2：加入阴影偏移逻辑】
    // 计算阴影绘制坐标
    int pos_shadow_x = pos.x + (FRAME_WIDTH / 2 - SHADOW_WIDTH / 2);

    // 根据朝向动态调整阴影X轴偏移（使用类成员变量，支持不同角色多态定制）
    if (facing_left) {
        pos_shadow_x += shadow_offset_value;
    }
    else {
        pos_shadow_x -= shadow_offset_value;
    }

    int pos_shadow_y = pos.y + 100;
    putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);

    // ================= 1. 绘制脚下血条（死亡时隐藏） =================
    // 增加了 is_game_started 判断，只有正式进游戏后才显示血条
    if (!is_dead && is_game_started)
    {
        int bar_width = 70;
        int bar_height = 8;
        int bar_x = pos.x + (FRAME_WIDTH - bar_width) / 2;
        int bar_y = pos.y + FRAME_HEIGHT + 20;

        setlinecolor(RGB(30, 30, 30));
        setfillcolor(RGB(60, 60, 60));
        fillroundrect(bar_x, bar_y, bar_x + bar_width, bar_y + bar_height, 4, 4);

        if (hp > 0)
        {
            int fill_width = (int)((double)hp / max_hp * bar_width);
            COLORREF hp_color = (hp > max_hp / 2) ? RGB(50, 220, 50) : ((hp > max_hp / 4) ? RGB(255, 200, 50) : RGB(255, 50, 50));
            setfillcolor(hp_color);
            solidroundrect(bar_x + 1, bar_y + 1, bar_x + fill_width - 1, bar_y + bar_height - 1, 4, 4);
        }
    }

    // 设置基础绘制坐标与时间戳
    int draw_x = pos.x - 35;
    int draw_y = pos.y - 35;
    DWORD current_time = GetTickCount();

    // ================= 2. 死亡动画特效分支 (史诗级强化版) =================
    if (is_dead)
    {
        DWORD elapsed = current_time - death_start_time;
        int center_x = pos.x + FRAME_WIDTH / 2;
        int center_y = pos.y + FRAME_HEIGHT / 2;

        // 【特效 A：高能能量波释放】(向外急速扩散的巨大双重冲击波)
        if (elapsed < 800) {
            double wave_progress = elapsed / 800.0;
            int wave_radius = (int)(wave_progress * 600); // 半径扩散到 600 像素

            // 外层红色警戒波
            setlinestyle(PS_SOLID, max(1, 20 - (int)(wave_progress * 20))); // 线条随时间变细消失
            setlinecolor(RGB(255, 50, 50));
            circle(center_x, center_y, wave_radius);

            // 内层金色能量波
            setlinestyle(PS_SOLID, max(1, 10 - (int)(wave_progress * 10)));
            setlinecolor(RGB(255, 200, 50));
            circle(center_x, center_y, (int)(wave_radius * 0.8));

            setlinestyle(PS_SOLID, 1); // 恢复默认线宽
        }

        // 【新增】：单独计算出身体下沉的距离
        int body_drop = min((int)(elapsed / 10), 120);

        // 【特效 B：角色剧烈抽搐与滑落】
        if (elapsed < 1000) {
            draw_x += (rand() % 21 - 10); // 极其剧烈的左右震荡
            draw_y += (rand() % 11 - 5);  // 上下微震
        }
        draw_y += body_drop; // 身体沉重下坠，倒在地上

        // 绘制静止的角色本体
        if (facing_left) anim_left->Play(draw_x, draw_y, 0);
        else anim_right->Play(draw_x, draw_y, 0);

        // 【特效 C：漫天飞舞的蜂蜜大爆浆】(粒子数量加倍，模拟真实的物理抛物线)
        if (elapsed < 2500) {
            for (int i = 0; i < 20; i++) { // 爆出 20 滴大小不一的蜂蜜
                double angle = i * (3.14159 * 2.0 / 20.0);
                double speed = 1.0 + (i % 4) * 0.6; // 不同的初速度产生层次感
                double radius = min((double)elapsed * speed, 350.0); // 向外狂喷

                // 重力抛物线下坠模拟
                double drop_y = (elapsed > 100) ? ((elapsed - 100) * (elapsed - 100) / 700.0) : 0;

                int px = center_x + (int)(radius * cos(angle));
                int py = center_y + (int)(radius * sin(angle)) + (int)drop_y;

                // 触地物理效果模拟（不会掉出屏幕下面）
                if (py > pos.y + FRAME_HEIGHT + 50) py = pos.y + FRAME_HEIGHT + 50;

                int size = max(1, 12 - (int)(elapsed / 200) + (i % 3)); // 粒子随时间蒸发变小
                if (elapsed < 2000) {
                    setlinecolor(RGB(255, 120, 0));
                    setfillcolor((i % 2 == 0) ? RGB(255, 200, 50) : RGB(255, 150, 0)); // 颜色交错
                    fillcircle(px, py, size);
                }
            }
        }

        // 【特效 D：五星连珠眩晕效果】
        for (int i = 0; i < 5; i++) {
            double angle = (elapsed / 80.0) + (i * 1.256);
            int star_x = center_x + (int)(55 * cos(angle));
            // 【修改】：在 Y 坐标上加上 body_drop，让星星跟着身体一起滑落！
            int star_y = center_y - 70 + body_drop + (int)(20 * sin(angle));

            COLORREF star_color = (elapsed / 40 % 2 == 0) ? RGB(255, 255, 0) : RGB(255, 50, 50);
            setlinecolor(star_color);
            setfillcolor(star_color);
            fillcircle(star_x, star_y, 6 + (i % 2) * 2);
        }

        // 【特效 E：灵魂遗言飘字】
        if (elapsed < 3000) {
            int text_y = center_y - 100 - (int)(elapsed / 12);
            settextstyle(36, 0, _T("黑体"), 0, 0, FW_BOLD, false, false, false);
            setbkmode(TRANSPARENT);

            // 【修改】：删掉了原来的硬编码，直接使用当前角色对象的 dead_text 成员变量
            int text_w = textwidth(dead_text);

            // 黑底白字，增加戏剧冲击力
            settextcolor(RGB(20, 20, 20));
            outtextxy(center_x - text_w / 2 + 3, text_y + 3, dead_text);
            settextcolor(RGB(255, 255, 255));
            outtextxy(center_x - text_w / 2, text_y, dead_text);
        }
    }

    // ================= 3. 正常存活绘制分支 =================
    else
    {
        bool is_invulnerable = (current_time - last_hurt_time < 500);

        // 正常受击震动 (Jitter)
        if (is_invulnerable && (current_time - last_hurt_time < 200))
        {
            draw_x += (rand() % 11 - 5);
            draw_y += (rand() % 11 - 5);
        }

        if (facing_left) anim_left->Play(draw_x, draw_y, delta);
        else anim_right->Play(draw_x, draw_y, delta);

        // 正常受击高光 (Hit Flash)
        if (current_time - last_hurt_time < 150)
        {
            setlinecolor(RGB(255, 50, 50));
            setfillcolor(RGB(200, 30, 30));
            fillcircle(pos.x + FRAME_WIDTH / 2, pos.y + FRAME_HEIGHT / 2 + 10, 35);
        }
    }

    // ================= 4. 浮动伤害数字 (致命一击的数字也要飘完) =================
    if (is_popup_active)
    {
        DWORD elapsed = current_time - popup_start_time;
        if (elapsed < 800)
        {
            int text_y = popup_pos.y - (elapsed / 15);
            TCHAR text[16];
            _stprintf_s(text, _T("-%d"), popup_damage);

            settextstyle(28, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
            setbkmode(TRANSPARENT);

            int text_w = textwidth(text);
            int text_x = popup_pos.x - text_w / 2;

            settextcolor(RGB(20, 20, 20));
            outtextxy(text_x + 2, text_y + 2, text);
            settextcolor(RGB(255, 50, 50));
            outtextxy(text_x, text_y, text);
        }
        else
        {
            is_popup_active = false;
        }
    }
}


// 获取玩家当前坐标（只读）
const POINT& Player::GetPosition() const
{
    return pos;
}

int Player::GetMaxHP() const
{
    return max_hp;
}

int Player::GetHP() const
{
    return hp;
}

void Player::TakeDamage(int damage)
{
    if (is_dead) return; // 如果已经死了，就不再重复受击

    // 0.5秒的受击无敌帧，防止连续扣血暴毙
    DWORD current_time = GetTickCount();
    if (current_time - last_hurt_time > 500)
    {
        hp -= damage;
        last_hurt_time = current_time;

        // 触发浮动伤害数字
        popup_damage = damage;
        popup_pos.x = pos.x + FRAME_WIDTH / 2;
        popup_pos.y = pos.y - 20;
        popup_start_time = current_time;
        is_popup_active = true;

        // 【新增】：检测死亡并触发死亡动画时间轴
        if (hp <= 0)
        {
            hp = 0;
            is_dead = true;
            death_start_time = current_time;
        }
    }
}

// 重置玩家状态（用于再来一局）
void Player::Reset()
{
    hp = max_hp;
    pos = { 500, 500 };
    is_move_up = false;
    is_move_down = false;
    is_move_left = false;
    is_move_right = false;
    is_popup_active = false;
    last_hurt_time = 0;

    //重置死亡状态
    is_dead = false;
    death_start_time = 0;

    // 重置朝向为默认的朝右
    facing_left = false;
}

// 【新增】：坐标设置函数
void Player::SetPosition(POINT p)
{
    pos = p;
}

int Player::GetAttackDamage() const
{
    return attack_damage;
}