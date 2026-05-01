#include "Animation.h"
#include "common.h"

// 构造函数：绑定图集对象并设置帧切换间隔时间
Animation::Animation(Atlas* atlas, int interval)
{
    anim_atlas = atlas;
    interval_ms = interval;
}

Animation::~Animation() = default;

// 播放并渲染动画帧
void Animation::Play(int x, int y, int delta)
{
    // 累加时间差
    timer += delta;

    // 校验是否到达帧切换间隔
    if (timer >= interval_ms)
    {
        idx_frame = (idx_frame + 1) % anim_atlas->frame_list.size();
        timer = 0;
    }

    // 在指定坐标渲染当前图集帧 (支持 Alpha 透明通道)
    putimage_alpha(x, y, anim_atlas->frame_list[idx_frame]);
}