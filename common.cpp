#include "common.h"
#include <fstream>

CharacterType selected_character = CharacterType::XiongEr; // 默认初始化角色类型为熊二

// 全局音量设置及射击特效资源实例初始化
int current_volume = 500;
IMAGE img_bullet_left;
IMAGE img_bullet_right;

// 设置系统默认加载的难度配置级别为 Normal
Difficulty current_difficulty = Difficulty::Normal;

// 全局玩家图集资源指针注册
Atlas* atlas_xiongda_left = nullptr;
Atlas* atlas_xiongda_right = nullptr;
Atlas* atlas_xionger_left = nullptr;
Atlas* atlas_xionger_right = nullptr;
Atlas* atlas_qiang_left = nullptr;
Atlas* atlas_qiang_right = nullptr;

// 全局敌对实体图集资源指针注册
Atlas* atlas_machine_left = nullptr;
Atlas* atlas_machine_right = nullptr;
Atlas* atlas_boss_left = nullptr;
Atlas* atlas_boss_right = nullptr;

// 定义主运行循环生命周期状态锁
bool running = true;
// 定义场景及交互模式切换的状态机标识
bool is_game_started = false;
bool is_game_over = false;

// 像素合成渲染接口 (封装 GDI AlphaBlend)
void putimage_alpha(int x, int y, IMAGE* img)
{
    int w = img->getwidth();
    int h = img->getheight();
    // 执行附带 Alpha 通道映射的纹理混合绘制
    AlphaBlend(GetImageHDC(NULL), x, y, w, h, GetImageHDC(img), 0, 0, w, h, { AC_SRC_OVER,0,255,AC_SRC_ALPHA });
}

// 存档数据结构内存分配
int high_score = 0;
int high_survival_time = 0;

// 反序列化读取本地持久化数据记录
void LoadGameData()
{
    std::ifstream ifs("save.dat");
    if (ifs.is_open())
    {
        // 提取最高纪录的分数与存活时长数据元
        ifs >> high_score >> high_survival_time;
        ifs.close();
    }
}

// 序列化覆盖本地存储的记录快照
void SaveGameData(int current_score, int current_time)
{
    bool updated = false; // 高分突破标记位

    if (current_score > high_score) {
        high_score = current_score;
        updated = true;
    }
    if (current_time > high_survival_time) {
        high_survival_time = current_time;
        updated = true;
    }

    // 更新磁盘记录并安全关闭流
    std::ofstream ofs("save.dat");
    if (ofs.is_open())
    {
        ofs << high_score << " " << high_survival_time;
        ofs.close();
    }
}