#include "common.h"
#include <fstream>

CharacterType selected_character = CharacterType::XiongEr; // 默认选熊二

// 【新增】：默认选择普通难度
Difficulty current_difficulty = Difficulty::Normal;

Atlas* atlas_xiongda_left = nullptr;
Atlas* atlas_xiongda_right = nullptr;
Atlas* atlas_xionger_left = nullptr;
Atlas* atlas_xionger_right = nullptr;
Atlas* atlas_qiang_left = nullptr;
Atlas* atlas_qiang_right = nullptr;

// 在文件开头附近，加上这四行：
Atlas* atlas_machine_left = nullptr;
Atlas* atlas_machine_right = nullptr;
Atlas* atlas_boss_left = nullptr;
Atlas* atlas_boss_right = nullptr;

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

// ======================= 新增存档系统实现 =======================
int high_score = 0;
int high_survival_time = 0;

// 读取本地存档文件
void LoadGameData()
{
    std::ifstream ifs("save.dat"); // 尝试打开游戏目录下的 save.dat
    if (ifs.is_open())
    {
        // 如果文件存在，直接读取里面的最高分和存活时间
        ifs >> high_score >> high_survival_time;
        ifs.close();
    }
    // 如果文件不存在（第一次玩），默认就是 0，不需要处理
}

// 游戏结束时调用，比较当前成绩并保存
void SaveGameData(int current_score, int current_time)
{
    bool updated = false; // 是否打破了记录

    if (current_score > high_score) {
        high_score = current_score;
        updated = true;
    }
    if (current_time > high_survival_time) {
        high_survival_time = current_time;
        updated = true;
    }

    // 只要玩了一局有成绩，就覆写更新存档文件
    std::ofstream ofs("save.dat");
    if (ofs.is_open())
    {
        ofs << high_score << " " << high_survival_time;
        ofs.close();
    }
}