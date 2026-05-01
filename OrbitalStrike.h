#pragma once
#include <easyx.h>
#include <vector>

class Enemy;

// 轨道打击数据节点结构
struct OrbitalStrike
{
    double x, y;
    DWORD start_time;
    bool active = true;
    bool damaged = false;
};

// 轨道打击组件控制模块
class OrbitalStrikeSkill
{
public:
    int level = 0;
    DWORD last_strike_time = 0;
    std::vector<OrbitalStrike> strikes_list;

    // 强化模块指令分发
    void Upgrade();

    // 应用于物理空间结算帧流更新
    void Update(std::vector<Enemy*>& enemy_list, int player_atk, POINT player_pos);

    // 渲染管线图元渲染推送
    void Draw(bool is_paused, DWORD pause_start_time);

    // 回收资源与内存
    void Reset();

    // 时间挂起补偿接口
    void AddPauseTime(DWORD pause_duration);
};