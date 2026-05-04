#include "common.h"
#include <fstream>

// 初始化 1P 默认角色为熊二，2P 默认角色为光头强
CharacterType selected_character = CharacterType::XiongEr;
CharacterType selected_character_p2 = CharacterType::GuangtouQiang;

// 初始化系统音量 50%
int current_volume = 500;
IMAGE img_bullet_left;
IMAGE img_bullet_right;

// 默认游戏难度与游戏模式
Difficulty current_difficulty = Difficulty::Normal;
int game_mode = 1;

// 初始化全局内存资源指针为空，防止野指针崩溃
Atlas* atlas_xiongda_left = nullptr;
Atlas* atlas_xiongda_right = nullptr;
Atlas* atlas_xionger_left = nullptr;
Atlas* atlas_xionger_right = nullptr;
Atlas* atlas_qiang_left = nullptr;
Atlas* atlas_qiang_right = nullptr;

Atlas* atlas_enemy_left = nullptr;
Atlas* atlas_enemy_right = nullptr;
Atlas* atlas_machine_left = nullptr;
Atlas* atlas_machine_right = nullptr;
Atlas* atlas_boss_left = nullptr;
Atlas* atlas_boss_right = nullptr;

// 初始化状态机
bool running = true;
bool is_game_started = false;
bool is_game_over = false;

// 核心图形接口：利用 Windows GDI 底层 API 实现带 Alpha 通道的高性能透明图像绘制
void putimage_alpha(int x, int y, IMAGE* img)
{
    int w = img->getwidth();
    int h = img->getheight();
    // AC_SRC_OVER 指示源图像覆盖目标图像，AC_SRC_ALPHA 指示使用源图像自身的 Alpha 通道参与混合运算
    AlphaBlend(GetImageHDC(NULL), x, y, w, h, GetImageHDC(img), 0, 0, w, h, { AC_SRC_OVER,0,255,AC_SRC_ALPHA });
}

// 存档数据内存映射
int high_score = 0;
int high_survival_time = 0;

// 初始化全局阴影图像实体
IMAGE img_shadow_player;
IMAGE img_shadow_normal;
IMAGE img_shadow_machine;
IMAGE img_shadow_boss;

// IO 操作：从本地磁盘加载二进制存档
void LoadGameData()
{
    std::ifstream ifs("save.dat");
    if (ifs.is_open())
    {
        ifs >> high_score >> high_survival_time;
        ifs.close();
    }
}

// IO 操作：比较当前战绩与历史记录，若打破记录则覆盖写入本地磁盘
void SaveGameData(int current_score, int current_time)
{
    bool updated = false;
    // 突破记录校验
    if (current_score > high_score) {
        high_score = current_score;
        updated = true;
    }
    if (current_time > high_survival_time) {
        high_survival_time = current_time;
        updated = true;
    }

    // 固化落地
    std::ofstream ofs("save.dat");
    if (ofs.is_open())
    {
        ofs << high_score << " " << high_survival_time;
        ofs.close();
    }
}