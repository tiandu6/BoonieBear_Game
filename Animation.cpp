#include "Animation.h"
#include "common.h"

// 构造与资源绑定：初始化图集指针与帧间隔时间
Animation::Animation(Atlas* atlas, int interval)
{
    anim_atlas = atlas;
    interval_ms = interval;
}

Animation::~Animation() = default;

// 播放并渲染动画帧，采用增量时间驱动 (Delta-Time Driven)
void Animation::Play(int x, int y, int delta)
{
    // 将主循环传入的帧间差量时间累加至内部时钟
    timer += delta;

    // 校验是否满足切帧条件（达到或超过设定的毫秒阈值）
    if (timer >= interval_ms)
    {
        // 游标递增，并利用取模运算实现动画列表的无缝循环轮播
        idx_frame = (idx_frame + 1) % anim_atlas->frame_list.size();
        timer = 0; // 重置内部时钟准备下一轮计时
    }

    // 调用公共模块底层的 Alpha 通道混色渲染接口，将当前帧绘制至指定坐标
    putimage_alpha(x, y, anim_atlas->frame_list[idx_frame]);
}