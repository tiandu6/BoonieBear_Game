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

// 经验球数据结构：存储掉落经验球的二维空间坐标及当前是否可被拾取的活跃状态
struct ExpDrop
{
    double x, y;
    bool active = true;
};

// 敌人生成器核心逻辑
// 根据当前的游戏阶段 (current_phase)、游戏难度 (current_difficulty) 以及时间间隔，动态在屏幕边缘生成不同种类的敌人
void TryGenerateEnemy(vector<Enemy*>& enemy_list, int current_phase, bool& boss_spawned)
{
    int interval = 150;     // 默认生成时间间隔（帧数）
    size_t max_enemies = 8; // 默认同屏最大敌人数量

    // 依据难度枚举，动态调节生成频率与同屏数量上限
    if (current_difficulty == Difficulty::Easy)
    {
        interval = 200; max_enemies = 4;
    }
    else if (current_difficulty == Difficulty::Normal)
    {
        interval = 120; max_enemies = 8;
    }
    else
    {
        interval = 60; max_enemies = 15;
    }

    // 如果是双人模式，怪物生成速度加快 30%，同屏最大怪物数量提升 50%
    if (game_mode == 2)
    {
        interval = (int)(interval * 0.7);
        max_enemies = (size_t)(max_enemies * 1.5);
    }

    // 阶段3（最终阶段）：若 Boss 尚未生成，则强制在怪物列表中压入一个 Boss 实体
    if (current_phase == 3 && !boss_spawned) {
        enemy_list.push_back(new BossEnemy());
        boss_spawned = true;
    }

    // 性能与难度控制：若当前同屏怪物数量已达上限，则跳过本次生成
    if (enemy_list.size() >= max_enemies) return;

    // 静态计数器：用于累计游戏循环的调用次数，当达到 interval 设定的阈值时触发一次生成
    static int counter = 0;
    if ((++counter) % interval == 0)
    {
        // 阶段 2 及以上时，有 30% 概率生成血厚移速快的“机械怪”
        if (current_phase >= 2 && (rand() % 100 < 30)) {
            enemy_list.push_back(new MachineEnemy());
        }
        else {
            enemy_list.push_back(new NormalEnemy());
        }
    }
}

// 玩家实例工厂模式
// 根据传入的角色类型枚举，利用多态特性动态分配对应的派生类（熊大、熊二、光头强）对象并返回
Player* CreatePlayer(CharacterType type)
{
    if (type == CharacterType::XiongDa)
        return new XiongDa();
    if (type == CharacterType::XiongEr)
        return new XiongEr();
    return new GuangtouQiang();
}

// 统一绘制游戏内的 HUD (Head-Up Display) 面板
// 包括：左上角分数、存活时间、动态自适应的双人血槽、右上角暂停按钮
void DrawHUD(int score, const vector<Player*>& players, int survival_time_sec)
{
    // 1. 绘制总分数（底层黑色做阴影偏移，表层亮色做主体，增加文本立体感）
    static TCHAR score_text[64];
    _stprintf_s(score_text, _T("当前分数：%d"), score);

    settextstyle(32, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
    setbkmode(TRANSPARENT); // 设置文字背景透明

    settextcolor(RGB(50, 50, 50));
    outtextxy(17, 17, score_text);
    settextcolor(RGB(255, 180, 20));
    outtextxy(15, 15, score_text);

    // 2. 绘制存活时间
    static TCHAR time_text[64];
    _stprintf_s(time_text, _T("存活时间：%d 秒"), survival_time_sec);
    settextstyle(24, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
    settextcolor(RGB(50, 50, 50));
    outtextxy(17, 57, time_text);
    settextcolor(RGB(150, 200, 255));
    outtextxy(15, 55, time_text);

    // 3. 动态适应同屏血槽布局渲染阵列（完美兼容单人/双人模式）
    for (size_t i = 0; i < players.size(); i++) {
        int hp = players[i]->GetHP();
        int max_hp = players[i]->GetMaxHP();
        int bar_x = 15;
        int bar_y = 95 + i * 40; // 根据玩家索引向下偏移 Y 轴坐标，避免血槽重叠
        int bar_width = 300;
        int bar_height = 28;

        // 绘制血槽底色外框
        setlinecolor(RGB(30, 30, 30));
        setfillcolor(RGB(50, 50, 50));
        fillroundrect(bar_x, bar_y, bar_x + bar_width, bar_y + bar_height, 8, 8);

        // 计算当前生命值占比，并绘制血量填充条
        if (hp > 0)
        {
            int fill_width = (int)((double)hp / max_hp * bar_width);

            // 动态血量预警变色：>50%绿色，25%~50%黄色，<25%红色
            COLORREF hp_color;
            double hp_ratio = (double)hp / max_hp;

            if (hp_ratio > 0.5) hp_color = RGB(50, 220, 50);
            else if (hp_ratio > 0.25) hp_color = RGB(255, 200, 50);
            else hp_color = RGB(255, 50, 50);

            setfillcolor(hp_color);
            solidroundrect(bar_x + 1, bar_y + 1, bar_x + fill_width - 1, bar_y + bar_height - 1, 8, 8);
        }

        // 绘制血量具体数字文本（双人模式下会标注 1P/2P）
        TCHAR hp_text[64];
        if (game_mode == 2) _stprintf_s(hp_text, _T("%dP HP: %d / %d"), i + 1, hp, max_hp);
        else _stprintf_s(hp_text, _T("HP: %d / %d"), hp, max_hp);

        settextstyle(20, 0, _T("Arial"), 0, 0, FW_BOLD, false, false, false);

        // 计算文本居中渲染的位置
        int text_w = textwidth(hp_text);
        int text_h = textheight(hp_text);
        int text_x = bar_x + (bar_width - text_w) / 2;
        int text_y = bar_y + (bar_height - text_h) / 2;

        settextcolor(RGB(30, 30, 30));
        outtextxy(text_x + 2, text_y + 2, hp_text);
        settextcolor(RGB(255, 255, 255));
        outtextxy(text_x, text_y, hp_text);
    }

    // 4. 绘制右上角提示性的暂停按钮框架
    int btn_w = 130;
    int btn_h = 40;
    int btn_x = WINDOW_WIDTH - btn_w - 20;
    int btn_y = 20;

    setlinecolor(RGB(50, 50, 50));
    setfillcolor(RGB(40, 40, 40));
    fillroundrect(btn_x, btn_y, btn_x + btn_w, btn_y + btn_h, 10, 10);

    settextstyle(18, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
    settextcolor(RGB(220, 220, 220));
    LPCTSTR btn_text = _T("|| 暂停 (P)");

    int t_w = textwidth(btn_text);
    int t_h = textheight(btn_text);
    outtextxy(btn_x + (btn_w - t_w) / 2, btn_y + (btn_h - t_h) / 2, btn_text);
}

// 游戏主入口进程
int main()
{
    // 初始化 EasyX 绘图窗口，分辨率设定为 1280x720
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);

    // ================= 图集与素材资源初始化 =================
    // 从磁盘加载角色切片动画资源（参数依次为：路径模板、总帧数、单帧宽、单帧高、是否翻转）
    atlas_xiongda_right = new Atlas(_T("img/XiongDa_right_%d.png"), 8, 150, 150, false);
    atlas_xiongda_left = new Atlas(_T("img/XiongDa_right_%d.png"), 8, 150, 150, true);
    atlas_xionger_right = new Atlas(_T("img/XiongEr_right_%d.png"), 8, 150, 150, false);
    atlas_xionger_left = new Atlas(_T("img/XiongEr_right_%d.png"), 8, 150, 150, true);
    atlas_qiang_right = new Atlas(_T("img/GuangtouQiang_right_%d.png"), 8, 150, 150, false);
    atlas_qiang_left = new Atlas(_T("img/GuangtouQiang_right_%d.png"), 8, 150, 150, true);

    // 加载怪物切片动画资源
    atlas_enemy_right = new Atlas(_T("img/LoggingWorker_right_%d.png"), 8, 80, 80, false);
    atlas_enemy_left = new Atlas(_T("img/LoggingWorker_right_%d.png"), 8, 80, 80, true);
    atlas_machine_right = new Atlas(_T("img/MechanicalLoggingWorker_right_%d.png"), 8, 120, 120, false);
    atlas_machine_left = new Atlas(_T("img/MechanicalLoggingWorker_right_%d.png"), 8, 120, 120, true);
    atlas_boss_right = new Atlas(_T("img/Boss_right_%d.png"), 8, 200, 200, false);
    atlas_boss_left = new Atlas(_T("img/Boss_right_%d.png"), 8, 200, 200, true);

    // 加载全局阴影贴图并预拉伸好尺寸
    loadimage(&img_shadow_player, _T("img/shadow_player.png"), 80, 25, true);
    loadimage(&img_shadow_normal, _T("img/shadow_enemy.png"), 65, 20, true);
    loadimage(&img_shadow_machine, _T("img/shadow_enemy.png"), 90, 20, true);
    loadimage(&img_shadow_boss, _T("img/shadow_enemy.png"), 160, 30, true);

    // 加载并手动处理子弹贴图镜像
    loadimage(&img_bullet_right, _T("img/bullet_right.png"), 25, 10, true);
    img_bullet_left = img_bullet_right;

    // 直接获取显存中的图片像素缓冲区 (Buffer)，通过矩阵对折运算，物理生成向左飞行的子弹镜像图
    int bullet_w = img_bullet_left.getwidth();
    int bullet_h = img_bullet_left.getheight();
    DWORD* bullet_buffer = GetImageBuffer(&img_bullet_left);

    for (int y = 0; y < bullet_h; y++)
    {
        for (int x = 0; x < bullet_w / 2; x++)
        {
            int left_idx = y * bullet_w + x;
            int right_idx = y * bullet_w + (bullet_w - 1 - x);
            DWORD temp = bullet_buffer[left_idx];
            bullet_buffer[left_idx] = bullet_buffer[right_idx];
            bullet_buffer[right_idx] = temp;
        }
    }

    // ================= 多媒体音频资源预加载 =================
    // 刚打开游戏时不播放，仅加载后台资源流并配置别名，设置默认音量为 50%（500/1000）
    mciSendString(_T("open mus/bgm.mp3 alias bgm"), NULL, 0, NULL);
    mciSendString(_T("open mus/hit.mp3 alias hit"), NULL, 0, NULL);
    mciSendString(_T("setaudio bgm volume to 500"), NULL, 0, NULL);

    // ================= 核心游戏状态与实体容器初始化 =================
    int score = 0;                        // 击杀得分
    vector<Player*> players;              // 玩家对象数组（双人模式会有两个实体）
    ExMessage msg;                        // 操作系统的鼠标键盘消息捕获结构体
    IMAGE img_menu;                       // 菜单背景
    IMAGE img_background;                 // 游戏战斗内场景背景
    vector<Enemy*> enemy_list;            // 同屏敌人追踪数组

    // ================= 【架构重构】：UI 菜单按钮的多态池化管理 =================
    RECT region_btn_start_game, region_btn_quit_game;
    region_btn_start_game.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2;
    region_btn_start_game.right = region_btn_start_game.left + BUTTON_WIDTH;
    region_btn_start_game.top = 430;
    region_btn_start_game.bottom = region_btn_start_game.top + BUTTON_HEIGHT;

    region_btn_quit_game.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2;
    region_btn_quit_game.right = region_btn_quit_game.left + BUTTON_WIDTH;
    region_btn_quit_game.top = 550;
    region_btn_quit_game.bottom = region_btn_quit_game.top + BUTTON_HEIGHT;

    // 利用基类指针数组统一承载不同行为模式的按钮实体，方便后续进行统一分发
    vector<Button*> menu_buttons;
    menu_buttons.push_back(new StartGameButton(region_btn_start_game, _T("img/ui_start_idle.png"), _T("img/ui_start_hovered.png"), _T("img/ui_start_pushed.png")));
    menu_buttons.push_back(new QuitGameButton(region_btn_quit_game, _T("img/ui_quit_idle.png"), _T("img/ui_quit_hovered.png"), _T("img/ui_quit_pushed.png")));

    loadimage(&img_menu, _T("img/menu.png"));
    loadimage(&img_background, _T("img/background.png"));

    // 开启 EasyX 的双缓冲绘图机制，将所有绘图动作先写在内存中，最后一次性输出到屏幕，彻底消除画面闪烁
    BeginBatchDraw();

    // 游戏内部状态机标记
    bool is_game_paused = false;          // 游戏是否被暂停
    DWORD pause_start_time = 0;           // 记录暂停开启瞬间的系统时间，用于解除暂停时做时间补偿补偿
    DWORD level_up_start_time = 0;        // 记录升级面板弹出的时间
    static DWORD game_over_time = 0;      // 记录玩家死亡或通关导致游戏结束的时间

    vector<ExpDrop> exp_list;             // 场上散落的经验球数组
    bool is_leveling_up = false;          // 当前是否处于三选一升级状态
    int current_upgrade_options[3];       // 本次升级抽取的 3 个随机强化项目 ID

    LoadGameData();                       // 本地读取历史最高分和最高存活时间
    int survival_time_ms = 0;             // 本局累积存活时长（毫秒级）

    int current_phase = 0;                // 当前游戏阶段：1(树林), 2(机械), 3(Boss)
    DWORD phase_announce_time = 0;        // 触发新阶段提示横幅的时间
    bool is_game_won = false;             // 标识是否已击杀 Boss
    bool boss_spawned = false;            // 限制 Boss 唯一性，避免重复刷新

    // ================= UI 预览专用实体池 =================
    Player* preview_players[3];
    preview_players[0] = CreatePlayer(CharacterType::XiongDa);
    preview_players[1] = CreatePlayer(CharacterType::XiongEr);
    preview_players[2] = CreatePlayer(CharacterType::GuangtouQiang);

    // ================= 游戏主循环 (Game Loop) =================
    while (running)
    {
        DWORD start_time = GetTickCount(); // 记录本帧开始的时间戳，用于最后做帧率锁定 (FPS Control)

        // 1. 消息轮询阶段：非阻塞式读取鼠标、键盘的各项操作输入
        while (peekmessage(&msg))
        {
            // 监听键盘按键：处理暂停逻辑
            if (msg.message == WM_KEYDOWN && msg.vkcode == 'P')
            {
                if (is_game_started && !is_game_over && !is_leveling_up)
                {
                    is_game_paused = !is_game_paused; // 翻转暂停状态
                    if (is_game_paused)
                    {
                        pause_start_time = GetTickCount();
                    }
                    else
                    {
                        // 解除暂停时，必须把暂停消耗的总时间补偿给所有定时器，否则会产生冷却错乱
                        DWORD pause_duration = GetTickCount() - pause_start_time;
                        for (Player* p : players) p->AddPauseTime(pause_duration);
                        if (phase_announce_time > 0) phase_announce_time += pause_duration;
                    }
                }
            }

            // 监听鼠标滚轮：仅在暂停菜单中生效，用于调节音量大小
            if (msg.message == WM_MOUSEWHEEL)
            {
                if (is_game_started && is_game_paused && !is_game_over)
                {
                    if (msg.wheel > 0)
                    {
                        current_volume += 50;
                        if (current_volume > 1000) current_volume = 1000;
                    }
                    else if (msg.wheel < 0)
                    {
                        current_volume -= 50;
                        if (current_volume < 0) current_volume = 0;
                    }

                    // 组装并发送音频控制指令，动态修改底层的 mci 播放状态
                    TCHAR cmd[64];
                    _stprintf_s(cmd, _T("setaudio bgm volume to %d"), current_volume);
                    mciSendString(cmd, NULL, 0, NULL);
                }
            }

            // 【菜单独立按键】：仅在主界面未开始游戏时响应的选人与模式切换操作
            if (msg.message == WM_KEYDOWN && !is_game_started)
            {
                if (msg.vkcode == VK_TAB) {
                    int next_char = ((int)selected_character + 1) % 3; // 取模运算实现 3 个角色循环切换
                    selected_character = (CharacterType)next_char;
                }
                if (msg.vkcode == 'Q' && game_mode == 2) {
                    int next_char = ((int)selected_character_p2 + 1) % 3;
                    selected_character_p2 = (CharacterType)next_char;
                }
                if (msg.vkcode == 'Y') {
                    game_mode = (game_mode == 1) ? 2 : 1; // 1代表单机，2代表本地同屏双人
                }
            }

            // 【鼠标点击处理】：包括升级选项、游戏内暂停键、主界面难度按钮等热区的相交判定
            if (msg.message == WM_LBUTTONDOWN)
            {
                // 如果在升级面板中，判定鼠标点击在哪张卡片上
                if (is_leveling_up)
                {
                    int card_w = 220, card_h = 320, gap = 50;
                    int start_x = (WINDOW_WIDTH - (3 * card_w + 2 * gap)) / 2;
                    int start_y = (WINDOW_HEIGHT - card_h) / 2;

                    for (int i = 0; i < 3; i++)
                    {
                        int cx = start_x + i * (card_w + gap);
                        // AABB 矩形包围盒点击相交测试
                        if (msg.x >= cx && msg.x <= cx + card_w && msg.y >= start_y && msg.y <= start_y + card_h)
                        {
                            // 确认选择后，将该强化效果广度应用给所有的玩家对象
                            for (Player* p : players) p->ApplyUpgrade(current_upgrade_options[i]);
                            is_leveling_up = false; // 解除升级锁定状态

                            // 同样需要做时间补偿，否则刚选完升级怪物可能瞬间位移
                            DWORD pause_duration = GetTickCount() - level_up_start_time;
                            for (Player* p : players) p->AddPauseTime(pause_duration);
                            if (phase_announce_time > 0) phase_announce_time += pause_duration;
                            break;
                        }
                    }
                }
                else if (is_game_started && !is_game_over)
                {
                    // 检测是否点击了右上角虚拟暂停按钮
                    int btn_x = WINDOW_WIDTH - 130 - 20;
                    int btn_y = 20;
                    if (msg.x >= btn_x && msg.x <= btn_x + 130 && msg.y >= btn_y && msg.y <= btn_y + 40)
                    {
                        is_game_paused = !is_game_paused;
                        if (is_game_paused)
                        {
                            pause_start_time = GetTickCount();
                        }
                        else
                        {
                            DWORD pause_duration = GetTickCount() - pause_start_time;
                            for (Player* p : players) p->AddPauseTime(pause_duration);
                            if (phase_announce_time > 0) phase_announce_time += pause_duration;
                        }
                    }
                }
                else if (!is_game_started)
                {
                    // 检测主菜单难度切换按钮（简、普、难循环）
                    int diff_btn_w = 160;
                    int diff_btn_h = 50;
                    int diff_btn_x = WINDOW_WIDTH - diff_btn_w - 40;
                    int diff_btn_y = 40;

                    if (msg.x >= diff_btn_x && msg.x <= diff_btn_x + diff_btn_w &&
                        msg.y >= diff_btn_y && msg.y <= diff_btn_y + diff_btn_h)
                    {
                        int next_diff = ((int)current_difficulty + 1) % 3;
                        current_difficulty = (Difficulty)next_diff;
                    }
                }
            }

            // 利用多态机制，遍历池化按钮分发底层鼠标消息
            if (!is_game_started)
            {
                for (Button* btn : menu_buttons)
                {
                    btn->ProcessEvent(msg);
                }
            }

            // 【玩家游戏内操作接收】：将操作指令向下层层派发给各个活着的 Player 实例
            if (is_game_started && !is_game_over)
            {
                for (Player* p : players) {
                    // 只处理存活玩家的操作指令，避免死后依然乱移
                    if (p->GetHP() > 0)
                        p->ProcessEvent(msg, game_mode);
                }

                // 独立的主动技能按键响应与 CD 冷却判定
                if (msg.message == WM_KEYDOWN && !is_game_paused && !is_leveling_up)
                {
                    DWORD cur_t = GetTickCount();
                    // 1P 放技能（绑定空格键）
                    if (msg.vkcode == VK_SPACE && players.size() > 0 && players[0]->GetHP() > 0)
                    {
                        if (cur_t - players[0]->last_skill_time >= players[0]->SKILL_CD)
                        {
                            players[0]->UseSkill(enemy_list);
                            players[0]->last_skill_time = cur_t; // 重置冷却
                        }
                    }
                    // 2P 放技能（绑定数字键 0）
                    if (game_mode == 2 && (msg.vkcode == VK_NUMPAD0 || msg.vkcode == '0') && players.size() > 1 && players[1]->GetHP() > 0)
                    {
                        if (cur_t - players[1]->last_skill_time >= players[1]->SKILL_CD)
                        {
                            players[1]->UseSkill(enemy_list);
                            players[1]->last_skill_time = cur_t;
                        }
                    }
                }
            }
        } // end of while (peekmessage) 消息轮询结束

        // ================= 【极度安全的防卡死初始化 & 音乐播放逻辑】 =================
        // 防止玩家刚点完开始按钮、但对象数组来不及初始化就被渲染层拦截导致的崩溃死锁
        if (is_game_started && players.empty())
        {
            // 在正式切入游戏的一瞬间，开始无限循环播放 BGM
            mciSendString(_T("play bgm repeat"), NULL, 0, NULL);

            // 通过工厂构建 1P 对象，并赋予 1 号手柄标识
            players.push_back(CreatePlayer(selected_character));
            players[0]->player_id = 1;
            players[0]->SetPosition({ 400, 500 });

            // 若在双人模式下，一并拉起 2P 对象并放置在右侧区域
            if (game_mode == 2) {
                players.push_back(CreatePlayer(selected_character_p2));
                players[1]->player_id = 2;
                players[1]->SetPosition({ 800, 500 });
            }
        }

        // 2. 游戏核心逻辑运算阶段：处理移动、碰撞、AI追踪、经验结算（仅在游戏进行且未暂停时进行）
        if (is_game_started && !is_game_over && !is_game_paused && !is_leveling_up && !players.empty())
        {
            // 以固定刷新率累加游戏存活时间
            survival_time_ms += (1000 / 144);

            // ================= 经验球磁力追踪吸收逻辑 =================
            for (size_t i = 0; i < exp_list.size(); i++)
            {
                if (!exp_list[i].active) continue; // 跳过已被标记为删除的废弃节点

                Player* closest_p = nullptr;
                double min_dist = 999999;

                // 找到距离当前经验球欧几里得距离最近的存活玩家
                for (Player* p : players) {
                    if (p->GetHP() <= 0) continue;
                    double dx = (p->GetPosition().x + p->FRAME_WIDTH / 2) - exp_list[i].x;
                    double dy = (p->GetPosition().y + p->FRAME_HEIGHT / 2) - exp_list[i].y;
                    double dist = sqrt(dx * dx + dy * dy);
                    if (dist < min_dist) { min_dist = dist; closest_p = p; }
                }

                // 如果最近的玩家进入了经验球的磁吸判定范围（180像素内）
                if (closest_p && min_dist < 180)
                {
                    double dx = (closest_p->GetPosition().x + closest_p->FRAME_WIDTH / 2) - exp_list[i].x;
                    double dy = (closest_p->GetPosition().y + closest_p->FRAME_HEIGHT / 2) - exp_list[i].y;

                    // 基于向量对经验球进行插值移动，模拟逐渐吸附过去的特效
                    exp_list[i].x += (dx / min_dist) * 10.0;
                    exp_list[i].y += (dy / min_dist) * 10.0;

                    // 若二者已经极其贴近（35像素内），判定为成功拾取
                    if (min_dist < 35)
                    {
                        exp_list[i].active = false;
                        players[0]->exp += 1; // 经验值是全局共享池，统一挂载在 0 号玩家身上即可
                    }
                }
            }

            // ================= 性能优化：快速清理垃圾内存 =================
            // 采用 swap(头, 尾) -> pop_back 的无序删除技巧，将擦除时间复杂度由 O(N) 降为 O(1)
            for (size_t i = 0; i < exp_list.size(); i++) {
                if (!exp_list[i].active) {
                    swap(exp_list[i], exp_list.back());
                    exp_list.pop_back();
                    i--;
                }
            }

            // ================= 等级提升判定逻辑 =================
            if (players[0]->exp >= players[0]->max_exp)
            {
                players[0]->exp -= players[0]->max_exp;
                for (Player* p : players) p->level++;
                players[0]->max_exp += 3; // 升级曲线：每级多需 3 点经验

                // 挂起游戏物理逻辑，触发 UI 层升级界面拦截
                is_leveling_up = true;
                level_up_start_time = GetTickCount();

                // 使用洗牌算法 (Fisher-Yates Shuffle) 打乱 7 个备选属性 ID 池，并抽出前 3 个作为本轮强化选项
                int pool[7] = { 0, 1, 2, 3, 4, 5, 6 };
                for (int i = 0; i < 7; i++)
                {
                    int r = i + rand() % (7 - i);
                    swap(pool[i], pool[r]);
                }
                current_upgrade_options[0] = pool[0];
                current_upgrade_options[1] = pool[1];
                current_upgrade_options[2] = pool[2];
            }

            // ================= 游戏进度与难度阶段调度器 =================
            int target_phase = 1;
            if (survival_time_ms >= 150000) target_phase = 3;      // 150秒后进入三阶段（boss）
            else if (survival_time_ms >= 60000) target_phase = 2;  // 60秒后进入二阶段（刷机械怪）

            // 若达成跃迁条件，更新阶段标识并触发巨幕文字播报计时
            if (target_phase > current_phase) {
                current_phase = target_phase;
                phase_announce_time = GetTickCount();
            }

            // ================= 玩家与子系统逻辑自循环 =================
            for (Player* p : players) {
                if (p->GetHP() > 0) {
                    p->Move();
                    p->UpdateAttacks();
                    p->UpdateExtraSkills(enemy_list);
                }
                else {
                    // 如果玩家已经死亡，强行重置/清空其武器库状态，防止死后武器依然活跃
                    if (p->GetBullets().size() > 0 && p->GetBullets()[0].is_active) {
                        for (Bullet& b : p->GetBullets()) b.is_active = false;
                    }
                }
            }

            // 调用敌人生成分发器补充场上怪物
            TryGenerateEnemy(enemy_list, current_phase, boss_spawned);

            // ================= AI 索敌寻路算法 =================
            size_t current_enemy_count = enemy_list.size();
            for (size_t i = 0; i < current_enemy_count; i++)
            {
                // Move 内部会遍历所有玩家，测算最短路径向量并不断迫近
                enemy_list[i]->Move(players, enemy_list);
            }

            // ================= 物理碰撞：敌方伤害结算 =================
            for (Enemy* enemy : enemy_list)
            {
                for (Player* p : players) {
                    // 当存活的玩家坐标包围盒与敌人包围盒发生相交判定
                    if (p->GetHP() > 0 && enemy->CheckPlayerCollision(*p))
                    {
                        // 计算当前所选难度对应的伤害放大/缩小倍率
                        double diff_multiplier = 1.0;
                        if (current_difficulty == Difficulty::Easy) diff_multiplier = 0.6;
                        else if (current_difficulty == Difficulty::Normal) diff_multiplier = 1.0;
                        else if (current_difficulty == Difficulty::Hard) diff_multiplier = 2.0;

                        // 让玩家承受带难度系数的削减后最终伤害
                        int dmg = (int)(enemy->collision_damage * diff_multiplier);
                        p->TakeDamage(dmg);
                    }
                }
            }

            // ================= 玩家团灭终结判定 =================
            bool all_dead = true;
            for (Player* p : players) if (p->GetHP() > 0) all_dead = false;

            if (all_dead && !is_game_over)
            {
                is_game_over = true;
                game_over_time = GetTickCount(); // 封存死亡瞬时时间
                SaveGameData(score, survival_time_ms / 1000); // 并将成绩落盘固化
            }

            // ================= 物理碰撞：玩家弹幕/武器攻击结算 =================
            for (Enemy* enemy : enemy_list)
            {
                for (Player* p : players)
                {
                    // 如果玩家已经阵亡，他的武器不再具有任何物理伤害判定效力，直接跳过
                    if (p->GetHP() <= 0) continue;

                    // 遍历该名存活玩家投射在屏幕上的所有火力实体
                    for (const Bullet& bullet : p->GetBullets())
                    {
                        // 若子弹击中该名敌人
                        if (enemy->CheckBulletCollision(bullet))
                        {
                            // 扣除怪物血量（如果怪物处于无敌硬直内则 hurt 返回 false）
                            if (enemy->Hurt(p->GetAttackDamage(), bullet.pos))
                            {
                                // 确认真伤命中时，从零重置播放受击音效
                                mciSendString(_T("play hit from 0"), NULL, 0, NULL);
                            }
                        }
                    }
                }
            }

            // ================= 清理战场与击杀奖励结算 =================
            for (size_t i = 0; i < enemy_list.size(); i++)
            {
                Enemy* enemy = enemy_list[i];
                if (!enemy->CheckAlive()) // 如果怪物血量归零且通过了死亡缓冲帧
                {
                    if (enemy->is_boss)
                    {
                        // Boss 阵亡是游戏胜利的唯一条件
                        is_game_won = true;
                        is_game_over = true;
                        game_over_time = GetTickCount();
                        SaveGameData(score, survival_time_ms / 1000);

                        // 清屏机制：给场上残余的小怪施加超长时长的无敌闪烁遮蔽，视觉上使其消失
                        for (Enemy* e : enemy_list)
                        {
                            if (e != enemy)
                            {
                                e->ApplySkillFlicker(3000);
                            }
                        }
                    }
                    else
                    {
                        // 普通怪阵亡增加分数，并在其尸体坐标处爆出一颗蓝色经验球进入场中
                        score++;
                        ExpDrop exp_drop;
                        exp_drop.x = enemy->GetPosition().x + 40;
                        exp_drop.y = enemy->GetPosition().y + 40;
                        exp_list.push_back(exp_drop);
                    }

                    // 垃圾回收：清除野指针释放堆内存，同样采用 swap 法快速缩容
                    swap(enemy_list[i], enemy_list.back());
                    enemy_list.pop_back();
                    delete enemy;
                    i--;
                }
            }
        }
        // ================= 游戏结束下的状态捕获 =================
        else if (is_game_over)
        {
            if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
            {
                running = false; // 按 ESC 直接关闭程序
            }
            if (GetAsyncKeyState('R') & 0x8000)
            {
                // 按 R 键快捷“再来一局”：清空并重置所有数据指针与标志位
                is_game_over = false;
                is_game_paused = false;
                score = 0;
                survival_time_ms = 0;
                for (Enemy* enemy : enemy_list) delete enemy;
                enemy_list.clear();

                for (Player* p : players) delete p;
                players.clear();

                // 重新派发满血新角色进入战斗区域
                players.push_back(CreatePlayer(selected_character));
                players[0]->player_id = 1;
                players[0]->SetPosition({ 400, 500 });

                if (game_mode == 2) {
                    players.push_back(CreatePlayer(selected_character_p2));
                    players[1]->player_id = 2;
                    players[1]->SetPosition({ 800, 500 });
                }

                Sleep(150); // 简单防抖，避免按键瞬间被连续触发两次

                current_phase = 0;
                phase_announce_time = 0;
                is_game_won = false;
                boss_spawned = false;

                exp_list.clear();
                is_leveling_up = false;
            }
            if (GetAsyncKeyState('M') & 0x8000)
            {
                // ================= 【音乐播放修改 3】 =================
                // 玩家返回主菜单时，将音乐停止并拉回起始轨道，避免在菜单产生 BGM 残留吵闹
                mciSendString(_T("stop bgm"), NULL, 0, NULL);
                mciSendString(_T("seek bgm to start"), NULL, 0, NULL);

                // 抹除战局状态，将视角退回主菜单
                is_game_started = false;
                is_game_over = false;
                is_game_paused = false;
                score = 0;
                survival_time_ms = 0;

                // 深度内存清理
                for (Enemy* enemy : enemy_list) delete enemy;
                enemy_list.clear();
                for (Player* p : players) delete p;
                players.clear();

                Sleep(150);

                current_phase = 0;
                phase_announce_time = 0;
                is_game_won = false;
                boss_spawned = false;

                exp_list.clear();
                is_leveling_up = false;
            }
        }

        // 3. 游戏界面渲染绘制阶段
        // 调用底层的 cleardevice 以默认黑色洗去上一帧残留在屏幕缓冲区的图像脏数据
        cleardevice();

        // === 游戏进行时的画面渲染层 ===
        if (is_game_started && !players.empty())
        {
            // Z-Index = 0：贴上纯静态的大图背景底板
            putimage(0, 0, &img_background);

            // 获取差量时间，如果暂停或在选升级界面中，给到后续动画库的 delta 设为 0（视觉冻结）
            int current_delta = (is_game_paused || is_leveling_up) ? 0 : (1000 / 144);
            DWORD tick = GetTickCount();

            // Z-Index = 1：绘制地上散落的经验球
            for (const ExpDrop& e : exp_list)
            {
                int breath = (tick / 100) % 4; // 通过 tick 取模制作 4 帧循环的视觉涨缩呼吸感效果
                setlinecolor(RGB(170, 230, 255));
                setfillcolor(RGB(170, 230, 255));
                solidcircle((int)e.x, (int)e.y, 8 + breath); // 外圈光晕
                setfillcolor(RGB(220, 245, 255));
                solidcircle((int)e.x, (int)e.y, 5 + breath / 2); // 内圈本体
                setfillcolor(RGB(255, 255, 255));
                solidcircle((int)e.x, (int)e.y, 2);              // 高光白斑
            }

            // Z-Index = 2：调用各个玩家自封装的渲染模块画出自身动画帧与投影
            for (Player* p : players) p->Draw(current_delta);

            // Z-Index = 3：绘制爬行的敌人
            for (Enemy* enemy : enemy_list)
            {
                // 如果此时已经通关（Boss已死）且超过 1.5 秒演出期，直接屏蔽小怪绘制
                if (is_game_over && is_game_won && (tick - game_over_time > 1500)) continue;
                enemy->Draw(current_delta);
            }

            // Z-Index = 4：在人物贴图上层绘制环绕自身的武器、投射出去的子弹以及从天而降的落雷特效
            for (Player* p : players) {
                if (p->GetHP() > 0) { // 核心拦截：死亡状态下不允许绘制漂浮特效，避免画面出现灵异残留
                    p->DrawAttacks();
                    p->orbital_skill.Draw(is_game_paused || is_leveling_up, is_game_paused ? pause_start_time : level_up_start_time);
                }
            }

            // Z-Index = 5：通关时的庆祝特效层
            if (is_game_over && is_game_won && (tick - game_over_time < 1500))
            {
                DWORD elapsed = tick - game_over_time;
                double progress = elapsed / 1500.0;

                // 在每一个残余敌人的身上绘制一道不断向外扩散膨胀并渐渐变细消失的白色终结冲击波
                for (Enemy* enemy : enemy_list)
                {
                    POINT p = enemy->GetPosition();
                    int center_x = p.x + 40;
                    int center_y = p.y + 40;

                    int radius = (int)(progress * 200);
                    int thickness = (int)((1.0 - progress) * 15);

                    if (thickness > 0)
                    {
                        setlinecolor(RGB(100, 200, 255));
                        setlinestyle(PS_SOLID, thickness);
                        circle(center_x, center_y, radius);

                        setlinecolor(RGB(255, 255, 255));
                        setlinestyle(PS_SOLID, max(1, thickness / 2));
                        circle(center_x, center_y, (int)(radius * 0.8));
                    }

                    // 伴随冲击波的四角爆裂十字星特效
                    if (progress < 0.5)
                    {
                        double star_prog = progress * 2.0;
                        int star_len = (int)(star_prog * 120);
                        int star_thick = (int)((1.0 - star_prog) * 8);

                        if (star_thick > 0)
                        {
                            setlinecolor(RGB(255, 255, 255));
                            setlinestyle(PS_SOLID, star_thick);
                            line(center_x - star_len, center_y - star_len, center_x + star_len, center_y + star_len);
                            line(center_x - star_len, center_y + star_len, center_x + star_len, center_y - star_len);
                        }
                    }
                }
                setlinestyle(PS_SOLID, 1); // 必须归位线条样式，避免污染下一帧的默认绘图线宽
            }

            // Z-Index = 6：绘制左上角通用静态 UI 信息面板（血槽、计分板）
            DrawHUD(score, players, survival_time_ms / 1000);

            // ================= 经验槽及进度等相关动态 UI 渲染 =================
            // 修复后的代码：
            DWORD current_render_time = is_game_paused ? pause_start_time : (is_leveling_up ? level_up_start_time : tick); // 防止暂停时冷却面板颜色继续偷跑计算

            // 绘制底部的全局共享升级经验蓝色空心拉丝进度条
            int exp_bar_y = 95 + players.size() * 40;
            int exp_bar_w = 300;
            int exp_bar_h = 16;
            int exp_bar_x = 15;

            setlinecolor(RGB(50, 50, 50));
            setfillcolor(RGB(30, 30, 30));
            fillroundrect(exp_bar_x, exp_bar_y, exp_bar_x + exp_bar_w, exp_bar_y + exp_bar_h, 3, 3);

            if (players[0]->exp > 0)
            {
                int exp_fill = (int)((double)players[0]->exp / players[0]->max_exp * exp_bar_w);
                setfillcolor(RGB(0, 180, 255));
                solidroundrect(exp_bar_x + 1, exp_bar_y + 1, exp_bar_x + exp_fill - 1, exp_bar_y + exp_bar_h - 1, 3, 3);
            }

            // 经验条附带的 Lv 文字等级与 EXP 字母标注
            TCHAR lvl_text[32];
            _stprintf_s(lvl_text, _T("Lv.%d"), players[0]->level);
            settextstyle(16, 0, _T("黑体"), 0, 0, FW_BOLD, false, false, false);

            settextcolor(RGB(20, 20, 20));
            outtextxy(exp_bar_x + 8 + 1, exp_bar_y + 1, lvl_text);
            settextcolor(RGB(255, 215, 0));
            outtextxy(exp_bar_x + 8, exp_bar_y, lvl_text);

            settextcolor(RGB(20, 20, 20));
            outtextxy(exp_bar_x + exp_bar_w - 35 + 1, exp_bar_y + 1, _T("EXP"));
            settextcolor(RGB(150, 200, 255));
            outtextxy(exp_bar_x + exp_bar_w - 35, exp_bar_y, _T("EXP"));

            // 绘制顶部中轴的难度推进宽大进度条（橙色）
            int prog_bar_w = 500;
            int prog_bar_h = 16;
            int prog_bar_x = (WINDOW_WIDTH - prog_bar_w) / 2;
            int prog_bar_y = 40;

            double phase_progress = 0.0;
            LPCTSTR phase_target_text = _T("");
            int time_left_sec = 0;

            // 根据不同的生存时间判定，拆分绘制区间计算并展示下一阶段文本倒计时
            if (survival_time_ms < 60000) {
                phase_progress = (double)survival_time_ms / 60000.0;
                time_left_sec = (60000 - survival_time_ms) / 1000;
                phase_target_text = _T("阶段一：森林保卫战 距离下一阶段");
            }
            else if (survival_time_ms < 150000) {
                phase_progress = (double)(survival_time_ms - 60000) / 90000.0;
                time_left_sec = (150000 - survival_time_ms) / 1000;
                phase_target_text = _T("阶段二：机械危机 距离下一阶段");
            }
            else {
                phase_progress = 1.0;
                time_left_sec = 0;
                phase_target_text = _T("最终阶段：击败黑化光头强！");
            }

            // 绘制框体容器与橙色高亮填充
            setlinecolor(RGB(150, 150, 150));
            setlinestyle(PS_SOLID, 2);
            roundrect(prog_bar_x - 2, prog_bar_y - 2, prog_bar_x + prog_bar_w + 2, prog_bar_y + prog_bar_h + 2, 5, 5);
            setlinestyle(PS_SOLID, 1);

            setlinecolor(RGB(50, 50, 50));
            setfillcolor(RGB(30, 30, 30));
            fillroundrect(prog_bar_x, prog_bar_y, prog_bar_x + prog_bar_w, prog_bar_y + prog_bar_h, 3, 3);

            if (phase_progress > 0)
            {
                int prog_fill = (int)(phase_progress * prog_bar_w);
                setfillcolor(RGB(255, 120, 50));
                solidroundrect(prog_bar_x + 1, prog_bar_y + 1, prog_bar_x + prog_fill - 1, prog_bar_y + prog_bar_h - 1, 3, 3);
            }

            // 拼装进度时间动态字符串，采取上下左右画四次黑字，中间画一次黄字的低成本文本描边手法
            TCHAR prog_text[64];
            if (survival_time_ms < 150000) {
                _stprintf_s(prog_text, _T("%s还有 %d 秒"), phase_target_text, time_left_sec);
            }
            else {
                _stprintf_s(prog_text, _T("%s"), phase_target_text);
            }

            settextstyle(22, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
            setbkmode(TRANSPARENT);

            int tw = textwidth(prog_text);
            int tx = prog_bar_x + (prog_bar_w - tw) / 2;
            int ty = prog_bar_y - 30;

            settextcolor(RGB(30, 30, 30)); // 绘制描边
            outtextxy(tx - 1, ty - 1, prog_text);
            outtextxy(tx + 1, ty - 1, prog_text);
            outtextxy(tx - 1, ty + 1, prog_text);
            outtextxy(tx + 1, ty + 1, prog_text);
            outtextxy(tx, ty + 2, prog_text);
            settextcolor(RGB(255, 220, 50)); // 绘制核心色彩
            outtextxy(tx, ty, prog_text);

            // ================= 满级悬浮框 UI 面板渲染 =================
            if (is_leveling_up)
            {
                int panel_w = 900, panel_h = 500;
                int px = (WINDOW_WIDTH - panel_w) / 2;
                int py = (WINDOW_HEIGHT - panel_h) / 2;

                setlinecolor(RGB(200, 150, 50));
                setlinestyle(PS_SOLID, 2);
                setfillcolor(RGB(30, 30, 30));
                fillroundrect(px, py, px + panel_w, py + panel_h, 15, 15); // 黑金色遮罩悬浮板
                setlinestyle(PS_SOLID, 1);

                settextstyle(42, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
                settextcolor(RGB(255, 200, 50));
                LPCTSTR up_title = _T("等级提升！请选择一项强化：");
                outtextxy((WINDOW_WIDTH - textwidth(up_title)) / 2, py + 30, up_title);

                int card_w = 220, card_h = 320, gap = 50;
                int start_x = px + (panel_w - (3 * card_w + 2 * gap)) / 2;
                int start_y = py + 120;

                // 循环根据前面 Fisher-Yates 洗出的三个有效随机数组 ID，构建 3 张带有详情的强化选拔扑克牌
                for (int i = 0; i < 3; i++)
                {
                    int cx = start_x + i * (card_w + gap);
                    int cy = start_y;

                    setlinecolor(RGB(100, 100, 100));
                    setfillcolor(RGB(45, 45, 45));
                    fillroundrect(cx, cy, cx + card_w, cy + card_h, 10, 10);

                    LPCTSTR title = _T("");
                    LPCTSTR desc = _T("");
                    COLORREF c_color = RGB(255, 255, 255);

                    // 多态选项数据装载，分别给不同词条赋予独立的高光颜色进行辨识
                    int opt = current_upgrade_options[i];
                    if (opt == 0) {
                        title = _T("强壮体魄"); desc = _T("最大生命值 +20"); c_color = RGB(50, 255, 50);
                    }
                    else if (opt == 1) {
                        title = _T("火力强化"); desc = _T("基础攻击力 +1"); c_color = RGB(255, 100, 100);
                    }
                    else if (opt == 2) {
                        title = _T("身轻如燕"); desc = _T("移动速度 +1"); c_color = RGB(100, 200, 255);
                    }
                    else if (opt == 3) {
                        title = _T("绝地急救"); desc = _T("恢复 50% 生命"); c_color = RGB(255, 200, 50);
                    }
                    else if (opt == 4) {
                        title = _T("缩减cd"); desc = _T("技能冷却减少 20%"); c_color = RGB(200, 100, 255);
                    }
                    else if (opt == 5) {
                        title = _T("钢铁之躯"); desc = _T("减免 20% 受到伤害"); c_color = RGB(180, 180, 180);
                    }
                    else if (opt == 6) {
                        title = _T("天降正义");
                        static TCHAR orbital_desc[64];
                        // 动态获取玩家0（共用）的技能等级来动态展示卡牌描述文字
                        if (players[0]->orbital_skill.level == 0) {
                            desc = _T("自动召唤高伤害落雷");
                        }
                        else {
                            _stprintf_s(orbital_desc, _T("升级落雷 (Lv.%d) : 伤害+ 冷却-"), players[0]->orbital_skill.level + 1);
                            desc = orbital_desc;
                        }
                        c_color = RGB(0, 255, 255);
                    }

                    // 绘制每张牌内部带颜色的荧光内边框
                    setlinecolor(c_color);
                    setlinestyle(PS_SOLID, 3);
                    roundrect(cx + 10, cy + 10, cx + card_w - 10, cy + card_h - 10, 5, 5);
                    setlinestyle(PS_SOLID, 1);

                    settextstyle(28, 0, _T("黑体"), 0, 0, FW_BOLD, false, false, false);
                    settextcolor(c_color);
                    outtextxy(cx + (card_w - textwidth(title)) / 2, cy + 40, title);

                    settextstyle(18, 0, _T("微软雅黑"), 0, 0, FW_NORMAL, false, false, false);
                    settextcolor(RGB(220, 220, 220));
                    outtextxy(cx + (card_w - textwidth(desc)) / 2, cy + 160, desc);

                    settextstyle(16, 0, _T("黑体"), 0, 0, FW_NORMAL, false, false, false);
                    settextcolor(RGB(120, 120, 120));
                    outtextxy(cx + (card_w - textwidth(_T("- 点击选择 -"))) / 2, cy + 280, _T("- 点击选择 -"));
                }
            }

            // ================= 玩家独立按键技能指示面板 =================
            // 绘制双边技能独立 CD 提示充能槽
            for (size_t i = 0; i < players.size(); i++) {
                DWORD elapsed = current_render_time - players[i]->last_skill_time; // 使用修复后的安全时间变量
                int cd_w = 260;
                int cd_h = 26;
                int cd_x = 0;

                // 适配排版：如果只有 1 人，置于屏幕正中底部；如果双人，则强制靠屏幕两侧排布分明
                if (game_mode == 1) {
                    cd_x = (WINDOW_WIDTH - cd_w) / 2;
                }
                else {
                    if (i == 0) cd_x = 30;
                    else cd_x = WINDOW_WIDTH - cd_w - 30;
                }
                int cd_y = WINDOW_HEIGHT - 40;

                setlinecolor(RGB(50, 50, 50));
                setfillcolor(RGB(30, 30, 30));
                fillroundrect(cd_x, cd_y, cd_x + cd_w, cd_y + cd_h, 5, 5);

                // 字符串解析拼接，根据角色名称映射展示不同的技能称号
                CharacterType p_char = (i == 0) ? selected_character : selected_character_p2;
                LPCTSTR skill_name = _T("技能");
                if (p_char == CharacterType::XiongDa) skill_name = _T("熊掌拍击");
                else if (p_char == CharacterType::XiongEr) skill_name = _T("蜂蜜回血");
                else skill_name = _T("火力全开");

                TCHAR ready_text[64];
                if (game_mode == 1 || i == 0) _stprintf_s(ready_text, _T("[空格] %s"), skill_name);
                else _stprintf_s(ready_text, _T("[Num0] %s"), skill_name);

                // 根据独立时间戳，如果冷却完成呈现高亮色，如果充能中截断长度呈现黄褐色
                if (elapsed >= players[i]->SKILL_CD)
                {
                    setfillcolor(RGB(50, 255, 255));
                    solidroundrect(cd_x + 1, cd_y + 1, cd_x + cd_w - 1, cd_y + cd_h - 1, 5, 5);
                    settextstyle(16, 0, _T("黑体"), 0, 0, FW_BOLD, false, false, false);
                    settextcolor(RGB(0, 0, 0));

                    int tx = cd_x + (cd_w - textwidth(ready_text)) / 2;
                    outtextxy(tx, cd_y + 5, ready_text);
                }
                else
                {
                    int fill_w = (int)((double)elapsed / players[i]->SKILL_CD * cd_w);
                    setfillcolor(RGB(150, 100, 50));
                    solidroundrect(cd_x + 1, cd_y + 1, cd_x + fill_w - 1, cd_y + cd_h - 1, 5, 5);
                    settextstyle(16, 0, _T("黑体"), 0, 0, FW_BOLD, false, false, false);
                    settextcolor(RGB(200, 200, 200));

                    LPCTSTR cd_text = _T("技能充能中...");
                    int tx = cd_x + (cd_w - textwidth(cd_text)) / 2;
                    outtextxy(tx, cd_y + 5, cd_text);
                }
            }

            // ================= 阶段跳跃预警跑马灯大字 =================
            // 当游戏难度进入下一阶段时，在中心展示带有红黄闪烁频闪特效的警告语，存在时间 3500ms
            if (current_phase > 0 && (tick - phase_announce_time < 3500) && !is_game_over && !is_leveling_up)
            {
                DWORD elapsed = tick - phase_announce_time;
                settextstyle(60, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
                setbkmode(TRANSPARENT);

                LPCTSTR phase_text;
                if (current_phase == 1) phase_text = _T("第一阶段：森林保卫战");
                else if (current_phase == 2) phase_text = _T("第二阶段：机械危机");
                else phase_text = _T("最终阶段：迎战黑化光头强！");

                COLORREF main_color = (elapsed / 150 % 2 == 0) ? RGB(255, 200, 50) : RGB(255, 80, 80);
                if (current_phase == 1) main_color = RGB(100, 255, 150);

                int tw = textwidth(phase_text);
                int tx = (WINDOW_WIDTH - tw) / 2;
                int ty = WINDOW_HEIGHT / 4;

                settextcolor(RGB(20, 20, 20));
                outtextxy(tx + 4, ty + 4, phase_text);
                settextcolor(main_color);
                outtextxy(tx, ty, phase_text);
            }

            // ================= 核心状态中断：通用系统菜单面板 =================
            // 绘制点击或者按下 P 触发的游戏暂停遮罩层板
            if (is_game_paused && !is_game_over)
            {
                setlinecolor(RGB(0, 0, 0));
                setfillcolor(RGB(20, 20, 20));

                int panel_w = 360;
                int panel_h = 200;
                int px = (WINDOW_WIDTH - panel_w) / 2;
                int py = (WINDOW_HEIGHT - panel_h) / 2;

                fillroundrect(px, py, px + panel_w, py + panel_h, 15, 15);

                settextstyle(40, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
                settextcolor(RGB(255, 200, 50));
                LPCTSTR pause_text = _T("游 戏 暂 停");
                outtextxy((WINDOW_WIDTH - textwidth(pause_text)) / 2, py + 25, pause_text);

                settextstyle(18, 0, _T("黑体"), 0, 0, FW_NORMAL, false, false, false);
                settextcolor(RGB(200, 200, 200));
                LPCTSTR hint_p = _T("- 再按 P 键恢复游戏 -");
                outtextxy((WINDOW_WIDTH - textwidth(hint_p)) / 2, py + 80, hint_p);

                // 悬挂内嵌的鼠标滚轮音量交互修改显示进度条
                int vol_bar_w = 260;
                int vol_bar_h = 12;
                int vol_bar_x = (WINDOW_WIDTH - vol_bar_w) / 2;
                int vol_bar_y = py + 125;

                setlinecolor(RGB(50, 50, 50));
                setfillcolor(RGB(40, 40, 40));
                fillroundrect(vol_bar_x, vol_bar_y, vol_bar_x + vol_bar_w, vol_bar_y + vol_bar_h, 5, 5);

                if (current_volume > 0)
                {
                    int fill_w = (int)((double)current_volume / 1000.0 * vol_bar_w);
                    setfillcolor(RGB(100, 200, 255));
                    solidroundrect(vol_bar_x + 1, vol_bar_y + 1, vol_bar_x + fill_w - 1, vol_bar_y + vol_bar_h - 1, 5, 5);
                }

                TCHAR vol_text[64];
                _stprintf_s(vol_text, _T("当前音量: %d%%  (使用鼠标滚轮调节)"), current_volume / 10);
                settextstyle(16, 0, _T("黑体"), 0, 0, FW_NORMAL, false, false, false);
                settextcolor(RGB(150, 150, 150));
                outtextxy((WINDOW_WIDTH - textwidth(vol_text)) / 2, vol_bar_y + 25, vol_text);
            }

            // 绘制 Game Over 终结清算提示黑板
            if (is_game_over && (tick - game_over_time > 2000))
            {
                setlinecolor(RGB(0, 0, 0));
                setfillcolor(RGB(40, 40, 40));
                int panel_width = 400;
                int panel_height = 250;
                int px = (WINDOW_WIDTH - panel_width) / 2;
                int py = (WINDOW_HEIGHT - panel_height) / 2;
                fillroundrect(px, py, px + panel_width, py + panel_height, 20, 20);

                // 根据胜利与否赋色判决文案
                settextstyle(48, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
                LPCTSTR over_text;
                if (is_game_won) {
                    settextcolor(RGB(50, 255, 50));
                    over_text = _T("恭 喜 通 关 !");
                }
                else {
                    settextcolor(RGB(255, 80, 80));
                    over_text = _T("游 戏 结 束");
                }
                outtextxy(px + (panel_width - textwidth(over_text)) / 2, py + 40, over_text);

                // 战绩汇报文本展示
                TCHAR final_score_text[64];
                _stprintf_s(final_score_text, _T("最终分数：%d 分 | 存活：%d 秒"), score, survival_time_ms / 1000);
                settextstyle(28, 0, _T("微软雅黑"), 0, 0, FW_NORMAL, false, false, false);
                settextcolor(RGB(255, 200, 50));
                outtextxy(px + (panel_width - textwidth(final_score_text)) / 2, py + 120, final_score_text);

                settextstyle(18, 0, _T("黑体"), 0, 0, FW_NORMAL, false, false, false);
                settextcolor(RGB(180, 180, 180));
                LPCTSTR hint_text = _T("[R] 再来一局  |  [M] 回主菜单  |  [ESC] 退出");
                outtextxy(px + (panel_width - textwidth(hint_text)) / 2, py + 190, hint_text);
            }
        }
        else // === 主菜单界面的画面渲染层 ===
        {
            // 贴底层带有树林原画的壁纸
            putimage(0, 0, &img_menu);

            // 【架构重构】：利用多态机制遍历绘制所有按钮
            for (Button* btn : menu_buttons)
            {
                btn->Draw();
            }

            // 定义内联 Lambda 匿名极速工厂闭包：用来绘制带有角色高亮光环和具体配置项的高级展示橱窗 UI
            auto DrawPreviewUI = [&](int x, int y, CharacterType char_type, LPCTSTR label, LPCTSTR key_hint) {
                int base_center_x = x + 30;
                int base_center_y = y + 108;
                // 光头强的贴图骨骼尺寸原因，视觉重心需要微调硬编码补正
                if (char_type == CharacterType::GuangtouQiang) base_center_x -= 20;

                // 人物脚下底部的环形科幻选中光斑底座
                setlinecolor(RGB(255, 200, 50));
                setlinestyle(PS_SOLID, 2);
                ellipse(base_center_x - 45, base_center_y - 8, base_center_x + 45, base_center_y + 8);
                setlinecolor(RGB(200, 150, 20));
                setlinestyle(PS_SOLID, 1);
                ellipse(base_center_x - 35, base_center_y - 4, base_center_x + 35, base_center_y + 4);

                Player* disp = preview_players[(int)char_type];
                disp->SetPosition({ x, y });
                disp->Draw(0); // 传入 0 作为 delta，防止菜单界面的角色乱动或走状态机

                // 基于线段拼接实现带缺角的极简风全息科技感瞄准边框定位器
                int ui_x1 = x - 100, ui_y1 = y - 20, ui_x2 = x + 200, ui_y2 = y + 270;
                setlinecolor(RGB(255, 200, 50));
                setlinestyle(PS_SOLID, 3);
                int corner_len = 25;
                // 绘制左上角折线
                line(ui_x1, ui_y1, ui_x1 + corner_len, ui_y1);
                line(ui_x1, ui_y1, ui_x1, ui_y1 + corner_len);
                // 绘制右上角折线
                line(ui_x2 - corner_len, ui_y1, ui_x2, ui_y1);
                line(ui_x2, ui_y1, ui_x2, ui_y1 + corner_len);
                // 绘制左下角折线
                line(ui_x1, ui_y2 - corner_len, ui_x1, ui_y2);
                line(ui_x1, ui_y2, ui_x1 + corner_len, ui_y2);
                // 绘制右下角折线
                line(ui_x2, ui_y2 - corner_len, ui_x2, ui_y2);
                line(ui_x2 - corner_len, ui_y2, ui_x2, ui_y2);
                setlinestyle(PS_SOLID, 1);

                // 信息索引与字面量拼接格式化
                LPCTSTR name_text = _T("");
                LPCTSTR desc_text = _T("");
                if (char_type == CharacterType::XiongDa) {
                    name_text = _T("角色: 熊 大");
                    desc_text = _T("定位: 重装战士 | 高血量");
                }
                else if (char_type == CharacterType::XiongEr) {
                    name_text = _T("角色: 熊 二");
                    desc_text = _T("定位: 敏捷刺客 | 高移速");
                }
                else {
                    name_text = _T("角色: 光头强");
                    desc_text = _T("定位: 狂战士 | 高输出");
                }

                TCHAR full_name[64];
                _stprintf_s(full_name, _T("%s%s"), label, name_text);

                // 属性值通过深暗色叠底产生强对比描边，以解决在杂乱背景上看不清文本的问题
                settextstyle(26, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
                settextcolor(RGB(20, 20, 20));
                outtextxy(ui_x1 + 32, ui_y1 + 142, full_name);
                settextcolor(RGB(255, 200, 50));
                outtextxy(ui_x1 + 30, ui_y1 + 140, full_name);

                settextstyle(18, 0, _T("黑体"), 0, 0, FW_NORMAL, false, false, false);
                settextcolor(RGB(20, 20, 20));
                outtextxy(ui_x1 + 22, ui_y1 + 182, desc_text);
                settextcolor(RGB(230, 230, 230));
                outtextxy(ui_x1 + 20, ui_y1 + 180, desc_text);

                settextstyle(20, 0, _T("黑体"), 0, 0, FW_BOLD, false, false, false);
                settextcolor(RGB(20, 20, 20));
                outtextxy(ui_x1 + 12, ui_y1 + 227, key_hint);
                settextcolor(RGB(100, 255, 100));
                outtextxy(ui_x1 + 10, ui_y1 + 225, key_hint);
                };

            // 直接调用 Lambda 实现对左右两侧玩家卡牌选修展板的对称化生成渲染
            DrawPreviewUI(140, 60, selected_character, _T("1P"), _T("[TAB键] 切换 1P 角色"));
            if (game_mode == 2) {
                DrawPreviewUI(950, 60, selected_character_p2, _T("2P"), _T("[ Q 键] 切换 2P 角色"));
            }

            // 1. 在左侧玩家面板底部提供明确的当前游玩人数环境展示与指示词
            LPCTSTR mode_txt = game_mode == 1 ? _T("当前模式：单机割草") : _T("当前模式：双人同屏合作");
            settextstyle(26, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
            setbkmode(TRANSPARENT);
            settextcolor(RGB(20, 20, 20));
            outtextxy(52, 352, mode_txt);
            settextcolor(game_mode == 1 ? RGB(100, 255, 100) : RGB(255, 150, 50));
            outtextxy(50, 350, mode_txt);

            // 2. 提供 Y 键模式切换器反馈的文字说明与指引
            settextstyle(20, 0, _T("黑体"), 0, 0, FW_BOLD, false, false, false);
            settextcolor(RGB(20, 20, 20));
            outtextxy(52, 392, _T("[按下 Y 键切换单/双人模式]"));
            settextcolor(RGB(100, 200, 255));
            outtextxy(50, 390, _T("[按下 Y 键切换单/双人模式]"));

            // 绘制左上侧悬空的难度调节交互区与方块按钮
            int diff_btn_w = 160;
            int diff_btn_h = 50;
            int diff_btn_x = WINDOW_WIDTH - diff_btn_w - 40;
            int diff_btn_y = 40;

            setlinecolor(RGB(200, 150, 50));
            setlinestyle(PS_SOLID, 2);
            setfillcolor(RGB(40, 40, 40));
            fillroundrect(diff_btn_x, diff_btn_y, diff_btn_x + diff_btn_w, diff_btn_y + diff_btn_h, 10, 10);
            setlinestyle(PS_SOLID, 1);

            LPCTSTR diff_text = _T("");
            COLORREF diff_color = RGB(255, 255, 255);
            // 将难度逻辑反馈到前段颜色，辅助加强认知交互：绿->黄->红 危险梯度上升
            if (current_difficulty == Difficulty::Easy) {
                diff_text = _T("难度: 简 单");
                diff_color = RGB(100, 255, 100);
            }
            else if (current_difficulty == Difficulty::Normal) {
                diff_text = _T("难度: 普 通");
                diff_color = RGB(255, 200, 50);
            }
            else {
                diff_text = _T("难度: 困 难");
                diff_color = RGB(255, 80, 80);
            }

            settextstyle(22, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
            setbkmode(TRANSPARENT);
            int tw = textwidth(diff_text);
            int th = textheight(diff_text);
            int tx = diff_btn_x + (diff_btn_w - tw) / 2;
            int ty = diff_btn_y + (diff_btn_h - th) / 2;

            settextcolor(RGB(20, 20, 20));
            outtextxy(tx + 2, ty + 2, diff_text);
            settextcolor(diff_color);
            outtextxy(tx, ty, diff_text);

            // 在最底端的横切向中轴绘制基于本地存档 IO 取出的大满贯荣誉信息
            TCHAR history_text[128];
            _stprintf_s(history_text, _T(" 历史最高纪录 - 分数: %d 分   存活: %d 秒 👑"), high_score, high_survival_time);

            settextstyle(24, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
            setbkmode(TRANSPARENT);

            int hw = textwidth(history_text);
            int hx = (WINDOW_WIDTH - hw) / 2;
            int hy = WINDOW_HEIGHT - 60;

            settextcolor(RGB(20, 20, 20));
            outtextxy(hx + 2, hy + 2, history_text);
            settextcolor(RGB(255, 215, 0));
            outtextxy(hx, hy, history_text);
        }

        // ================= 帧结尾操作：将后台构建好的整张画布显式投屏 =================
        FlushBatchDraw();

        // 帧率锁 (FPS Limiter)
        // 提取本帧总计耗费计算时间差，若系统处理过快导致耗时小于标定值 (即 144Hz 每帧约 6.94 毫秒)
        // 则强制调用系统级别 API 进行线程休眠挂盖等待补充，以防画面撕裂与逻辑倍速运行
        DWORD end_time = GetTickCount();
        DWORD delta_time = end_time - start_time;
        if (delta_time < 1000 / 144)
        {
            Sleep(1000 / 144 - delta_time);
        }
    }

    // 主循环结束，进行善后与析构对象
    delete atlas_xiongda_left;  
    delete atlas_xiongda_right;
    delete atlas_xionger_left;  
    delete atlas_xionger_right;
    delete atlas_qiang_left;    
    delete atlas_qiang_right;
    delete atlas_enemy_left;    
    delete atlas_enemy_right;
    delete atlas_machine_left; 
    delete atlas_machine_right;
    delete atlas_boss_left;     
    delete atlas_boss_right;

    for (Player* p : players) 
        delete p;
    players.clear();
    for (Enemy* enemy : enemy_list) 
        delete enemy;
    enemy_list.clear();

    // 释放预览池
    for (int i = 0; i < 3; i++) {
        delete preview_players[i];
    }

    // 触发多态销毁，安全释放堆区内存
    for (Button* btn : menu_buttons)
    {
        delete btn;
    }
    menu_buttons.clear();
    
    mciSendString(_T("close all"), NULL, 0, NULL);

    EndBatchDraw();
    return 0;
}