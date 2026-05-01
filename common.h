#pragma once
#include <easyx.h>
#include "Atlas.h"

// 像素合成渲染接口 (封装 GDI AlphaBlend)
void putimage_alpha(int x, int y, IMAGE* img);

// 配置系统底层依赖库链接
#pragma comment(lib,"MSIMG32.LIB")
#pragma comment(lib,"Winmm.lib")

// 定义渲染窗口视口尺寸边界
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

// 定义基础交互 UI 控件尺寸映射
const int BUTTON_WIDTH = 240;
const int BUTTON_HEIGHT = 90;

// 声明跨文件的全局生命周期与状态机控制变量
extern bool running;
extern bool is_game_started;
extern bool is_game_over;

// 声明全局玩家实体图集指针数组 
extern Atlas* atlas_xiongda_left;
extern Atlas* atlas_xiongda_right;
extern Atlas* atlas_xionger_left;
extern Atlas* atlas_xionger_right;
extern Atlas* atlas_qiang_left;
extern Atlas* atlas_qiang_right;

// 声明全局敌对实体图集指针数组
extern Atlas* atlas_enemy_left;
extern Atlas* atlas_enemy_right;
extern Atlas* atlas_machine_left;
extern Atlas* atlas_machine_right;
extern Atlas* atlas_boss_left;
extern Atlas* atlas_boss_right;

// 定义玩家角色类别的强类型枚举
enum class CharacterType
{
    XiongDa = 0,
    XiongEr,
    GuangtouQiang
};
extern CharacterType selected_character;

// 定义动态难度系统的调节系数枚举
enum class Difficulty
{
    Easy = 0,
    Normal,
    Hard
};
extern Difficulty current_difficulty;

// 声明系统级音量与特效图形资源池变量
extern int current_volume;
extern IMAGE img_bullet_left;
extern IMAGE img_bullet_right;

// 声明本地存档模块的数据容器与驱动方法
extern int high_score;
extern int high_survival_time;

void LoadGameData();
void SaveGameData(int current_score, int current_time);