#include "Enemy.h"

// 构造函数：初始化时在视窗外围（上下左右四个边界）随机抽取一处作为出生点
Enemy::Enemy()
{
    enum class SpawnEdge { Up = 0, Down, Left, Right };
    SpawnEdge edge = (SpawnEdge)(rand() % 4);

    // 隐藏在视窗外 -150 像素的位置出生，实现平滑走入画面的效果
    switch (edge) {
    case SpawnEdge::Up: pos.x = rand() % WINDOW_WIDTH; pos.y = -150; break;
    case SpawnEdge::Down: pos.x = rand() % WINDOW_WIDTH; pos.y = WINDOW_HEIGHT; break;
    case SpawnEdge::Left: pos.x = -150; pos.y = rand() % WINDOW_HEIGHT; break;
    case SpawnEdge::Right: pos.x = WINDOW_WIDTH; pos.y = rand() % WINDOW_HEIGHT; break;
    }

    real_x = pos.x;
    real_y = pos.y;
}

Enemy::~Enemy()
{
    delete anim_left;
    delete anim_right;
}

// 弹道学 AABB 矩形碰撞检测
bool Enemy::CheckBulletCollision(const Bullet& bullet) const
{
    if (hp <= 0) return false; // 免疫鞭尸

    // 子弹包围盒构建与交集测试
    bool is_overlap_x = (bullet.pos.x + 25) >= pos.x && (bullet.pos.x - 25) <= pos.x + frame_width;
    bool is_overlap_y = (bullet.pos.y + 15) >= pos.y && (bullet.pos.y - 15) <= pos.y + frame_height;
    return is_overlap_x && is_overlap_y;
}

// 玩家刚体 AABB 碰撞检测
bool Enemy::CheckPlayerCollision(const Player& player) const
{
    if (hp <= 0) return false;

    // 剥离图片的空白外发光区域，向内收缩 15 像素作为硬核判定区，提升手感
    int shrink = 15;
    int enemy_left = pos.x + shrink;
    int enemy_right = pos.x + frame_width - shrink;
    int enemy_top = pos.y + shrink;
    int enemy_bottom = pos.y + frame_height - shrink;

    const POINT& player_pos = player.GetPosition();
    int player_left = player_pos.x + shrink;
    int player_right = player_pos.x + player.FRAME_WIDTH - shrink;
    int player_top = player_pos.y + shrink;
    int player_bottom = player_pos.y + player.FRAME_HEIGHT - shrink;

    // AABB 相交定律
    bool is_overlap_x = (enemy_left < player_right) && (enemy_right > player_left);
    bool is_overlap_y = (enemy_top < player_bottom) && (enemy_bottom > player_top);
    return is_overlap_x && is_overlap_y;
}

// 多人协同 AI 追踪算法核心模块
void Enemy::Move(const vector<Player*>& players, vector<Enemy*>& enemy_list)
{
    // ================= 强制位移态拦截 =================
    // 若当前处于受击被击退的硬直周期内，强制执行被动位移，忽略 AI 寻路指令
    if (GetTickCount() < knockback_end_time)
    {
        real_x += knockback_vx;
        real_y += knockback_vy;
        pos.x = (int)real_x;
        pos.y = (int)real_y;

        // 刚体碰撞箱约束：防止怪物被击飞到视窗外面下不来
        if (pos.x < 0) { pos.x = 0; real_x = 0; }
        if (pos.x > WINDOW_WIDTH - frame_width) { pos.x = WINDOW_WIDTH - frame_width; real_x = pos.x; }
        if (pos.y < 0) { pos.y = 0; real_y = 0; }
        if (pos.y > WINDOW_HEIGHT - frame_height) { pos.y = WINDOW_HEIGHT - frame_height; real_y = pos.y; }
        return;
    }

    if (hp <= 0) return; // 死亡禁锢

    // ================= 仇恨雷达目标测算 =================
    Player* target = nullptr;
    double min_dist = -1;

    // 扫描存活的玩家个体，通过计算欧几里得距离平方寻找当前最近单位
    for (Player* p : players) {
        if (p->GetHP() > 0) {
            double dx = p->GetPosition().x - pos.x;
            double dy = p->GetPosition().y - pos.y;
            double dist = dx * dx + dy * dy; // 避免开方运算优化性能
            if (target == nullptr || dist < min_dist) {
                min_dist = dist;
                target = p; // 锁定仇恨目标
            }
        }
    }

    if (!target) return; // 若目标全灭，原地挂起滞留

    // ================= 矢量迫近算法 =================
    int dir_x = target->GetPosition().x - pos.x;
    int dir_y = target->GetPosition().y - pos.y;

    // 勾股定理开方获取两点线段真实距离
    double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
    if (len_dir != 0)
    {
        // 归一化提取方向系数，并乘以自身移速，转化为本帧真实位移增量
        double normalized_x = dir_x / len_dir;
        double normalized_y = dir_y / len_dir;
        real_x += speed * normalized_x;
        real_y += speed * normalized_y;
        pos.x = (int)real_x;
        pos.y = (int)real_y;
    }

    // 更新绘制朝向流
    if (dir_x < 0) facing_left = true;
    else if (dir_x > 0) facing_left = false;
}

// 敌方实体渲染管线
void Enemy::Draw(int delta)
{
    // 如果处于特殊受击闪烁惩罚期，通过时间除法制造高频隐身闪烁错觉
    if (GetTickCount() < skill_flicker_end_time)
    {
        if ((GetTickCount() / 50) % 2 == 0) return;
    }

    // 绘制底部阴影
    int pos_shadow_x = pos.x + (frame_width / 2 - shadow_width / 2);
    if (facing_left) pos_shadow_x += shadow_offset_value;
    else pos_shadow_x -= shadow_offset_value;

    int pos_shadow_y = pos.y + frame_height - shadow_offset_y;
    if (p_img_shadow) putimage_alpha(pos_shadow_x, pos_shadow_y, p_img_shadow);

    // 驱动图集播放当前动作帧
    if (facing_left) anim_left->Play(pos.x, pos.y, delta);
    else anim_right->Play(pos.x, pos.y, delta);

    // ================= UI：生命刻度板渲染 =================
    if (is_boss)
    {
        // Boss 享有头顶大型专属生命条
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

        // 附带生命值文本透传
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
        // 杂鱼怪物采用微型几何生命条
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

    // ================= 特效：受击光斑与十字星飙血效果 =================
    DWORD current_time = GetTickCount();
    if (current_time - last_hurt_time < 150)
    {
        double progress = (current_time - last_hurt_time) / 150.0;
        int center_x = pos.x + frame_width / 2;
        int center_y = pos.y + frame_height / 2;

        // 动态根据怪物骨架结构体积适应爆炸范围
        int hit_scale = 20;
        if (frame_width >= 120) hit_scale = 30;
        if (is_boss) hit_scale = 60;

        int star_len = (int)(hit_scale * (1.0 - progress));
        int star_thick = max(1, (int)(6 * (1.0 - progress)));

        // 绘制收缩型受击四角十字芒星
        if (star_len > 0)
        {
            setlinecolor(RGB(255, 80, 20));
            setlinestyle(PS_SOLID, star_thick + 4); // 描边粗度
            line(center_x - star_len, center_y - star_len, center_x + star_len, center_y + star_len);
            line(center_x - star_len, center_y + star_len, center_x + star_len, center_y - star_len);
            setlinecolor(RGB(255, 255, 255));
            setlinestyle(PS_SOLID, star_thick);     // 芯线粗度
            line(center_x - star_len, center_y - star_len, center_x + star_len, center_y + star_len);
            line(center_x - star_len, center_y + star_len, center_x + star_len, center_y - star_len);
        }

        // 绘制散落的向外喷溅粒子
        setlinecolor(RGB(255, 200, 50));
        setfillcolor(RGB(255, 200, 50));
        setlinestyle(PS_SOLID, 1);

        for (int i = 0; i < 3; i++)
        {
            double angle = i * 2.09 + (progress * 2.0); // 附带自旋偏转
            int px = center_x + (int)(cos(angle) * hit_scale * progress * 1.5);
            int py = center_y + (int)(sin(angle) * hit_scale * progress * 1.5);
            int particle_size = max(1, 4 - (int)(progress * 4));
            fillcircle(px, py, particle_size);
        }
        setlinestyle(PS_SOLID, 1);
    }
}

// 敌方伤害响应逻辑
bool Enemy::Hurt(int damage, const POINT& source_pos)
{
    DWORD current_time = GetTickCount();
    // 300 毫秒基础抗僵直霸体免疫期（防止被高频低伤子弹无限粘滞）
    if (current_time - last_hurt_time > 300)
    {
        hp -= damage;
        last_hurt_time = current_time;

        // 计算目标自身几何中心
        int center_x = pos.x + frame_width / 2;
        int center_y = pos.y + frame_height / 2;

        // 反向测距提取远离伤害来源的外向排斥向量
        double dir_x = center_x - source_pos.x;
        double dir_y = center_y - source_pos.y;

        double len = sqrt(dir_x * dir_x + dir_y * dir_y);
        if (len != 0)
        {
            // 给受击对象施加原本移速 4 倍强度的动能冲量
            knockback_vx = (dir_x / len) * (speed * 4.0);
            knockback_vy = (dir_y / len) * (speed * 4.0);
        }

        // 赋予 150 毫秒的失控被击退时长
        knockback_end_time = current_time + 150;
        return true; // 返回真伤确认命中
    }
    return false; // 返回无敌免疫跳过
}

// 标记为 const
bool Enemy::CheckAlive() const
{
    // 如果血槽归零，且度过了 150 毫秒的最后受击动画观赏期，才判决物理死亡
    if (hp <= 0 && GetTickCount() - last_hurt_time > 150) return false;
    return alive;
}

void Enemy::SetPosition(POINT p)
{
    pos = p;
    real_x = p.x;
    real_y = p.y;
}

POINT Enemy::GetPosition() const { return pos; }
void Enemy::ApplySkillFlicker(int duration_ms) { skill_flicker_end_time = GetTickCount() + duration_ms; }