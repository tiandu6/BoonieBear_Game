#include"Animation.h"
#include"common.h"

// 初始化动画：绑定图集并设置帧间隔
Animation::Animation(Atlas* atlas, int interval)
{
    anim_atlas = atlas;
    interval_ms = interval;
}

Animation::~Animation() = default;

// 播放动画逻辑
void Animation::Play(int x, int y, int delta)
{
    // 累计帧间隔时间
    timer += delta;
    // 达到帧间隔则切换下一帧
    if (timer >= interval_ms)
    {
        idx_frame = (idx_frame + 1) % anim_atlas->frame_list.size();
        timer = 0;
    }

    // 绘制当前帧（带透明）
    putimage_alpha(x, y, anim_atlas->frame_list[idx_frame]);
}