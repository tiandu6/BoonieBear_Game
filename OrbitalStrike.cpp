#include "OrbitalStrike.h"
#include "Enemy.h"
#include <cmath>
#include <algorithm>

using namespace std;

void OrbitalStrikeSkill::Upgrade()
{
    level++;
}

// 核心业务层：处理自动寻的预测、倒计时结算与圆柱形 AOE 伤害核算
void OrbitalStrikeSkill::Update(vector<Enemy*>& enemy_list, int player_atk, POINT player_pos)
{
    if (level <= 0) return; // 未获取该能力则静默
    DWORD current_tick = GetTickCount();

    // 冷却缩减公式：基准 3000ms 触发周期，每升一级剥减 200ms，硬底线封顶 1500ms
    int strike_interval = max(1500, 3000 - level * 200);

    // ================= 弹道生成与发射请求 =================
    if (current_tick - last_strike_time > strike_interval)
    {
        last_strike_time = current_tick;
        if (!enemy_list.empty())
        {
            // 通过随机数进行伪智能抽签抓取视窗内的怪物作为活靶
            int idx = rand() % enemy_list.size();
            OrbitalStrike strike;

            // 获取目前目标相对玩家的坐标空间相对矢量方向
            double dx = player_pos.x - enemy_list[idx]->GetPosition().x;
            double dy = player_pos.y - enemy_list[idx]->GetPosition().y;
            double dist = sqrt(dx * dx + dy * dy);

            // ================= 运动预测与提前量补偿算法 =================
            // 由于天基武器从天而降拥有 600ms 锁定延迟，怪物会走出原圈，故施加反向物理追猎预判
            double lead_x = 0, lead_y = 0;
            if (dist > 0)
            {
                // 固定朝怪物背对玩家逃离的方向补偿施加 100 像素坐标偏移预瞄（封走位）
                lead_x = (dx / dist) * 100.0;
                lead_y = (dy / dist) * 100.0;
            }

            // 装载带有补偿的最终打击核心坐标
            strike.x = enemy_list[idx]->GetPosition().x + 40 + lead_x;
            strike.y = enemy_list[idx]->GetPosition().y + 40 + lead_y;
            strike.start_time = current_tick;

            // 压入缓冲池准备下发执行
            strikes_list.push_back(strike);
        }
    }

    // ================= 场内弹道推进与终末伤害结算引擎 =================
    for (auto& strike : strikes_list)
    {
        if (!strike.active) continue;
        DWORD elapsed = current_tick - strike.start_time;

        // 引信控制：验证动画已抵达 600ms （即从红圈聚焦到落雷轰下的临界帧）
        if (elapsed >= 600 && !strike.damaged)
        {
            strike.damaged = true; // 锁死扣板，本发不再二次结算

            // 基础面板系数翻 2 倍，并追加等级附带真实伤害补正
            int strike_dmg = player_atk * 2 + (level * 2);
            for (Enemy* enemy : enemy_list)
            {
                // 检测处于爆心周围 120 像素半径范围内的波及敌人
                double dx = enemy->GetPosition().x + 40 - strike.x;
                double dy = enemy->GetPosition().y + 40 - strike.y;

                if (sqrt(dx * dx + dy * dy) <= 120)
                {
                    // 下发雷击高伤指令
                    enemy->Hurt(strike_dmg, { (int)strike.x, (int)strike.y });
                    // 施加短暂 150ms 抽搐闪光状态锁
                    enemy->ApplySkillFlicker(150);
                }
            }
        }

        // 生命周期逾限处理：演完 1200ms 总剧本后，通知 GC 将其回收
        if (elapsed > 1200) strike.active = false;
    }

    // O(1) 惰性回收废弃实体，解除内存高危压力
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

// 将天基武器物理状态翻译呈现至玩家显示器的像素视觉模块
void OrbitalStrikeSkill::Draw(bool is_paused, DWORD pause_start_time)
{
    if (level <= 0) return;
    DWORD current_tick = GetTickCount();
    // 冻结保护：若游戏正处于暂停界面内，挂起渲染时针阻断推演
    if (is_paused) current_tick = pause_start_time;

    for (const auto& strike : strikes_list)
    {
        DWORD elapsed = current_tick - strike.start_time;
        int center_x = (int)strike.x;
        int center_y = (int)strike.y;

        // ================= 渲染第一阶段：动态聚能雷达光环 (0 - 600ms) =================
        if (elapsed < 600)
        {
            double prog = elapsed / 600.0;

            // 色阶降级渐变演算机制：由安全黄预警转向极度危险红 (RGB(255,200,50) -> RGB(255,0,50))
            int g_color = 200 - (int)(200 * prog);
            setlinecolor(RGB(255, g_color, 50));
            setlinestyle(PS_SOLID, 2);

            // 光环动态向内锁敌聚缩效果
            int radius = 160 - (int)(prog * 40);
            circle(center_x, center_y, radius);

            // 十字基线瞄点顺时针翻滚测绘与内聚向心生长
            double spin_angle = prog * 3.14159;
            int line_len = 15 + (int)(prog * 10);
            for (int i = 0; i < 4; i++)
            {
                // 四个四分位象限依次布线生成十字军刀轮廓
                double a = spin_angle + i * 1.5708;
                line(center_x + (int)(cos(a) * radius), center_y + (int)(sin(a) * radius),
                    center_x + (int)(cos(a) * (radius - line_len)), center_y + (int)(sin(a) * (radius - line_len)));
            }

            // 中轴内点涨满爆发预警提示
            setfillcolor(RGB(255, 50, 50));
            fillcircle(center_x, center_y, (int)(prog * 6));
        }
        // ================= 渲染第二阶段：地核毁灭与高频电离激波层 (600 - 1200ms) =================
        else if (elapsed <= 1200)
        {
            // 获取后半段专属 0.0 - 1.0 的平滑阶段映射比例因子
            double prog = (elapsed - 600) / 600.0;

            // 振动常数：模拟超高压射线切透大气的视错觉频闪
            int pulse = rand() % 5 - 2;
            // 光柱宽度随时间快速衰缩
            int max_width = 80 - (int)(prog * 70);

            // 三合一光带混叠方案：深蓝外壳、湖蓝中层、耀眼发白内核构造立体穿透感光斑
            if (max_width > 0)
            {
                setfillcolor(RGB(0, 80, 255));
                solidrectangle(center_x - max_width, 0, center_x + max_width, center_y + 10);

                setfillcolor(RGB(0, 200, 255));
                solidrectangle(center_x - max_width / 2 + pulse, 0, center_x + max_width / 2 - pulse, center_y + 10);

                setfillcolor(RGB(255, 255, 255));
                solidrectangle(center_x - max_width / 4, 0, center_x + max_width / 4, center_y + 10);
            }

            // 同步输出自击穿爆点为圆心的地面排开冲击推移波环
            int blast_radius = (int)(prog * 160);
            // 环形厚度骤缩模拟能量涣散损耗
            int thick = max(1, (int)(18 * (1.0 - prog)));
            setlinecolor(RGB(0, 255, 255));
            setlinestyle(PS_SOLID, thick);
            circle(center_x, center_y, blast_radius);

            // 第二层极度锐利的内核电圈跟随扩展
            int blast2_radius = (int)(prog * 100);
            setlinecolor(RGB(255, 255, 255));
            setlinestyle(PS_SOLID, max(1, thick / 2));
            circle(center_x, center_y, blast2_radius);

            // 同步计算生成 6 道六角形极化带电崩散电涌光刺
            setlinecolor(RGB(150, 255, 255));
            setlinestyle(PS_SOLID, 2);
            for (int i = 0; i < 6; i++)
            {
                // 呈 60 度角发散排列并附加顺时针旋扭角
                double angle = i * 1.047 + (prog * 1.5);
                int dist = 30 + (int)(prog * 120);
                if (thick > 0) line(center_x + (int)(cos(angle) * dist), center_y + (int)(sin(angle) * dist),
                    center_x + (int)(cos(angle) * (dist - 25)), center_y + (int)(sin(angle) * (dist - 25)));
            }
        }
    }
    setlinestyle(PS_SOLID, 1); // 还原外部环境基础画笔参数
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