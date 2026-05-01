#pragma once
#include <iostream>
#include <easyx.h>
#include <vector>
#include "Atlas.h"
using namespace std;

// 动画管理类：基于连续图集实现帧动画播放机制
class Animation
{
public:
    // 构造函数参数：目标图集指针, 帧间隔时长(ms)
    Animation(Atlas* atlas, int interval);
    ~Animation();

    // 执行渲染逻辑：x/y 为目标屏幕坐标，delta 为距上一帧的时间增量
    void Play(int x, int y, int delta);

private:
    int timer = 0;          // 内部计时器
    int idx_frame = 0;      // 当前渲染帧索引
    int interval_ms = 0;    // 设定的帧切换间隔阈值
    Atlas* anim_atlas;      // 关联的图集指针
};
