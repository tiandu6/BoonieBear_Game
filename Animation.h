#pragma once
#include <iostream>
#include <easyx.h>
#include <vector>
#include "Atlas.h"
using namespace std;

// 动画播放类，基于图集实现帧动画
class Animation
{
public:
    // 构造函数：传入图集、帧间隔（ms）
    Animation(Atlas* atlas, int interval);
    ~Animation();
    // 播放动画：x/y为绘制坐标，delta为距上一帧的时间差
    void Play(int x, int y, int delta);

private:
    int timer = 0;          // 帧计时
    int idx_frame = 0;      // 当前播放的帧索引
    int interval_ms = 0;    // 帧间隔（毫秒）
    Atlas* anim_atlas;      // 绑定的图集
};
