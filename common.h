#pragma once
#include <easyx.h>
#include "Atlas.h"

// 声明全局透明通道混色绘图接口
void putimage_alpha(int x, int y, IMAGE* img);

// 链接底层依赖库：处理透明度混合与多媒体音频播放
#pragma comment(lib,"MSIMG32.LIB")
#pragma comment(lib,"Winmm.lib")

// 全局常量：定义游戏视窗基础分辨率
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

// 全局常量：定义标准 UI 按钮尺寸
const int BUTTON_WIDTH = 240;
const int BUTTON_HEIGHT = 90;

// ================= 全局状态机与控制阀 =================
extern bool running;           // 控制主程序的生命周期（设为 false 则直接退出进程）
extern bool is_game_started;   // 标识当前是否已从主菜单进入战斗场景
extern bool is_game_over;      // 标识当前战局是否已经结束（通关或团灭）

// 游戏模式标识：1 为单机，2 为双人合作
extern int game_mode;

// ================= 全局资源句柄：动画图集 =================
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

// 角色类型枚举定义
enum class CharacterType
{
    XiongDa = 0,
    XiongEr,
    GuangtouQiang
};
extern CharacterType selected_character;     // 1P 角色
extern CharacterType selected_character_p2;  // 2P 角色

// 难度类型枚举定义
enum class Difficulty
{
    Easy = 0,
    Normal,
    Hard
};
extern Difficulty current_difficulty;        // 当前环境难度指针

// ================= 全局系统参数与持久化数据 =================
extern int current_volume;           // 系统音量 (0~1000)
extern IMAGE img_bullet_left;        // 预处理后的左向子弹材质
extern IMAGE img_bullet_right;       // 原始右向子弹材质

extern int high_score;               // 历史最高击杀记录（存档）
extern int high_survival_time;       // 历史最高存活时间（存档）

// 全局阴影资源（预加载到显存）
extern IMAGE img_shadow_player;
extern IMAGE img_shadow_normal;
extern IMAGE img_shadow_machine;
extern IMAGE img_shadow_boss;

// 持久化 IO 接口声明
void LoadGameData();
void SaveGameData(int current_score, int current_time);