#include "common.h"

// 程序运行状态标识，控制主循环
bool running = true;
// 游戏是否已开始的标识
bool is_game_started = false;
bool is_game_over = false;

// 绘制带透明通道的图片
void putimage_alpha(int x, int y, IMAGE* img)
{
    int w = img->getwidth();
    int h = img->getheight();
    // 调用系统API实现图片的透明混合绘制
    AlphaBlend(GetImageHDC(NULL), x, y, w, h, GetImageHDC(img), 0, 0, w, h, { AC_SRC_OVER,0,255,AC_SRC_ALPHA });
}

