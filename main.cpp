#include <easyx.h>
#include <string>
#include "Animation.h"
#include "common.h"
#include "Player.h"
#include "Enemy.h"
#include "Bullet.h"
#include "Atlas.h"
#include "Button.h"
#include "StartGameButton.h"
#include "QuitGameButton.h"
#include <vector>
using namespace std;

// 玩家动画帧数量
const int PLAYER_ANIM_NUM = 6;

// 全局图集指针（玩家/敌人左右方向）
Atlas* atlas_player_left = nullptr;
Atlas* atlas_player_right = nullptr;
Atlas* atlas_enemy_left = nullptr;
Atlas* atlas_enemy_right = nullptr;

// 尝试生成敌人（按固定间隔随机生成）
void TryGenerateEnemy(vector<Enemy*>& enemy_list)
{
    const int INTERVAL = 100;
    static int counter = 0;
    // 每累计100帧生成一个敌人
    if ((++counter) % INTERVAL == 0)
        enemy_list.push_back(new Enemy());
}

// 更新子弹位置（环绕玩家旋转）
void UpdateBullets(vector<Bullet>& bullet_list, const Player& player)
{
    const double RADIAL_SPEED = 0.0045;   // 子弹轨道半径变化速度
    const double TANGENT_SPEED = 0.0045;  // 子弹绕玩家旋转速度
    // 子弹均匀分布在圆周上的角度间隔
    double radian_interval = 2 * 3.1415926 / bullet_list.size();
    POINT player_pos = player.GetPosition();
    // 计算轨道半径（随时间正弦变化）
    double radius = 100 + 25 * sin(GetTickCount() * RADIAL_SPEED);

    for (size_t i = 0; i < bullet_list.size(); i++)
    {
        // 计算当前子弹的角度
        double radian = GetTickCount() * TANGENT_SPEED + radian_interval * i;
        // 更新子弹坐标（绕玩家旋转）
        bullet_list[i].pos.x = player_pos.x + player.FRAME_WIDTH / 2 + (int)(radius * cos(radian));
        bullet_list[i].pos.y = player_pos.y + player.FRAME_HEIGHT / 2 + (int)(radius * sin(radian));
    }
}

// 绘制玩家当前分数
void DrawPlayerScore(int score)
{
    static TCHAR text[64];
    _stprintf_s(text, _T("当前蜂蜜分数：%d"), score); 

    // 设置字体：高度32，粗体，微软雅黑
    settextstyle(32, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
    setbkmode(TRANSPARENT);

    // 1. 绘制文字阴影（深灰色，向右下偏移2像素）
    settextcolor(RGB(50, 50, 50));
    outtextxy(17, 17, text);

    // 2. 绘制主文字（蜂蜜金色）
    settextcolor(RGB(255, 180, 20));
    outtextxy(15, 15, text);
}

int main()
{
    // 初始化绘图窗口（1280x720）
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);

    // 加载玩家/敌人动画图集（指定帧尺寸）
    atlas_player_left = new Atlas(_T("img/XiongEr_left_%d.png"), 8, 150, 150);
    atlas_player_right = new Atlas(_T("img/XiongEr_right_%d.png"), 8, 150, 150);
    atlas_enemy_left = new Atlas(_T("img/LoggingWorker_left_%d.png"), 8, 80, 80);
    atlas_enemy_right = new Atlas(_T("img/LoggingWorker_right_%d.png"), 8, 80, 80);

    // 打开背景音乐、击中音效（设置别名）
    mciSendString(_T("open mus/bgm.mp3 alias bgm"), NULL, 0, NULL);
    mciSendString(_T("open mus/hit.mp3 alias hit"), NULL, 0, NULL);

    // 游戏数据初始化
    int score = 0;               // 玩家分数
    Player player;               // 玩家实例
    ExMessage msg;               // 消息缓冲区
    IMAGE img_menu;              // 游戏菜单背景
    IMAGE img_background;        // 游戏场景背景
    vector<Enemy*> enemy_list;   // 敌人列表
    vector<Bullet> bullet_list(3);// 子弹列表（初始3个）

    // 按钮区域初始化（开始/退出按钮）
    RECT region_btn_start_game, region_btn_quit_game;
    // 开始游戏按钮：居中显示，y坐标430
    region_btn_start_game.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2;
    region_btn_start_game.right = region_btn_start_game.left + BUTTON_WIDTH;
    region_btn_start_game.top = 430;
    region_btn_start_game.bottom = region_btn_start_game.top + BUTTON_HEIGHT;

    // 退出游戏按钮：居中显示，y坐标550
    region_btn_quit_game.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2;
    region_btn_quit_game.right = region_btn_quit_game.left + BUTTON_WIDTH;
    region_btn_quit_game.top = 550;
    region_btn_quit_game.bottom = region_btn_quit_game.top + BUTTON_HEIGHT;

    // 创建开始/退出按钮实例
    StartGameButton btn_start_game = StartGameButton(region_btn_start_game, _T("img/ui_start_idle.png"),
        _T("img/ui_start_hovered.png"), _T("img/ui_start_pushed.png"));
    QuitGameButton btn_quit_game = QuitGameButton(region_btn_quit_game, _T("img/ui_quit_idle.png"),
        _T("img/ui_quit_hovered.png"), _T("img/ui_quit_pushed.png"));

    // 加载菜单、游戏背景图片
    loadimage(&img_menu, _T("img/menu.png"));
    loadimage(&img_background, _T("img/background.png"));

    // 开启双缓冲绘图（防止画面闪烁）
    BeginBatchDraw();

    // 游戏主循环
    while (running)
    {
        // 记录当前帧开始时间（用于控制帧率）
        DWORD start_time = GetTickCount();

        // 处理所有待处理的消息
        while (peekmessage(&msg))
        {
            // 游戏开始前处理按钮事件，开始后处理玩家输入
            if (is_game_started)
                player.ProcessEvent(msg);
            else
            {
                btn_start_game.ProcessEvent(msg);
                btn_quit_game.ProcessEvent(msg);
            }
        }

        // ================= 游戏运行逻辑（开始后执行） =================
        if (is_game_started && !is_game_over) 
        {
            // 更新玩家位置
            player.Move();
            // 更新子弹位置
            UpdateBullets(bullet_list, player);
            // 尝试生成新敌人
            TryGenerateEnemy(enemy_list);
            // 所有敌人朝向玩家移动
            for (Enemy* enemy : enemy_list)
                enemy->Move(player);

            // 检测玩家与敌人的碰撞
            for (Enemy* enemy : enemy_list)
            {
                if (enemy->CheckPlayerCollision(player))
                {
                    is_game_over = true;
                    break;
                }
            }

            // 检测子弹与敌人的碰撞（击中则加分、播放音效）
            for (Enemy* enemy : enemy_list)
            {
                for (const Bullet& bullet : bullet_list)
                {
                    if (enemy->CheckBulletCollision(bullet))
                    {
                        mciSendString(_T("play hit from 0"), NULL, 0, NULL);
                        enemy->Hurt();
                        score++;
                    }
                }
            }

            // 清理死亡的敌人（释放内存）
            for (size_t i = 0; i < enemy_list.size(); i++)
            {
                Enemy* enemy = enemy_list[i];
                if (!enemy->CheckAlive())
                {
                    swap(enemy_list[i], enemy_list.back());
                    enemy_list.pop_back();
                    delete enemy;
                }
            }
        }
        else if (is_game_over)
        {
            // 游戏结束状态下，按 ESC 键退出主循环
            if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
            {
                running = false;
            }
        }

        // ================= 画面绘制逻辑 =================
        cleardevice();  // 清空画布
        if (is_game_started)
        {
            // 绘制游戏背景
            putimage(0, 0, &img_background);
            // 绘制玩家（固定144帧/秒的delta）
            player.Draw(1000 / 144);
            // 绘制所有敌人
            for (Enemy* enemy : enemy_list)
                enemy->Draw(1000 / 144);
            // 绘制所有子弹
            for (const Bullet& bullet : bullet_list)
                bullet.Draw();
            // 绘制分数 
            DrawPlayerScore(score);

            // 这是第四步新增的：如果游戏结束，叠加绘制结算面板
            if (is_game_over)
            {
                // 绘制一个半透明的黑色全屏遮罩 
                setlinecolor(RGB(0, 0, 0));
                setfillcolor(RGB(40, 40, 40));

                int panel_width = 400;
                int panel_height = 250;
                int px = (WINDOW_WIDTH - panel_width) / 2;
                int py = (WINDOW_HEIGHT - panel_height) / 2;
                fillroundrect(px, py, px + panel_width, py + panel_height, 20, 20);

                // 绘制“游戏结束”大标题
                settextstyle(48, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
                settextcolor(RGB(255, 80, 80)); // 红色警告
                LPCTSTR over_text = _T("游 戏 结 束");
                int tx = px + (panel_width - textwidth(over_text)) / 2;
                outtextxy(tx, py + 40, over_text);

                // 绘制最终得分
                TCHAR final_score_text[64];
                _stprintf_s(final_score_text, _T("最终获取蜂蜜：%d 罐"), score);
                settextstyle(28, 0, _T("微软雅黑"), 0, 0, FW_NORMAL, false, false, false);
                settextcolor(RGB(255, 200, 50));
                int sx = px + (panel_width - textwidth(final_score_text)) / 2;
                outtextxy(sx, py + 120, final_score_text);

                // 绘制操作提示
                settextstyle(20, 0, _T("黑体"), 0, 0, FW_NORMAL, false, false, false);
                settextcolor(RGB(180, 180, 180));
                LPCTSTR hint_text = _T("按下 ESC 键退出游戏");
                int hx = px + (panel_width - textwidth(hint_text)) / 2;
                outtextxy(hx, py + 190, hint_text);
            }
        }
        else
        {
            // 绘制菜单背景+按钮
            putimage(0, 0, &img_menu);
            btn_start_game.Draw();
            btn_quit_game.Draw();
        }

        // 刷新双缓冲（显示绘制内容）
        FlushBatchDraw();

        // 控制帧率为144帧/秒
        DWORD end_time = GetTickCount();
        DWORD delta_time = end_time - start_time;
        if (delta_time < 1000 / 144)
        {
            Sleep(1000 / 144 - delta_time);
        }
    }

    // 释放图集资源
    delete atlas_player_left;
    delete atlas_player_right;
    delete atlas_enemy_left;
    delete atlas_enemy_right;

    // 关闭双缓冲
    EndBatchDraw();
    return 0;
}