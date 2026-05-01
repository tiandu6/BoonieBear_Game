#include "Enemy.h"

Enemy::Enemy()
{
    // 在视口边界外生成随机出生点分配策略
    enum class SpawnEdge { Up = 0, Down, Left, Right };
    SpawnEdge edge = (SpawnEdge)(rand() % 4);
    switch (edge) {
    case SpawnEdge::Up: pos.x = rand() % WINDOW_WIDTH; pos.y = -150; break;
    case SpawnEdge::Down: pos.x = rand() % WINDOW_WIDTH; pos.y = WINDOW_HEIGHT; break;
    case SpawnEdge::Left: pos.x = -150; pos.y = rand() % WINDOW_HEIGHT; break;
    case SpawnEdge::Right: pos.x = WINDOW_WIDTH; pos.y = rand() % WINDOW_HEIGHT; break;
    }

    // 同步初始化双精度浮点模型坐标系
    real_x = pos.x;
    real_y = pos.y;
}

Enemy::~Enemy()
{
    delete anim_left;
    delete anim_right;
}

// 运算飞行弹道实体的碰撞判定
bool Enemy::CheckBulletCollision(const Bullet& bullet)
{
    if (hp <= 0) return false;

    // 对弹射物进行边界膨胀，提供更宽容的打击判定区间
    bool is_overlap_x = (bullet.pos.x + 25) >= pos.x && (bullet.pos.x - 25) <= pos.x + frame_width;
    bool is_overlap_y = (bullet.pos.y + 15) >= pos.y && (bullet.pos.y - 15) <= pos.y + frame_height;

    return is_overlap_x && is_overlap_y;
}

// 运算玩家实体的交互碰撞
bool Enemy::CheckPlayerCollision(const Player& player)
{
    if (hp <= 0) return false;

    // 配置碰撞包围盒的收缩阈值以修正模型贴图边缘的空白误差
    int shrink = 15;

    // 提取自身内敛化的响应边界
    int enemy_left = pos.x + shrink;
    int enemy_right = pos.x + frame_width - shrink;
    int enemy_top = pos.y + shrink;
    int enemy_bottom = pos.y + frame_height - shrink;

    // 提取玩家内敛化的响应边界
    const POINT& player_pos = player.GetPosition();
    int player_left = player_pos.x + shrink;
    int player_right = player_pos.x + player.FRAME_WIDTH - shrink;
    int player_top = player_pos.y + shrink;
    int player_bottom = player_pos.y + player.FRAME_HEIGHT - shrink;

    // 轴向对齐包围盒 (AABB) 交叉检测计算
    bool is_overlap_x = (enemy_left < player_right) && (enemy_right > player_left);
    bool is_overlap_y = (enemy_top < player_bottom) && (enemy_bottom > player_top);

    return is_overlap_x && is_overlap_y;
}

// 根据导航寻路算法驱动实体追击行为
void Enemy::Move(const Player& player, vector<Enemy*>& enemy_list)
{
    // 如果存在受击的刚体物理动量反馈，优先接管移动计算
    if (GetTickCount() < knockback_end_time)
    {
        real_x += knockback_vx;
        real_y += knockback_vy;
        pos.x = (int)real_x;
        pos.y = (int)real_y;

        // 执行视图边界钳制处理以防止被物理反馈弹射出有效视野
        if (pos.x < 0) { pos.x = 0; real_x = 0; }
        if (pos.x > WINDOW_WIDTH - frame_width) { pos.x = WINDOW_WIDTH - frame_width; real_x = pos.x; }
        if (pos.y < 0) { pos.y = 0; real_y = 0; }
        if (pos.y > WINDOW_HEIGHT - frame_height) { pos.y = WINDOW_HEIGHT - frame_height; real_y = pos.y; }

        return;
    }

    if (hp <= 0) return;

    // 获取相对于追踪目标的路径向量
    const POINT& player_pos = player.GetPosition();
    int dir_x = player_pos.x - pos.x;
    int dir_y = player_pos.y - pos.y;

    // 将路径向量单位化，并根据自身速率因子推算本帧的实际位移变化率
    double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
    if (len_dir != 0)
    {
        double normalized_x = dir_x / len_dir;
        double normalized_y = dir_y / len_dir;

        // 使用双精度浮点数累计真实位移，解决整型截断导致的低速移动丢失问题
        real_x += speed * normalized_x;
        real_y += speed * normalized_y;

        pos.x = (int)real_x;
        pos.y = (int)real_y;
    }

    // 派发并同步骨骼帧贴图朝向状态
    if (dir_x < 0) facing_left = true;
    else if (dir_x > 0) facing_left = false;
}

// 敌对模型与受击关联视觉流的渲染封装
void Enemy::Draw(int delta)
{
    // 拦截帧绘制请求：应用特定伤害类型的半透明频率闪烁机制
    if (GetTickCount() < skill_flicker_end_time)
    {
        // 构建50ms周期的隐形交替闪烁反馈
        if ((GetTickCount() / 50) % 2 == 0)
        {
            return;
        }
    }

    // 执行阴影贴图映射坐标及宽高的适应性配比渲染
    int pos_shadow_x = pos.x + (frame_width / 2 - shadow_width / 2);

    if (facing_left) {
        pos_shadow_x += shadow_offset_value;
    }
    else {
        pos_shadow_x -= shadow_offset_value;
    }

    int pos_shadow_y = pos.y + frame_height - shadow_offset_y;
    putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);

    if (facing_left)
        anim_left->Play(pos.x, pos.y, delta);
    else
        anim_right->Play(pos.x, pos.y, delta);

    // 区分 Boss 级与普通级的 UI 生命值组件渲染配置方案
    if (is_boss)
    {
        // 挂载首领专属血槽长背景外框
        int bar_width = 140;
        int bar_height = 16;
        int bar_x = pos.x + (frame_width - bar_width) / 2;
        int bar_y = pos.y - 30;

        setlinecolor(RGB(20, 20, 20));
        setfillcolor(RGB(60, 20, 20));
        fillroundrect(bar_x, bar_y, bar_x + bar_width, bar_y + bar_height, 4, 4);

        if (hp > 0) {
            int fill_width = (int)((double)hp / max_hp * bar_width);
            setfillcolor(RGB(255, 50, 50));
            solidroundrect(bar_x + 1, bar_y + 1, bar_x + fill_width - 1, bar_y + bar_height - 1, 4, 4);
        }

        // 绑定数值化文字渲染呈现
        TCHAR hp_txt[32];
        _stprintf_s(hp_txt, _T("%d / %d"), hp, max_hp);
        settextstyle(16, 0, _T("Arial"), 0, 0, FW_BOLD, false, false, false);
        setbkmode(TRANSPARENT);
        int tw = textwidth(hp_txt);
        int th = textheight(hp_txt);
        int tx = bar_x + (bar_width - tw) / 2;
        int ty = bar_y + (bar_height - th) / 2;

        settextcolor(RGB(20, 20, 20));
        outtextxy(tx + 1, ty + 1, hp_txt);
        settextcolor(RGB(255, 255, 255));
        outtextxy(tx, ty, hp_txt);
    }
    else
    {
        // 挂载标准小型血条图形化呈现
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
    // 控制受击瞬间爆发特效渲染生命周期为 150ms
    if (current_time - last_hurt_time < 150)
    {
        double progress = (current_time - last_hurt_time) / 150.0;
        int center_x = pos.x + frame_width / 2;
        int center_y = pos.y + frame_height / 2;

        // 依照不同的碰撞箱尺寸，配置基础爆火粒子比例缩放大小
        int hit_scale = 20;

        if (frame_width >= 120)
        {
            hit_scale = 30;
        }
        if (is_boss)
        {
            hit_scale = 60;
        }

        // 渲染内收星芒切线图案，运用衰减比例提供动感表现
        int star_len = (int)(hit_scale * (1.0 - progress));
        int star_thick = max(1, (int)(6 * (1.0 - progress)));

        if (star_len > 0)
        {
            setlinecolor(RGB(255, 80, 20));
            setlinestyle(PS_SOLID, star_thick + 4);
            line(center_x - star_len, center_y - star_len, center_x + star_len, center_y + star_len);
            line(center_x - star_len, center_y + star_len, center_x + star_len, center_y - star_len);

            setlinecolor(RGB(255, 255, 255));
            setlinestyle(PS_SOLID, star_thick);
            line(center_x - star_len, center_y - star_len, center_x + star_len, center_y + star_len);
            line(center_x - star_len, center_y + star_len, center_x + star_len, center_y - star_len);
        }

        // 绘制受物理向量反馈的粒子弹射溅射表现
        setlinecolor(RGB(255, 200, 50));
        setfillcolor(RGB(255, 200, 50));
        setlinestyle(PS_SOLID, 1);

        for (int i = 0; i < 3; i++)
        {
            double angle = i * 2.09 + (progress * 2.0);
            int px = center_x + (int)(cos(angle) * hit_scale * progress * 1.5);
            int py = center_y + (int)(sin(angle) * hit_scale * progress * 1.5);

            int particle_size = max(1, 4 - (int)(progress * 4));
            fillcircle(px, py, particle_size);
        }

        setlinestyle(PS_SOLID, 1);
    }
}

// 运算实体接受的最终伤害数据计算
bool Enemy::Hurt(int damage, const POINT& source_pos)
{
    DWORD current_time = GetTickCount();
    // 配置实体基础内聚安全无敌判定帧(i-frame)间隔为 300ms
    if (current_time - last_hurt_time > 300)
    {
        hp -= damage;
        last_hurt_time = current_time;

        // 根据入射弹道推导物理刚体击退的受力方向
        int center_x = pos.x + frame_width / 2;
        int center_y = pos.y + frame_height / 2;
        double dir_x = center_x - source_pos.x;
        double dir_y = center_y - source_pos.y;

        double len = sqrt(dir_x * dir_x + dir_y * dir_y);
        if (len != 0)
        {
            // 配置冲击受力速度为基线移速的4倍
            knockback_vx = (dir_x / len) * (speed * 4.0);
            knockback_vy = (dir_y / len) * (speed * 4.0);
        }

        // 附加 150ms 时长的硬直失控状态位移标签
        knockback_end_time = current_time + 150;

        return true;
    }
    return false;
}

bool Enemy::CheckAlive()
{
    // 对即将释放的单位提供 150ms 延迟缓存以便完全播放受击渲染图元动画
    if (hp <= 0 && GetTickCount() - last_hurt_time > 150)
    {
        alive = false;
    }
    return alive;
}

void Enemy::SetPosition(POINT p)
{
    pos = p;

    // 对冲数据坐标结构：以适配基于真实系统时钟的位移推演
    real_x = p.x;
    real_y = p.y;
}

POINT Enemy::GetPosition() const
{
    return pos;
}

// 抛出闪烁图元挂载配置选项
void Enemy::ApplySkillFlicker(int duration_ms)
{
    skill_flicker_end_time = GetTickCount() + duration_ms;
}