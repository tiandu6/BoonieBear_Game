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
// 【修改】：将玩家的图集拆分为三个角色的专属图集
extern Atlas* atlas_xiongda_left;
extern Atlas* atlas_xiongda_right;
extern Atlas* atlas_xionger_left;
extern Atlas* atlas_xionger_right;
extern Atlas* atlas_qiang_left;
extern Atlas* atlas_qiang_right;

extern Atlas* atlas_enemy_left;
extern Atlas* atlas_enemy_right;
extern Atlas* atlas_machine_left;
extern Atlas* atlas_machine_right;
extern Atlas* atlas_boss_left;
extern Atlas* atlas_boss_right;

// 【新增】：定义角色类型枚举
enum class CharacterType
{
    XiongDa = 0,
    XiongEr,
    GuangtouQiang
};
extern CharacterType selected_character; // 当前选中的角色

// 【新增】：定义游戏难度枚举
enum class Difficulty
{
    Easy = 0,
    Normal,
    Hard
};
extern Difficulty current_difficulty; // 当前选中的难度

// ====================== = 新增存档系统 ====================== =
// 存档系统全局变量和函数声明
extern int high_score;         // 历史最高分
extern int high_survival_time; // 历史最长存活时间（秒）

void LoadGameData();           // 读取本地存档
void SaveGameData(int current_score, int current_time); // 比较并保存存档