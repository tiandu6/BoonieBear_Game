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
#include "Characters.h" 
#include "Enemies.h"
using namespace std;

// 玩家动画帧数量
const int PLAYER_ANIM_NUM = 6;

// 全局图集指针（玩家/敌人左右方向）
Atlas* atlas_player_left = nullptr;
Atlas* atlas_player_right = nullptr;
Atlas* atlas_enemy_left = nullptr;
Atlas* atlas_enemy_right = nullptr;

// 尝试生成敌人（按阶段划分）
void TryGenerateEnemy(vector<Enemy*>& enemy_list, int current_phase, bool& boss_spawned)
{
    int interval = 100;
    size_t max_enemies = 10;
    if (current_difficulty == Difficulty::Easy) { interval = 150; max_enemies = 5; }
    else if (current_difficulty == Difficulty::Normal) { interval = 100; max_enemies = 12; }
    else { interval = 40; max_enemies = 30; }

    // 第三阶段：只生成一次 Boss
    if (current_phase == 3 && !boss_spawned) {
        enemy_list.push_back(new BossEnemy());
        boss_spawned = true;
    }

    if (enemy_list.size() >= max_enemies) return;

    static int counter = 0;
    if ((++counter) % interval == 0)
    {
        // 第二阶段开始，30%概率出机器怪
        if (current_phase >= 2 && (rand() % 100 < 30)) {
            enemy_list.push_back(new MachineEnemy());
        }
        else {
            enemy_list.push_back(new NormalEnemy());
        }
    }
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

// 绘制玩家HUD（包含分数与具体血量信息）
void DrawHUD(int score, int hp, int max_hp, int survival_time_sec)//存活时间参数 survival_time_sec
{
    // ================= 1. 绘制蜂蜜分数 =================
    static TCHAR score_text[64];
    _stprintf_s(score_text, _T("当前分数：%d"), score);

    // 设置字体：高度32，粗体，微软雅黑
    settextstyle(32, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
    setbkmode(TRANSPARENT);

    // 文字阴影与主文字 
    settextcolor(RGB(50, 50, 50));
    outtextxy(17, 17, score_text);
    settextcolor(RGB(255, 180, 20));
    outtextxy(15, 15, score_text);

    // ================= 1.5 【新增】绘制存活时间 =================
    static TCHAR time_text[64];
    _stprintf_s(time_text, _T("存活时间：%d 秒"), survival_time_sec);
    settextstyle(24, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
    settextcolor(RGB(50, 50, 50));
    outtextxy(17, 57, time_text);
    settextcolor(RGB(150, 200, 255)); // 科技感浅蓝色
    outtextxy(15, 55, time_text);

    // ================= 2. 绘制左上角 UI 大血条 =================
    int bar_x = 15;         // 血条X坐标（对齐分数）
    int bar_y = 95;         // 血条Y坐标（在分数下方）
    int bar_width = 300;    // UI大血条比脚下血条长得多
    int bar_height = 28;    // 加粗血条

    // 绘制底槽（黑灰色，带圆角）
    setlinecolor(RGB(30, 30, 30));
    setfillcolor(RGB(50, 50, 50));
    fillroundrect(bar_x, bar_y, bar_x + bar_width, bar_y + bar_height, 8, 8);

    // 绘制当前血量条
    if (hp > 0)
    {
        int fill_width = (int)((double)hp / max_hp * bar_width);

        // 动态计算血量比例并变色
        COLORREF hp_color;
        double hp_ratio = (double)hp / max_hp;

        // 大于50%显示绿色，大于25%显示黄色，否则显示红色警告
        if (hp_ratio > 0.5)
            hp_color = RGB(50, 220, 50);   // 高血量：绿色
        else if (hp_ratio > 0.25)
            hp_color = RGB(255, 200, 50);  // 中血量：黄色
        else
            hp_color = RGB(255, 50, 50);   // 低血量：红色

        setfillcolor(hp_color);
        solidroundrect(bar_x + 1, bar_y + 1, bar_x + fill_width - 1, bar_y + bar_height - 1, 8, 8);
    }

    // ================= 3. 绘制具体血量文字 =================
    static TCHAR hp_text[64];
    _stprintf_s(hp_text, _T("HP: %d / %d"), hp, max_hp); // 格式化为：HP: 当前 / 满血

    // 使用英文无衬线字体，类似大部分现代游戏的UI风格
    settextstyle(20, 0, _T("Arial"), 0, 0, FW_BOLD, false, false, false);

    // 计算文字尺寸，使其在血条内绝对居中
    int text_w = textwidth(hp_text);
    int text_h = textheight(hp_text);
    int text_x = bar_x + (bar_width - text_w) / 2;
    int text_y = bar_y + (bar_height - text_h) / 2;

    // 绘制文字阴影，保证在红底或灰底上都清晰可见
    settextcolor(RGB(30, 30, 30));
    outtextxy(text_x + 2, text_y + 2, hp_text);
    // 绘制纯白文字
    settextcolor(RGB(255, 255, 255));
    outtextxy(text_x, text_y, hp_text);

    // ================= 4. 绘制右上角 UI 暂停按钮 =================
    int btn_w = 130;  // 按钮宽度
    int btn_h = 40;   // 按钮高度
    int btn_x = WINDOW_WIDTH - btn_w - 20; // 靠右对齐，留20像素边距
    int btn_y = 20;

    // 绘制按钮底框（带圆角的深灰色框）
    setlinecolor(RGB(50, 50, 50));
    setfillcolor(RGB(40, 40, 40));
    fillroundrect(btn_x, btn_y, btn_x + btn_w, btn_y + btn_h, 10, 10);

    // 绘制按钮文字提示
    settextstyle(18, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
    settextcolor(RGB(220, 220, 220));
    LPCTSTR btn_text = _T("|| 暂停 (P)");

    // 居中对齐文字
    int t_w = textwidth(btn_text);
    int t_h = textheight(btn_text);
    outtextxy(btn_x + (btn_w - t_w) / 2, btn_y + (btn_h - t_h) / 2, btn_text);
}

int main()
{
    // 初始化绘图窗口（1280x720）
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);

    // 加载玩家/敌人动画图集（指定帧尺寸）
   // 玩家：熊大
    atlas_xiongda_right = new Atlas(_T("img/XiongDa_right_%d.png"), 8, 150, 150, false);
    atlas_xiongda_left = new Atlas(_T("img/XiongDa_right_%d.png"), 8, 150, 150, true);
    // 玩家：熊二
    atlas_xionger_right = new Atlas(_T("img/XiongEr_right_%d.png"), 8, 150, 150, false);
    atlas_xionger_left = new Atlas(_T("img/XiongEr_right_%d.png"), 8, 150, 150, true);
    // 玩家：光头强
    atlas_qiang_right = new Atlas(_T("img/GuangtouQiang_right_%d.png"), 8, 150, 150, false);
    atlas_qiang_left = new Atlas(_T("img/GuangtouQiang_right_%d.png"), 8, 150, 150, true);

    // 普通伐木工
    atlas_enemy_right = new Atlas(_T("img/LoggingWorker_right_%d.png"), 8, 80, 80, false);
    atlas_enemy_left = new Atlas(_T("img/LoggingWorker_right_%d.png"), 8, 80, 80, true);
    // 机器伐木工
    atlas_machine_right = new Atlas(_T("img/MechanicalLoggingWorker_right_%d.png"), 8, 120, 120, false);
    atlas_machine_left = new Atlas(_T("img/MechanicalLoggingWorker_right_%d.png"), 8, 120, 120, true);
    // boss 
    atlas_boss_right = new Atlas(_T("img/Boss_right_%d.png"), 8, 200, 200, false);
    atlas_boss_left = new Atlas(_T("img/Boss_right_%d.png"), 8, 200, 200, true);

    // 打开背景音乐、击中音效（设置别名）
    mciSendString(_T("open mus/bgm.mp3 alias bgm"), NULL, 0, NULL);
    mciSendString(_T("open mus/hit.mp3 alias hit"), NULL, 0, NULL);

    // 游戏数据初始化
    int score = 0;               // 玩家分数
    Player* player = new XiongEr(); // 使用基类指针指向派生类
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

    bool is_game_paused = false; // 游戏暂停状态标识
    static DWORD game_over_time = 0; // 记录玩家死亡瞬间的时间

    LoadGameData(); // 游戏启动时读取本地最高分记录
    int survival_time_ms = 0; // 记录本局当前存活的毫秒数

    // 阶段与胜利控制器
    int current_phase = 0;           // 当前阶段 (1~3)
    DWORD phase_announce_time = 0;   // 记录阶段提示出现的时间
    bool is_game_won = false;        // 是否通关
    bool boss_spawned = false;       // Boss是否已生成

    // 游戏主循环
    while (running)
    {
        DWORD start_time = GetTickCount();

        // 处理所有待处理的消息
        // 处理所有待处理的消息
        // 处理所有待处理的消息
        while (peekmessage(&msg))
        {
            // 按下 P 键切换暂停状态
            if (msg.message == WM_KEYDOWN && msg.vkcode == 'P')
            {
                if (is_game_started && !is_game_over)
                {
                    is_game_paused = !is_game_paused;
                }
            }

            // 【新增】：在主菜单按 TAB 键无缝切换角色
            if (msg.message == WM_KEYDOWN && msg.vkcode == VK_TAB)
            {
                if (!is_game_started) // 只有在游戏开始前能换人
                {
                    // 记录旧角色的坐标，实现无缝传承
                    POINT old_pos = player->GetPosition();

                    // 利用虚析构函数安全释放旧对象，防止内存泄漏
                    delete player;

                    // 角色在 0, 1, 2 之间轮流切换
                    int next_char = ((int)selected_character + 1) % 3;
                    selected_character = (CharacterType)next_char;

                    // 核心多态展现：使用基类指针构造不同的子类对象
                    if (selected_character == CharacterType::XiongDa)
                        player = new XiongDa();
                    else if (selected_character == CharacterType::XiongEr)
                        player = new XiongEr();
                    else
                        player = new GuangtouQiang();

                    // 把新对象放到旧对象的坐标上，实现视觉上的无缝替换
                    player->SetPosition(old_pos);
                }
            }

            // 检测鼠标左键点击右上角的暂停按钮
            if (msg.message == WM_LBUTTONDOWN)
            {
                if (is_game_started && !is_game_over)
                {
                    int btn_x = WINDOW_WIDTH - 130 - 20;
                    int btn_y = 20;
                    if (msg.x >= btn_x && msg.x <= btn_x + 130 && msg.y >= btn_y && msg.y <= btn_y + 40)
                    {
                        is_game_paused = !is_game_paused;
                    }
                }

                else if (!is_game_started)
                {
                    // 原本的开始/退出按钮逻辑
                    btn_start_game.ProcessEvent(msg);
                    btn_quit_game.ProcessEvent(msg);

                    // 【新增】：检测主菜单右上角的“难度选择”按钮点击
                    int diff_btn_w = 160;
                    int diff_btn_h = 50;
                    int diff_btn_x = WINDOW_WIDTH - diff_btn_w - 40;
                    int diff_btn_y = 40;

                    if (msg.x >= diff_btn_x && msg.x <= diff_btn_x + diff_btn_w &&
                        msg.y >= diff_btn_y && msg.y <= diff_btn_y + diff_btn_h)
                    {
                        // 循环切换难度：简单 -> 普通 -> 困难
                        int next_diff = ((int)current_difficulty + 1) % 3;
                        current_difficulty = (Difficulty)next_diff;
                    }
                }
            }

            // 游戏开始前处理菜单按钮事件，开始后处理玩家输入
            if (is_game_started && !is_game_over)
            {
                player->ProcessEvent(msg); // 注意这里变成了 player->
            }
            else if (!is_game_started)
            {
                btn_start_game.ProcessEvent(msg);
                btn_quit_game.ProcessEvent(msg);
            }
        }

        // ================= 游戏运行逻辑（开始后执行） =================
        // 暂停时停止一切画面内元素的更新
        if (is_game_started && !is_game_over && !is_game_paused)
        {
            survival_time_ms += (1000 / 144);// 每帧累加存活毫秒数（因为锁了144帧）
            
            // 根据存活时间计算当前应该在哪个阶段 (这里设定 45秒进入二阶段，90秒进入三阶段出Boss)
            int target_phase = 1;
            if (survival_time_ms >= 10000) target_phase = 3;       // 90秒进入第三阶段 (Boss)
            else if (survival_time_ms >= 5000) target_phase = 2;  // 45秒进入第二阶段 (机器怪)

            // 如果阶段升级了，触发大字提示特效！
            if (target_phase > current_phase) {
                current_phase = target_phase;
                phase_announce_time = GetTickCount();
            }

            player->Move();
            UpdateBullets(bullet_list, *player);
            TryGenerateEnemy(enemy_list, current_phase, boss_spawned);

            size_t current_enemy_count = enemy_list.size();
            for (size_t i = 0; i < current_enemy_count; i++)
            {
                enemy_list[i]->Move(*player, enemy_list);
            }

            for (Enemy* enemy : enemy_list)
            {
                if (enemy->CheckPlayerCollision(*player))
                {
                    // 【新增】：不同难度下，敌人造成的伤害不同
                    int dmg = 15;
                    if (current_difficulty == Difficulty::Easy) dmg = 8;        // 简单：刮痧
                    else if (current_difficulty == Difficulty::Normal) dmg = 15;// 普通：正常
                    else if (current_difficulty == Difficulty::Hard) dmg = 35;  // 困难：摸两下光头强就直接暴毙！

                    player->TakeDamage(dmg);
                    if (player->GetHP() <= 0 && !is_game_over)
                    {
                        is_game_over = true;
                        game_over_time = GetTickCount();

                        // 玩家阵亡瞬间，对比并保存最高分记录到硬盘！
                        SaveGameData(score, survival_time_ms / 1000);
                    }
                }
            }

            for (Enemy* enemy : enemy_list)
            {
                for (const Bullet& bullet : bullet_list)
                {
                    if (enemy->CheckBulletCollision(bullet))
                    {
                        // 【修改】：使用 player->GetAttackDamage() 替代写死的 1
                        if (enemy->Hurt(player->GetAttackDamage(), bullet.pos))
                        {
                            mciSendString(_T("play hit from 0"), NULL, 0, NULL);
                            if (!enemy->CheckAlive()) score++;
                        }
                    }
                }
            }

            for (size_t i = 0; i < enemy_list.size(); i++)
            {
                Enemy* enemy = enemy_list[i];
                if (!enemy->CheckAlive())
                {
                    // 如果死掉的是 Boss，恭喜通关！
                    if (enemy->is_boss)
                    {
                        is_game_won = true;
                        is_game_over = true;
                        game_over_time = GetTickCount();
                        SaveGameData(score, survival_time_ms / 1000);
                    }

                    swap(enemy_list[i], enemy_list.back());
                    enemy_list.pop_back();
                    delete enemy;
                    i--; // 防止删完一个怪后漏检查换过来的那个怪
                }
            }
        }

        else if (is_game_over)
        {
            // 游戏结束状态下：按 ESC 键退出，按 R 键再来一局，按 M 键回主菜单
            if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
            {
                running = false;
            }
            if (GetAsyncKeyState('R') & 0x8000) // 再来一局逻辑
            {
                // 1. 恢复状态
                is_game_over = false;
                is_game_paused = false;
                score = 0;
                survival_time_ms = 0;
                // 2. 清空并释放现有的敌人
                for (Enemy* enemy : enemy_list) delete enemy;
                enemy_list.clear();
                // 3. 重置玩家与子弹
                player->Reset();
                bullet_list = vector<Bullet>(3);
                // 防止按键太快导致连触
                Sleep(150);

                // 重置：
                current_phase = 0;
                phase_announce_time = 0;
                is_game_won = false;
                boss_spawned = false;
            }
            // 【新增】：回到主菜单逻辑
            if (GetAsyncKeyState('M') & 0x8000)
            {
                // 1. 恢复并切换场景状态
                is_game_started = false; // 👈 核心：退回菜单场景
                is_game_over = false;
                is_game_paused = false;
                score = 0;
                survival_time_ms = 0;
                // 2. 清空并释放现有的敌人
                for (Enemy* enemy : enemy_list) delete enemy;
                enemy_list.clear();
                // 3. 重置玩家与子弹
                player->Reset();
                bullet_list = vector<Bullet>(3);
                // 防止按键连触
                Sleep(150);

                // 重置：
                current_phase = 0;
                phase_announce_time = 0;
                is_game_won = false;
                boss_spawned = false;
            }
        }

        // ================= 画面绘制逻辑 =================
        cleardevice();
        if (is_game_started)
        {
            putimage(0, 0, &img_background);

            // 如果暂停了，传递 delta = 0 停止播放动画帧，否则正常传递 1000/144
            int current_delta = is_game_paused ? 0 : (1000 / 144);

            player->Draw(current_delta);
            for (Enemy* enemy : enemy_list) enemy->Draw(current_delta);
            for (const Bullet& bullet : bullet_list) bullet.Draw();

            DrawHUD(score, player->GetHP(), player->GetMaxHP(), survival_time_ms / 1000);

            // 绘制震撼的阶段提示横幅特效 (持续 3.5 秒)
            if (current_phase > 0 && (GetTickCount() - phase_announce_time < 3500) && !is_game_over)
            {
                DWORD elapsed = GetTickCount() - phase_announce_time;
                settextstyle(60, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
                setbkmode(TRANSPARENT);

                LPCTSTR phase_text;
                if (current_phase == 1) phase_text = _T("第一阶段：森林保卫战");
                else if (current_phase == 2) phase_text = _T("第二阶段：机械危机");
                else phase_text = _T("最终阶段：迎战黑化光头强！");

                // 让颜色随时间疯狂闪烁跳动，极具感官刺激
                COLORREF main_color = (elapsed / 150 % 2 == 0) ? RGB(255, 200, 50) : RGB(255, 80, 80);
                if (current_phase == 1) main_color = RGB(100, 255, 150); // 第一阶段颜色温和一点

                int tw = textwidth(phase_text);
                int tx = (WINDOW_WIDTH - tw) / 2;
                int ty = WINDOW_HEIGHT / 4; // 放在偏上方

                settextcolor(RGB(20, 20, 20)); // 重阴影
                outtextxy(tx + 4, ty + 4, phase_text);
                settextcolor(main_color);      // 高亮频闪字
                outtextxy(tx, ty, phase_text);
            }

            // 绘制暂停 UI
            if (is_game_paused && !is_game_over)
            {
                setlinecolor(RGB(0, 0, 0));
                setfillcolor(RGB(20, 20, 20));
                fillroundrect(WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 60, WINDOW_WIDTH / 2 + 150, WINDOW_HEIGHT / 2 + 60, 15, 15);

                settextstyle(40, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
                settextcolor(RGB(255, 200, 50));
                LPCTSTR pause_text = _T("游 戏 暂 停");
                outtextxy((WINDOW_WIDTH - textwidth(pause_text)) / 2, WINDOW_HEIGHT / 2 - 35, pause_text);

                settextstyle(18, 0, _T("黑体"), 0, 0, FW_NORMAL, false, false, false);
                settextcolor(RGB(200, 200, 200));
                LPCTSTR hint_p = _T("- 再按 P 键恢复游戏 -");
                outtextxy((WINDOW_WIDTH - textwidth(hint_p)) / 2, WINDOW_HEIGHT / 2 + 20, hint_p);
            }

            if (is_game_over && (GetTickCount() - game_over_time > 2500))
            {
                setlinecolor(RGB(0, 0, 0));
                setfillcolor(RGB(40, 40, 40));
                int panel_width = 400;
                int panel_height = 250;
                int px = (WINDOW_WIDTH - panel_width) / 2;
                int py = (WINDOW_HEIGHT - panel_height) / 2;
                fillroundrect(px, py, px + panel_width, py + panel_height, 20, 20);

                settextstyle(48, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
                // 判断是胜利还是死亡，显示完全不同的文案和颜色
                LPCTSTR over_text;
                if (is_game_won) {
                    settextcolor(RGB(50, 255, 50)); // 胜利的亮绿色
                    over_text = _T("恭 喜 通 关 !");
                }
                else {
                    settextcolor(RGB(255, 80, 80)); // 阵亡的血红色
                    over_text = _T("游 戏 结 束");
                }
                outtextxy(px + (panel_width - textwidth(over_text)) / 2, py + 40, over_text);

                TCHAR final_score_text[64];
                _stprintf_s(final_score_text, _T("最终分数：%d 分 | 存活：%d 秒"), score, survival_time_ms / 1000);
                settextstyle(28, 0, _T("微软雅黑"), 0, 0, FW_NORMAL, false, false, false);
                settextcolor(RGB(255, 200, 50));
                outtextxy(px + (panel_width - textwidth(final_score_text)) / 2, py + 120, final_score_text);

                // 更新了底部的操作提示文案
                settextstyle(18, 0, _T("黑体"), 0, 0, FW_NORMAL, false, false, false);
                settextcolor(RGB(180, 180, 180));
                LPCTSTR hint_text = _T("[R] 再来一局  |  [M] 回主菜单  |  [ESC] 退出");
                outtextxy(px + (panel_width - textwidth(hint_text)) / 2, py + 190, hint_text);
            }
        }
        else
        {
            // 绘制菜单背景+按钮
            putimage(0, 0, &img_menu);
            btn_start_game.Draw();
            btn_quit_game.Draw();

            // ================= 选人展示台 UI (沉浸式极简风格) =================

            int preview_x = 140; // 预览模型 X 坐标
            int preview_y = 60;  // 预览模型 Y 坐标

            // 1. 绘制角色脚下的“全息展示底座” (双层扁平椭圆)
            // 整体向左下微调，修正视觉上的“偏右上”错觉
            int base_center_x = preview_x + 30; // 整体向左调
            int base_center_y = preview_y + 108; // 整体向下调，使其稳稳托住阴影

            // 【新增】：针对光头强的模型重心和特殊阴影偏移，单独进行向左修正
            if (selected_character == CharacterType::GuangtouQiang)
            {
                base_center_x -= 20; // 光头强专属向左额外偏移 20 像素
            }

            setlinecolor(RGB(255, 200, 50)); // 外圈高亮金色
            setlinestyle(PS_SOLID, 2);
            ellipse(base_center_x - 45, base_center_y - 8, base_center_x + 45, base_center_y + 8);

            setlinecolor(RGB(200, 150, 20)); // 内圈暗金色
            setlinestyle(PS_SOLID, 1);
            ellipse(base_center_x - 35, base_center_y - 4, base_center_x + 35, base_center_y + 4);

            // 2. 绘制动态角色模型预览
            POINT old_pos = player->GetPosition();
            player->SetPosition({ preview_x, preview_y });
            player->Draw(1000 / 144); // 播放动画
            player->SetPosition(old_pos); // 绘制完把真实坐标还回去

            // 3. 绘制极简科技感边框 (四个角的折线)，完全不挡背景
            int ui_x1 = 40, ui_y1 = 40, ui_x2 = 340, ui_y2 = 300;
            setlinecolor(RGB(255, 200, 50)); // 蜂蜜金边框
            setlinestyle(PS_SOLID, 3);       // 加粗线条
            int corner_len = 25;             // 边角折线的长度

            // 左上角
            line(ui_x1, ui_y1, ui_x1 + corner_len, ui_y1);
            line(ui_x1, ui_y1, ui_x1, ui_y1 + corner_len);
            // 右上角
            line(ui_x2 - corner_len, ui_y1, ui_x2, ui_y1);
            line(ui_x2, ui_y1, ui_x2, ui_y1 + corner_len);
            // 左下角
            line(ui_x1, ui_y2 - corner_len, ui_x1, ui_y2);
            line(ui_x1, ui_y2, ui_x1 + corner_len, ui_y2);
            // 右下角
            line(ui_x2, ui_y2 - corner_len, ui_x2, ui_y2);
            line(ui_x2 - corner_len, ui_y2, ui_x2, ui_y2);

            // 恢复默认线宽，防止影响其他绘制
            setlinestyle(PS_SOLID, 1);

            // 4. 准备文字内容
            LPCTSTR name_text = _T("");
            LPCTSTR desc_text = _T("");
            if (selected_character == CharacterType::XiongDa) {
                name_text = _T("当前选择: 熊 大");
                desc_text = _T("定位: 重装战士 | 高血量");
            }
            else if (selected_character == CharacterType::XiongEr) {
                name_text = _T("当前选择: 熊 二");
                desc_text = _T("定位: 敏捷刺客 | 高移速");
            }
            else {
                name_text = _T("当前选择: 光头强");
                desc_text = _T("定位: 狂战士 | 脆皮高输出");
            }

            // 5. 绘制带强阴影的文字 (确保在花哨的背景上也清晰可见)
            setbkmode(TRANSPARENT);

            // 绘制名字
            settextstyle(28, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
            settextcolor(RGB(20, 20, 20)); // 黑色阴影
            outtextxy(72, 192, name_text);
            settextcolor(RGB(255, 200, 50)); // 金色主字
            outtextxy(70, 190, name_text);

            // 绘制定位描述
            settextstyle(18, 0, _T("黑体"), 0, 0, FW_NORMAL, false, false, false);
            settextcolor(RGB(20, 20, 20)); // 黑色阴影
            outtextxy(62, 232, desc_text);
            settextcolor(RGB(230, 230, 230)); // 亮灰色主字
            outtextxy(60, 230, desc_text);

            // 绘制 TAB 键操作提示
            settextstyle(20, 0, _T("黑体"), 0, 0, FW_BOLD, false, false, false);
            settextcolor(RGB(20, 20, 20)); // 黑色阴影
            outtextxy(52, 277, _T("[按下 TAB 键切换角色]"));
            settextcolor(RGB(100, 255, 100)); // 亮绿色提示
            outtextxy(50, 275, _T("[按下 TAB 键切换角色]"));
            
            // ================= 6. 【新增】绘制右上角的“难度选择”按钮 =================
            int diff_btn_w = 160;
            int diff_btn_h = 50;
            int diff_btn_x = WINDOW_WIDTH - diff_btn_w - 40; // 放在右上角
            int diff_btn_y = 40;

            // 绘制按钮底板 (带金边的深灰色圆角矩形)
            setlinecolor(RGB(200, 150, 50));
            setlinestyle(PS_SOLID, 2);
            setfillcolor(RGB(40, 40, 40));
            fillroundrect(diff_btn_x, diff_btn_y, diff_btn_x + diff_btn_w, diff_btn_y + diff_btn_h, 10, 10);
            setlinestyle(PS_SOLID, 1); // 恢复默认线宽

            // 准备难度文本和专属颜色
            LPCTSTR diff_text = _T("");
            COLORREF diff_color = RGB(255, 255, 255);
            if (current_difficulty == Difficulty::Easy) {
                diff_text = _T("难度: 简 单");
                diff_color = RGB(100, 255, 100); // 绿色，让人放松
            }
            else if (current_difficulty == Difficulty::Normal) {
                diff_text = _T("难度: 普 通");
                diff_color = RGB(255, 200, 50);  // 经典蜂蜜黄
            }
            else {
                diff_text = _T("难度: 困 难");
                diff_color = RGB(255, 80, 80);   // 红色警告
            }

            // 绘制难度按钮里的文字 (带阴影居中)
            settextstyle(22, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
            setbkmode(TRANSPARENT);
            int tw = textwidth(diff_text);
            int th = textheight(diff_text);
            int tx = diff_btn_x + (diff_btn_w - tw) / 2;
            int ty = diff_btn_y + (diff_btn_h - th) / 2;

            settextcolor(RGB(20, 20, 20));   // 阴影
            outtextxy(tx + 2, ty + 2, diff_text);
            settextcolor(diff_color);        // 主文字
            outtextxy(tx, ty, diff_text);

            // ================= 7. 【新增】绘制历史最高分记录面板 =================
            TCHAR history_text[128];
            _stprintf_s(history_text, _T(" 历史最高纪录 - 分数: %d 罐   存活: %d 秒 👑"), high_score, high_survival_time);

            settextstyle(24, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
            setbkmode(TRANSPARENT);

            // 计算文字宽度，使其完美在屏幕底端居中
            int hw = textwidth(history_text);
            int hx = (WINDOW_WIDTH - hw) / 2;
            int hy = WINDOW_HEIGHT - 60; // 放在画面最底部靠上一点点

            // 绘制极致立体的纯黑阴影
            settextcolor(RGB(20, 20, 20));
            outtextxy(hx + 2, hy + 2, history_text);
            // 绘制纯金高亮主文字
            settextcolor(RGB(255, 215, 0));
            outtextxy(hx, hy, history_text);
        }

        FlushBatchDraw();

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