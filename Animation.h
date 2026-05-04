#pragma once
#include <iostream>
#include <easyx.h>
#include <vector>
#include "Atlas.h"
using namespace std;

// 动画状态机管理类：基于连续图集实现帧动画的定时切帧与播放机制
class Animation
{
public:
    // 构造函数：注入目标图集指针，并设定帧间隔时长(ms)
    Animation(Atlas* atlas, int interval);
    ~Animation();

    // 核心渲染流：x/y 为目标屏幕绝对坐标，delta 为距上一帧的时间增量(dt)
    void Play(int x, int y, int delta);

private:
    int timer = 0;          // 内部时钟：记录距上一次切帧累计的差量时间
    int idx_frame = 0;      // 游标索引：当前正在渲染的图集帧序号
    int interval_ms = 0;    // 刷新阈值：设定的帧切换间隔时长
    Atlas* anim_atlas;      // 资源挂载：关联的底层图集资源指针
};