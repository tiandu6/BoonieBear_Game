#include "OrbitalStrike.h"
#include "Enemy.h"
#include <cmath>
#include <algorithm>

using namespace std;

void OrbitalStrikeSkill::Upgrade()
{
    level++;
}

void OrbitalStrikeSkill::Update(vector<Enemy*>& enemy_list, int player_atk, POINT player_pos)
{
    if (level <= 0) return;
    DWORD current_tick = GetTickCount();

    // 冷却缩减公式：根据等级指数级缩短发火周期
    int strike_interval = max(1500, 3000 - level * 200);

    // 计算生成新一轮轨道打击阵列
    if (current_tick - last_strike_time > strike_interval)
    {
        last_strike_time = current_tick;
        if (!enemy_list.empty())
        {
            // 通过随机数产生目标索敌
            int idx = rand() % enemy_list.size();
            OrbitalStrike strike;

            // 提取目标向宿主方向的移动推演向量
            double dx = player_pos.x - enemy_list[idx]->GetPosition().x;
            double dy = player_pos.y - enemy_list[idx]->GetPosition().y;
            double dist = sqrt(dx * dx + dy * dy);

            double lead_x = 0, lead_y = 0;
            if (dist > 0)
            {
                // 固定施加 100 像素的提前量补偿 0.6 秒轰炸延迟
                lead_x = (dx / dist) * 100.0;
                lead_y = (dy / dist) * 100.0;
            }

            // 合成带预判的高精度锁定打击坐标原点
            strike.x = enemy_list[idx]->GetPosition().x + 40 + lead_x;
            strike.y = enemy_list[idx]->GetPosition().y + 40 + lead_y;
            strike.start_time = current_tick;
            strikes_list.push_back(strike);
        }
    }

    // 独立推进场上所有弹道实体的阶段时间与状态流
    for (auto& strike : strikes_list)
    {
        if (!strike.active) continue;
        DWORD elapsed = current_tick - strike.start_time;

        // 验证 600ms 引信时间以分发伤害结算判定协议
        if (elapsed >= 600 && !strike.damaged)
        {
            strike.damaged = true;
            // 获取技能组件修正后的基准伤害阈值
            int strike_dmg = player_atk * 2 + (level * 2);
            for (Enemy* enemy : enemy_list)
            {
                // 检测碰撞边界判定区域
                double dx = enemy->GetPosition().x + 40 - strike.x;
                double dy = enemy->GetPosition().y + 40 - strike.y;

                if (sqrt(dx * dx + dy * dy) <= 120)
                {
                    enemy->Hurt(strike_dmg, { (int)strike.x, (int)strike.y });
                    enemy->ApplySkillFlicker(150);
                }
            }
        }

        // 生命周期逾限处理
        if (elapsed > 1200) strike.active = false;
    }

    // 应用惰性回收销毁无用状态内存块
    for (size_t i = 0; i < strikes_list.size(); i++)
    {
        if (!strikes_list[i].active)
        {
            swap(strikes_list[i], strikes_list.back());
            strikes_list.pop_back();
            i--;
        }
    }
}

void OrbitalStrikeSkill::Draw(bool is_paused, DWORD pause_start_time)
{
    if (level <= 0) return;
    DWORD current_tick = GetTickCount();
    // 监听视图冻结的挂起状态
    if (is_paused) current_tick = pause_start_time;

    for (const auto& strike : strikes_list)
    {
        DWORD elapsed = current_tick - strike.start_time;
        int center_x = (int)strike.x;
        int center_y = (int)strike.y;

        // 图元生成第一阶段：动态聚能锁定渲染光环
        if (elapsed < 600)
        {
            double prog = elapsed / 600.0;

            // 色彩动态过渡映射（黄->红）
            int g_color = 200 - (int)(200 * prog);
            setlinecolor(RGB(255, g_color, 50));
            setlinestyle(PS_SOLID, 2);

            // 准星追踪缩圈
            int radius = 160 - (int)(prog * 40);
            circle(center_x, center_y, radius);

            // 十字锚点跟随自旋追踪反馈
            double spin_angle = prog * 3.14159;
            int line_len = 15 + (int)(prog * 10);
            for (int i = 0; i < 4; i++)
            {
                double a = spin_angle + i * 1.5708;
                line(center_x + (int)(cos(a) * radius), center_y + (int)(sin(a) * radius),
                    center_x + (int)(cos(a) * (radius - line_len)), center_y + (int)(sin(a) * (radius - line_len)));
            }

            setfillcolor(RGB(255, 50, 50));
            fillcircle(center_x, center_y, (int)(prog * 6));
        }
        // 图元生成第二阶段：核爆冲击毁伤渲染模型
        else if (elapsed <= 1200)
        {
            double prog = (elapsed - 600) / 600.0;

            // 高频闪频振幅映射常数
            int pulse = rand() % 5 - 2;
            int max_width = 80 - (int)(prog * 70);

            // 复合高能伪泛光束（模拟光晕堆叠）
            if (max_width > 0)
            {
                setfillcolor(RGB(0, 80, 255));
                solidrectangle(center_x - max_width, 0, center_x + max_width, center_y + 10);

                setfillcolor(RGB(0, 200, 255));
                solidrectangle(center_x - max_width / 2 + pulse, 0, center_x + max_width / 2 - pulse, center_y + 10);

                setfillcolor(RGB(255, 255, 255));
                solidrectangle(center_x - max_width / 4, 0, center_x + max_width / 4, center_y + 10);
            }

            // 同步地面外向物理冲击光环扩散反馈
            int blast_radius = (int)(prog * 160);
            int thick = max(1, (int)(18 * (1.0 - prog)));
            setlinecolor(RGB(0, 255, 255));
            setlinestyle(PS_SOLID, thick);
            circle(center_x, center_y, blast_radius);

            int blast2_radius = (int)(prog * 100);
            setlinecolor(RGB(255, 255, 255));
            setlinestyle(PS_SOLID, max(1, thick / 2));
            circle(center_x, center_y, blast2_radius);

            // 同步地面向外极速粒子电弧放电飞溅特效
            setlinecolor(RGB(150, 255, 255));
            setlinestyle(PS_SOLID, 2);
            for (int i = 0; i < 6; i++)
            {
                double angle = i * 1.047 + (prog * 1.5);
                int dist = 30 + (int)(prog * 120);
                if (thick > 0) line(center_x + (int)(cos(angle) * dist), center_y + (int)(sin(angle) * dist),
                    center_x + (int)(cos(angle) * (dist - 25)), center_y + (int)(sin(angle) * (dist - 25)));
            }
        }
    }
    setlinestyle(PS_SOLID, 1);
}

void OrbitalStrikeSkill::Reset()
{
    level = 0;
    last_strike_time = 0;
    strikes_list.clear();
}

void OrbitalStrikeSkill::AddPauseTime(DWORD pause_duration)
{
    last_strike_time += pause_duration;
    for (auto& s : strikes_list) s.start_time += pause_duration;
}