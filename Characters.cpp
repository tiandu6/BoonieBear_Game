#include "Characters.h"
#include "Enemy.h" 

// 角色实体类实现：熊大
XiongDa::XiongDa()
{
    max_hp = 150;
    hp = 150;
    speed = 3;
    attack_damage = 2;
    anim_left = new Animation(atlas_xiongda_left, 100);
    anim_right = new Animation(atlas_xiongda_right, 100);
    dead_text = _T("光头强，你给我等着！");

    // 初始化专属武器实例配置
    current_weapon = new RockWeapon();
}

void XiongDa::UpdateAttacks()
{
    Player::UpdateAttacks();
}

void XiongDa::DrawAttacks()
{
    Player::DrawAttacks();

    DWORD elapsed = GetTickCount() - last_skill_time;

    // 控制主动技能特效生命周期 (设定为 600ms)
    if (elapsed < 600)
    {
        int center_x = pos.x + FRAME_WIDTH / 2;
        int center_y = pos.y + FRAME_HEIGHT / 2 + 10;
        double progress = elapsed / 600.0;

        // 图元渲染阶段 1：绘制实体掌印映射 (依据时间因子执行透明度衰减算法)
        if (progress < 0.6)
        {
            // 通过 RGB 分量动态重构模拟 Alpha 渐变操作
            int alpha = (int)(255 * (1.0 - progress / 0.6));
            COLORREF paw_color = RGB(92 * alpha / 255, 58 * alpha / 255, 33 * alpha / 255);

            setfillcolor(paw_color);
            setlinecolor(paw_color);

            fillellipse(center_x - 30, center_y - 10, center_x + 30, center_y + 35);
            fillcircle(center_x - 35, center_y - 30, 12);
            fillcircle(center_x - 12, center_y - 45, 14);
            fillcircle(center_x + 12, center_y - 45, 14);
            fillcircle(center_x + 35, center_y - 30, 12);
        }

        // 图元渲染阶段 2：计算并绘制双层同轴外扩冲击波
        int inner_wave = (int)(progress * 180);
        int outer_wave = (int)(progress * 260);

        setlinecolor(RGB(139, 69, 19));
        setlinestyle(PS_SOLID, max(1, (int)((1.0 - progress) * 15)));
        circle(center_x, center_y, inner_wave);

        setlinecolor(RGB(205, 133, 63));
        setlinestyle(PS_SOLID, max(1, (int)((1.0 - progress) * 8)));
        circle(center_x, center_y, outer_wave);

        // 图元渲染阶段 3：计算离散飞溅粒子坐标阵列
        setlinecolor(RGB(34, 139, 34));
        setfillcolor(RGB(184, 134, 11));
        setlinestyle(PS_SOLID, 1);

        for (int i = 0; i < 12; i++)
        {
            // 基于极坐标转换分布散点，并附加 Y 轴方向上的升力偏移补偿
            double angle = i * 0.523;
            int dist = (int)(progress * 150) + (i % 3) * 20;
            int px = center_x + (int)(cos(angle) * dist);
            int py = center_y + (int)(sin(angle) * dist) - (int)(progress * 50);

            int size = max(1, 5 - (int)(progress * 5));
            if (i % 2 == 0)
                fillcircle(px, py, size);
            else
                line(px, py, px + size * 2, py + size);
        }

        // 图元渲染阶段 4：执行技能文字状态 UI 呈现
        int text_y = pos.y - 40 - (int)(progress * 20);
        settextstyle(32, 0, _T("黑体"), 0, 0, FW_BOLD, false, false, false);
        setbkmode(TRANSPARENT);
        settextcolor(RGB(50, 20, 0));
        outtextxy(pos.x - 28, text_y + 2, _T("熊掌拍击！"));
        settextcolor(RGB(255, 165, 0));
        outtextxy(pos.x - 30, text_y, _T("熊掌拍击！"));
    }
}

void XiongDa::UseSkill(vector<Enemy*>& enemy_list)
{
    int center_x = pos.x + FRAME_WIDTH / 2;
    int center_y = pos.y + FRAME_HEIGHT / 2;
    int skill_radius = 250;

    for (size_t i = 0; i < enemy_list.size(); i++)
    {
        Enemy* enemy = enemy_list[i];
        POINT e_pos = enemy->GetPosition();

        // 依据欧几里得距离计算碰撞边界
        int dx = e_pos.x - center_x;
        int dy = e_pos.y - center_y;
        double dist = sqrt(dx * dx + dy * dy);

        // 检测目标是否处于作用域半径内
        if (dist <= skill_radius)
        {
            // 触发伤害判定链路模块
            enemy->Hurt(15, { center_x, center_y });

            // 追加特定渲染特效状态掩码
            enemy->ApplySkillFlicker(300);

            // 追加物理控制偏移运算 (执行推开位移效果)
            if (dist > 0)
            {
                int push_dist = 120;

                // 归一化后计算新坐标偏移点
                int new_x = e_pos.x + (int)((dx / dist) * push_dist);
                int new_y = e_pos.y + (int)((dy / dist) * push_dist);

                // 应用碰撞体边界安全检测协议
                if (new_x < 0)
                {
                    new_x = 0;
                }
                if (new_y < 0)
                {
                    new_y = 0;
                }
                if (new_x > WINDOW_WIDTH - 80)
                {
                    new_x = WINDOW_WIDTH - 80;
                }
                if (new_y > WINDOW_HEIGHT - 80)
                {
                    new_y = WINDOW_HEIGHT - 80;
                }

                // 重新派发目标实体坐标状态
                enemy->SetPosition({ new_x, new_y });
            }
        }
    }
}

// 角色实体类实现：熊二
XiongEr::XiongEr()
{
    max_hp = 100;
    hp = 100;
    speed = 4;
    attack_damage = 1;
    anim_left = new Animation(atlas_xionger_left, 80);
    anim_right = new Animation(atlas_xionger_right, 80);
    dead_text = _T("俺 的 蜂 蜜 ！！");

    // 初始化专属武器实例配置
    current_weapon = new HoneyWeapon();
}

void XiongEr::UpdateAttacks()
{
    Player::UpdateAttacks();
}

void XiongEr::DrawAttacks()
{
    Player::DrawAttacks();

    DWORD elapsed = GetTickCount() - last_skill_time;

    // 控制主动治愈技能特效生命周期 (设定为 800ms)
    if (elapsed < 800)
    {
        int center_x = pos.x + FRAME_WIDTH / 2;
        int center_y = pos.y + FRAME_HEIGHT / 2;

        // 构建规范化时间比率因子
        double progress = elapsed / 800.0;

        // 特效组件 1：计算外向能量扩散圆环坐标
        int aura_radius = (int)(progress * 150);
        int aura_thick = max(1, (int)((1.0 - progress) * 12));

        setlinecolor(RGB(50, 255, 100));
        setlinestyle(PS_SOLID, aura_thick);
        circle(center_x, center_y, aura_radius);

        // 特效组件 2：计算向心收缩能量环 (仅对生命周期前半段进行采样)
        if (progress < 0.5)
        {
            double inner_prog = progress * 2.0;
            int inner_radius = (int)((1.0 - inner_prog) * 120);
            int inner_thick = max(1, (int)(inner_prog * 8));

            setlinecolor(RGB(150, 255, 150));
            setlinestyle(PS_SOLID, inner_thick);
            circle(center_x, center_y, inner_radius);
        }

        // 特效组件 3：处理底部法阵的绘制变形约束
        int base_radius = 50 + (int)(sin(progress * 3.14159) * 15);
        setlinecolor(RGB(50, 200, 80));
        setlinestyle(PS_SOLID, max(1, aura_thick / 2));
        ellipse(center_x - base_radius, center_y + FRAME_HEIGHT / 2 - 15,
            center_x + base_radius, center_y + FRAME_HEIGHT / 2 + 5);

        setlinestyle(PS_SOLID, 1);

        // 特效组件 4：执行治愈文字状态 UI 呈现计算
        int text_y = pos.y - 20 - (int)(progress * 40);
        settextstyle(28, 0, _T("黑体"), 0, 0, FW_BOLD, false, false, false);
        setbkmode(TRANSPARENT);

        LPCTSTR heal_text = _T("紧急治愈！");

        settextcolor(RGB(20, 20, 20));
        outtextxy(pos.x - 28, text_y + 2, heal_text);

        settextcolor(RGB(50, 255, 50));
        outtextxy(pos.x - 30, text_y, heal_text);
    }
}

void XiongEr::UseSkill(vector<Enemy*>& enemy_list)
{
    // 应用恢复技能的反馈机制逻辑：按已损生命值百分比计算动态数值
    int missing_hp = max_hp - hp;
    int heal_amount = (int)(missing_hp * 0.25);

    // 设置最小回复阈值容错下限参数
    if (heal_amount < 5)
    {
        heal_amount = 5;
    }

    hp += heal_amount;

    // 执行状态界限溢出拦截
    if (hp > max_hp)
    {
        hp = max_hp;
    }
}

// 角色实体类实现：光头强
GuangtouQiang::GuangtouQiang()
{
    max_hp = 70;
    hp = 70;
    speed = 3;
    attack_damage = 3;
    shadow_offset_value = 30;
    anim_left = new Animation(atlas_qiang_left, 100);
    anim_right = new Animation(atlas_qiang_right, 100);
    dead_text = _T("李老板，我的工资啊...");

    // 初始化专属武器实例配置
    current_weapon = new GunWeapon();
}

void GuangtouQiang::UpdateAttacks()
{
    // 监听与管理技能的持续 Buff 提升生命周期
    if (GetTickCount() < skill_end_time)
    {
        attack_damage = 5;
        speed = 5;
    }
    else
    {
        attack_damage = 3;
        speed = 4;
    }

    Player::UpdateAttacks();
}

void GuangtouQiang::DrawAttacks()
{
    Player::DrawAttacks();

    // 绘制强化状态提示界面标记
    if (GetTickCount() < skill_end_time)
    {
        settextstyle(28, 0, _T("黑体"), 0, 0, FW_BOLD, false, false, false);
        settextcolor(RGB(255, 50, 50));
        outtextxy(pos.x - 20, pos.y - 30, _T("火力全开！"));
    }
}

void GuangtouQiang::UseSkill(vector<Enemy*>& enemy_list)
{
    // 追加 5000ms 技能强化增益时长
    skill_end_time = GetTickCount() + 5000;
}