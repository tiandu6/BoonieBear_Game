#include "Player.h"

Player::Player()
{
    // 加载角色通用脚下阴影素材
    loadimage(&img_shadow, _T("img/shadow_player.png"), SHADOW_WIDTH, 25, true);

    // 初始化技能冷却状态机，使其在生成时处于立即可用状态
    last_skill_time = GetTickCount() - SKILL_CD;
}

Player::~Player()
{
    if (anim_left) delete anim_left;
    if (anim_right) delete anim_right;

    // 清理实体挂载的武器组件实例
    if (current_weapon) delete current_weapon;
}

// 事件接收与状态机分发器
void Player::ProcessEvent(const ExMessage& msg)
{
    // 处理方向键与移动指令下达
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
    // 处理方向键与移动指令释放
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

// 物理移动更新结算器
void Player::Move()
{
    int dir_x = is_move_right - is_move_left;
    int dir_y = is_move_down - is_move_up;

    // 提取移动方向向量并执行归一化，规避对角线移动带来的速度放大偏移
    double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
    if (len_dir != 0)
    {
        double normalized_x = dir_x / len_dir;
        double normalized_y = dir_y / len_dir;
        pos.x += (int)(speed * normalized_x);
        pos.y += (int)(speed * normalized_y);
    }

    // 钳制视口边界坐标，阻止实体穿透屏幕
    if (pos.x < 0)
        pos.x = 0;
    if (pos.y < 0)
        pos.y = 0;
    if (pos.x + FRAME_WIDTH > WINDOW_WIDTH)
        pos.x = WINDOW_WIDTH - FRAME_WIDTH;
    if (pos.y + FRAME_HEIGHT > WINDOW_HEIGHT)
        pos.y = WINDOW_HEIGHT - FRAME_HEIGHT;
}

// 执行实体图元渲染及特效图层混合
void Player::Draw(int delta)
{
    if (delta > 0 && !is_dead)
    {
        int dir_x = is_move_right - is_move_left;
        if (dir_x < 0) facing_left = true;
        else if (dir_x > 0) facing_left = false;
    }

    // 同步执行阴影坐标计算与多态角色模型贴图中心对齐偏移校准
    int pos_shadow_x = pos.x + (FRAME_WIDTH / 2 - SHADOW_WIDTH / 2);

    if (facing_left) {
        pos_shadow_x += shadow_offset_value;
    }
    else {
        pos_shadow_x -= shadow_offset_value;
    }

    int pos_shadow_y = pos.y + 100;
    putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);

    // 渲染存活状态下属的生命槽 UI 组件
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

    int draw_x = pos.x - 35;
    int draw_y = pos.y - 35;
    DWORD current_time = GetTickCount();

    // 挂载阵亡阶段生命周期的相关特效渲染流
    if (is_dead)
    {
        DWORD elapsed = current_time - death_start_time;
        int center_x = pos.x + FRAME_WIDTH / 2;
        int center_y = pos.y + FRAME_HEIGHT / 2;

        // 特效序列 1：执行高强度向外扩散式激波图形混合计算
        if (elapsed < 800) {
            double wave_progress = elapsed / 800.0;
            int wave_radius = (int)(wave_progress * 600);

            setlinestyle(PS_SOLID, max(1, 20 - (int)(wave_progress * 20)));
            setlinecolor(RGB(255, 50, 50));
            circle(center_x, center_y, wave_radius);

            setlinestyle(PS_SOLID, max(1, 10 - (int)(wave_progress * 10)));
            setlinecolor(RGB(255, 200, 50));
            circle(center_x, center_y, (int)(wave_radius * 0.8));

            setlinestyle(PS_SOLID, 1);
        }

        // 推演刚体下坠相关的 Y 轴沉降偏置距离
        int body_drop = min((int)(elapsed / 10), 120);

        // 引入高频震颤效果
        if (elapsed < 1000) {
            draw_x += (rand() % 21 - 10);
            draw_y += (rand() % 11 - 5);
        }
        draw_y += body_drop;

        if (facing_left) anim_left->Play(draw_x, draw_y, 0);
        else anim_right->Play(draw_x, draw_y, 0);

        // 特效序列 2：通过重力函数模拟漫射抛物线物理飞溅粒子组
        if (elapsed < 2500) {
            for (int i = 0; i < 20; i++) {
                double angle = i * (3.14159 * 2.0 / 20.0);
                double speed = 1.0 + (i % 4) * 0.6;
                double radius = min((double)elapsed * speed, 350.0);

                // Y轴补偿模拟引力带来的自由落体行为
                double drop_y = (elapsed > 100) ? ((elapsed - 100) * (elapsed - 100) / 700.0) : 0;

                int px = center_x + (int)(radius * cos(angle));
                int py = center_y + (int)(radius * sin(angle)) + (int)drop_y;

                if (py > pos.y + FRAME_HEIGHT + 50) py = pos.y + FRAME_HEIGHT + 50;

                int size = max(1, 12 - (int)(elapsed / 200) + (i % 3));
                if (elapsed < 2000) {
                    setlinecolor(RGB(255, 120, 0));
                    setfillcolor((i % 2 == 0) ? RGB(255, 200, 50) : RGB(255, 150, 0));
                    fillcircle(px, py, size);
                }
            }
        }

        // 特效序列 3：计算螺旋晕眩星光粒子相对坐标
        for (int i = 0; i < 5; i++) {
            double angle = (elapsed / 80.0) + (i * 1.256);
            int star_x = center_x + (int)(55 * cos(angle));
            // 同步模型受重力下落的 Y 轴相对坐标联动
            int star_y = center_y - 70 + body_drop + (int)(20 * sin(angle));

            COLORREF star_color = (elapsed / 40 % 2 == 0) ? RGB(255, 255, 0) : RGB(255, 50, 50);
            setlinecolor(star_color);
            setfillcolor(star_color);
            fillcircle(star_x, star_y, 6 + (i % 2) * 2);
        }

        // 特效序列 4：执行遗言文字图形渐变展现
        if (elapsed < 3000) {
            int text_y = center_y - 100 - (int)(elapsed / 12);
            settextstyle(36, 0, _T("黑体"), 0, 0, FW_BOLD, false, false, false);
            setbkmode(TRANSPARENT);

            int text_w = textwidth(dead_text);

            settextcolor(RGB(20, 20, 20));
            outtextxy(center_x - text_w / 2 + 3, text_y + 3, dead_text);
            settextcolor(RGB(255, 255, 255));
            outtextxy(center_x - text_w / 2, text_y, dead_text);
        }
    }

    else
    {
        bool is_invulnerable = (current_time - last_hurt_time < 500);
        bool should_flicker_hide = false;

        // 执行无敌安全期帧缓冲(i-frame)剔除逻辑以表现半透明闪烁效果
        if (is_invulnerable)
        {
            if ((current_time / 50) % 2 == 0)
            {
                should_flicker_hide = true;
            }
        }

        if (!should_flicker_hide)
        {
            // 加入由于物理冲击导致的坐标抖动位移补偿
            if (is_invulnerable && (current_time - last_hurt_time < 200))
            {
                draw_x += (rand() % 11 - 5);
                draw_y += (rand() % 11 - 5);
            }

            if (facing_left) anim_left->Play(draw_x, draw_y, delta);
            else anim_right->Play(draw_x, draw_y, delta);

            // 绘制承伤高光爆发视觉滤镜图层
            if (current_time - last_hurt_time < 150)
            {
                setlinecolor(RGB(255, 50, 50));
                setfillcolor(RGB(200, 30, 30));
                fillcircle(pos.x + FRAME_WIDTH / 2, pos.y + FRAME_HEIGHT / 2 + 10, 35);
            }
        }
    }

    // 更新飘字系统的 Y 轴飘移动画状态
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
    if (is_dead) return;

    DWORD current_time = GetTickCount();
    // 拦截无敌缓冲帧内的连续重叠伤害判定逻辑
    if (current_time - last_hurt_time > 500)
    {
        // 挂载防具组件相关的基础减伤数值拦截运算机制
        int final_damage = damage - (int)(damage * damage_reduction);
        if (final_damage < 1 && damage > 0) final_damage = 1;

        hp -= final_damage;
        last_hurt_time = current_time;

        // 推送受击数值队列以生成浮空状态表现
        popup_damage = final_damage;
        popup_pos.x = pos.x + FRAME_WIDTH / 2;
        popup_pos.y = pos.y - 20;
        popup_start_time = current_time;
        is_popup_active = true;

        if (hp <= 0)
        {
            hp = 0;
            is_dead = true;
            death_start_time = current_time;
        }
    }
}

// 供全局状态控制器调用的主环境状态及依赖清理过程
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

    is_dead = false;
    death_start_time = 0;

    facing_left = false;

    last_skill_time = GetTickCount() - SKILL_CD;
    skill_end_time = 0;

    if (current_weapon)
    {
        current_weapon->Reset();
    }

    level = 1;
    exp = 0;
    max_exp = 5;
    SKILL_CD = 8000;

    damage_reduction = 0.0;
    orbital_skill.Reset();
}

void Player::SetPosition(POINT p)
{
    pos = p;
}

int Player::GetAttackDamage() const
{
    return attack_damage;
}

vector<Bullet>& Player::GetBullets()
{
    return current_weapon->GetBullets();
}

void Player::UpdateAttacks()
{
    if (current_weapon)
    {
        bool is_skill_active = (GetTickCount() < skill_end_time);
        current_weapon->Update(pos, facing_left, FRAME_WIDTH, FRAME_HEIGHT, is_skill_active);
    }
}
void Player::DrawAttacks()
{
    if (current_weapon)
    {
        bool is_skill_active = (GetTickCount() < skill_end_time);
        current_weapon->Draw(pos, facing_left, FRAME_WIDTH, FRAME_HEIGHT, is_skill_active);
    }
}
void Player::UseSkill(vector<Enemy*>& enemy_list) {}

// 当场景处于被挂起或拦截操作状态时提供内部计时器偏移的同步校准能力
void Player::AddPauseTime(DWORD pause_duration)
{
    last_skill_time += pause_duration;
    if (skill_end_time > 0) skill_end_time += pause_duration;
    if (last_hurt_time > 0) last_hurt_time += pause_duration;
    if (popup_start_time > 0) popup_start_time += pause_duration;
    if (death_start_time > 0) death_start_time += pause_duration;
}

// 事件派发：处理局内增益属性池升级配置注入
void Player::ApplyUpgrade(int upgrade_id)
{
    switch (upgrade_id)
    {
    case 0:
        max_hp += 20;
        hp += 20;
        break;
    case 1:
        attack_damage += 1;
        break;
    case 2:
        speed += 1;
        break;
    case 3:
        hp += (max_hp / 2);
        if (hp > max_hp) hp = max_hp;
        break;
    case 4:
        SKILL_CD = (int)(SKILL_CD * 0.8);

        if (SKILL_CD < 1000)
        {
            SKILL_CD = 1000;
        }
        break;
    case 5:
        damage_reduction += 0.20;
        if (damage_reduction > 0.80) damage_reduction = 0.80;
        break;
    case 6:
        orbital_skill.Upgrade();
        break;
    }
}

// 主动发起对外部可拓展武器与技能插槽更新派发的请求指令
void Player::UpdateExtraSkills(vector<Enemy*>& enemy_list)
{
    orbital_skill.Update(enemy_list, attack_damage, pos);
}