#include "Characters.h"
#include "Enemy.h" 

// ======================== 【派生实现：熊大】 ========================
XiongDa::XiongDa()
{
    // 注入坦克型基础数值模型
    max_hp = 150;
    hp = 150;
    speed = 3;
    attack_damage = 2;
    anim_left = new Animation(atlas_xiongda_left, 100);
    anim_right = new Animation(atlas_xiongda_right, 100);
    dead_text = _T("光头强，你给我等着！");

    // 绑定慢速高伤的近战防身滚石武器
    current_weapon = new RockWeapon();
}

void XiongDa::UpdateAttacks()
{
    Player::UpdateAttacks(); // 向上溯源调用通用弹道更新
}

// 渲染熊大的范围震地（群控）技能可视化反馈
void XiongDa::DrawAttacks()
{
    Player::DrawAttacks();

    DWORD elapsed = GetTickCount() - last_skill_time;

    // 限制技能 UI 的最高活跃周期为 600ms
    if (elapsed < 600)
    {
        int center_x = pos.x + FRAME_WIDTH / 2;
        int center_y = pos.y + FRAME_HEIGHT / 2 + 10;
        double progress = elapsed / 600.0;

        // 阶段一：依据生命周期映射 Alpha 透明度的巨大肉球足迹
        if (progress < 0.6)
        {
            // 通过 RGB 分量系数缩放模拟淡出 (Fade-out)
            int alpha = (int)(255 * (1.0 - progress / 0.6));
            COLORREF paw_color = RGB(92 * alpha / 255, 58 * alpha / 255, 33 * alpha / 255);

            setfillcolor(paw_color);
            setlinecolor(paw_color);

            // 组合图形勾勒庞大的熊掌底纹
            fillellipse(center_x - 30, center_y - 10, center_x + 30, center_y + 35); // 掌心
            fillcircle(center_x - 35, center_y - 30, 12);                            // 左侧趾头
            fillcircle(center_x - 12, center_y - 45, 14);                            // 左中趾头
            fillcircle(center_x + 12, center_y - 45, 14);                            // 右中趾头
            fillcircle(center_x + 35, center_y - 30, 12);                            // 右侧趾头
        }

        // 阶段二：计算同心冲击波形变外扩参数
        int inner_wave = (int)(progress * 180);
        int outer_wave = (int)(progress * 260);

        setlinecolor(RGB(139, 69, 19));
        setlinestyle(PS_SOLID, max(1, (int)((1.0 - progress) * 15)));
        circle(center_x, center_y, inner_wave);

        setlinecolor(RGB(205, 133, 63));
        setlinestyle(PS_SOLID, max(1, (int)((1.0 - progress) * 8)));
        circle(center_x, center_y, outer_wave);

        // 阶段三：基于抛物面轨迹散射岩石崩裂碎片
        setlinecolor(RGB(34, 139, 34));
        setfillcolor(RGB(184, 134, 11));
        setlinestyle(PS_SOLID, 1);

        for (int i = 0; i < 12; i++)
        {
            double angle = i * 0.523;
            // 获取向外的离心运动量并附加周期跳变差量
            int dist = (int)(progress * 150) + (i % 3) * 20;
            int px = center_x + (int)(cos(angle) * dist);
            int py = center_y + (int)(sin(angle) * dist) - (int)(progress * 50); // Y轴抗引力提升

            int size = max(1, 5 - (int)(progress * 5));
            // 交替绘制多边形碎块与圆形渣土
            if (i % 2 == 0) fillcircle(px, py, size);
            else line(px, py, px + size * 2, py + size);
        }

        // 阶段四：绘制随技能周期同步淡出上升的战斗怒吼文字
        int text_y = pos.y - 40 - (int)(progress * 20);
        settextstyle(32, 0, _T("黑体"), 0, 0, FW_BOLD, false, false, false);
        setbkmode(TRANSPARENT);
        settextcolor(RGB(50, 20, 0));
        outtextxy(pos.x - 28, text_y + 2, _T("熊掌拍击！"));
        settextcolor(RGB(255, 165, 0));
        outtextxy(pos.x - 30, text_y, _T("熊掌拍击！"));
    }
}

// 熊大主动技能业务处理层：AOE 扫击击退判定机制
void XiongDa::UseSkill(vector<Enemy*>& enemy_list)
{
    int center_x = pos.x + FRAME_WIDTH / 2;
    int center_y = pos.y + FRAME_HEIGHT / 2;
    int skill_radius = 250; // 定义技能波及物理判定阈值

    for (size_t i = 0; i < enemy_list.size(); i++)
    {
        Enemy* enemy = enemy_list[i];
        POINT e_pos = enemy->GetPosition();

        // 抽取敌我双方向量位移，解算欧几里得测距
        int dx = e_pos.x - center_x;
        int dy = e_pos.y - center_y;
        double dist = sqrt(dx * dx + dy * dy);

        // 如果敌人处在打击有效波及圈内
        if (dist <= skill_radius)
        {
            // 打断当前攻势并削去 15 点生命
            enemy->Hurt(15, { center_x, center_y });
            // 下发受击异常发光状态锁
            enemy->ApplySkillFlicker(300);

            // 基于物理向量执行强制位移击退算法
            if (dist > 0)
            {
                int push_dist = 120; // 定义刚性位移系数

                // 标量反向解析计算受击落地新坐标系
                int new_x = e_pos.x + (int)((dx / dist) * push_dist);
                int new_y = e_pos.y + (int)((dy / dist) * push_dist);

                // 改用 WINDOW_WIDTH/HEIGHT 全局宏定义框定推移边界以防止卡出屏幕外
                if (new_x < 0) new_x = 0;
                if (new_y < 0) new_y = 0;
                if (new_x > WINDOW_WIDTH - 80) new_x = WINDOW_WIDTH - 80;
                if (new_y > WINDOW_HEIGHT - 80) new_y = WINDOW_HEIGHT - 80;

                enemy->SetPosition({ new_x, new_y }); // 下发重定位操作
            }
        }
    }
}

// ======================== 【派生实现：熊二】 ========================
XiongEr::XiongEr()
{
    // 注入高速脆皮游走型数值模型
    max_hp = 100;
    hp = 100;
    speed = 4;
    attack_damage = 1;
    anim_left = new Animation(atlas_xionger_left, 80);
    anim_right = new Animation(atlas_xionger_right, 80);
    dead_text = _T("俺 的 蜂 蜜 ！！");

    // 绑定高频抛射蜂蜜罐武器
    current_weapon = new HoneyWeapon();
}

void XiongEr::UpdateAttacks()
{
    Player::UpdateAttacks();
}

// 渲染熊二专属自我疗愈技能的绿色光环特效
void XiongEr::DrawAttacks()
{
    Player::DrawAttacks();

    DWORD elapsed = GetTickCount() - last_skill_time;

    // 限制技能 UI 的最高活跃周期为 800ms
    if (elapsed < 800)
    {
        int center_x = pos.x + FRAME_WIDTH / 2;
        int center_y = pos.y + FRAME_HEIGHT / 2;
        double progress = elapsed / 800.0;

        // 构建外扩释放能量阵环
        int aura_radius = (int)(progress * 150);
        int aura_thick = max(1, (int)((1.0 - progress) * 12));

        setlinecolor(RGB(50, 255, 100));
        setlinestyle(PS_SOLID, aura_thick);
        circle(center_x, center_y, aura_radius);

        // 组合向内吸收坍缩能量阵环 (前半段活跃)
        if (progress < 0.5)
        {
            double inner_prog = progress * 2.0;
            int inner_radius = (int)((1.0 - inner_prog) * 120);
            int inner_thick = max(1, (int)(inner_prog * 8));

            setlinecolor(RGB(150, 255, 150));
            setlinestyle(PS_SOLID, inner_thick);
            circle(center_x, center_y, inner_radius);
        }

        // 绘制立体的底部生命法阵刻度
        int base_radius = 50 + (int)(sin(progress * 3.14159) * 15);
        setlinecolor(RGB(50, 200, 80));
        setlinestyle(PS_SOLID, max(1, aura_thick / 2));
        ellipse(center_x - base_radius, center_y + FRAME_HEIGHT / 2 - 15,
            center_x + base_radius, center_y + FRAME_HEIGHT / 2 + 5);

        setlinestyle(PS_SOLID, 1); // 收尾保护笔刷格式

        // 生成翠绿色补血反馈文字动画
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

// 熊二主动技能业务层：单体急救逻辑
void XiongEr::UseSkill(vector<Enemy*>& enemy_list)
{
    // 对其缺失血量进行采样，依据百分比配给相应的治疗量
    int missing_hp = max_hp - hp;
    int heal_amount = (int)(missing_hp * 0.25);

    // 插入保底容错阈值，确保血量将满时依然拥有起底治疗效益
    if (heal_amount < 5) heal_amount = 5;

    hp += heal_amount;

    // 防止治疗数值溢出栈爆
    if (hp > max_hp) hp = max_hp;
}

// ======================== 【派生实现：光头强】 ========================
GuangtouQiang::GuangtouQiang()
{
    // 注入高敏高爆发极限脆皮数值模型
    max_hp = 70;
    hp = 70;
    speed = 3;
    attack_damage = 3;
    shadow_offset_value = 30; // 适配由于拿枪带来的身位中心偏移动
    anim_left = new Animation(atlas_qiang_left, 100);
    anim_right = new Animation(atlas_qiang_right, 100);
    dead_text = _T("李老板，我的工资啊...");

    // 绑定高频直线贯穿猎枪
    current_weapon = new GunWeapon();
}

// 拦截武器触发与移动回调，接管光头强专属的双重物理提速 Buff
void GuangtouQiang::UpdateAttacks()
{
    // 若当前处在技能暴走周期内部（增益窗口未闭合）
    if (GetTickCount() < skill_end_time)
    {
        attack_damage = 5; // 狂暴伤害
        speed = 5;         // 狂暴移速
    }
    else
    {
        // 修复原先代码中错误重置移速为 4 导致永久提速的 Bug，恢复最初构造属性。
        attack_damage = 3;
        speed = 3;
    }

    Player::UpdateAttacks(); // 送返至物理引擎层应用以上修改
}

// 渲染火力全开常驻跟屏暴走红字提示
void GuangtouQiang::DrawAttacks()
{
    Player::DrawAttacks();

    if (GetTickCount() < skill_end_time)
    {
        settextstyle(28, 0, _T("黑体"), 0, 0, FW_BOLD, false, false, false);
        settextcolor(RGB(255, 50, 50));
        outtextxy(pos.x - 20, pos.y - 30, _T("火力全开！"));
    }
}

// 光头强自身面板修改注入层
void GuangtouQiang::UseSkill(vector<Enemy*>& enemy_list)
{
    // 操作寄存器：颁发一枚有效期为 5 秒的限时通行证印记供 Update 解析
    skill_end_time = GetTickCount() + 5000;
}