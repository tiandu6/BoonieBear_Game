#pragma once
#include <easyx.h>
#include "Atlas.h"

// 绘制带透明通道的图片（封装AlphaBlend）
void putimage_alpha(int x, int y, IMAGE* img);

// 链接系统库（透明绘制、音频播放）
#pragma comment(lib,"MSIMG32.LIB")
#pragma comment(lib,"Winmm.lib")

// 窗口尺寸常量
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

// 按钮默认尺寸
const int BUTTON_WIDTH = 240;
const int BUTTON_HEIGHT = 90;

// 全局状态变量（程序运行/游戏开始）
extern bool running;
extern bool is_game_started;
extern bool is_game_over; //游戏结束状态标识

// 全局图集指针（玩家/敌人动画）
extern Atlas* atlas_player_left;
extern Atlas* atlas_player_right;
extern Atlas* atlas_enemy_left;
extern Atlas* atlas_enemy_right;