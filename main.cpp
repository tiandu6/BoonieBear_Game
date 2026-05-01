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

// 分配内存经验单元实体构型
struct ExpDrop
{
    double x, y;
    bool active = true;
};

// 实例化的全局骨骼序列图集指针引用
Atlas* atlas_player_left = nullptr;
Atlas* atlas_player_right = nullptr;
Atlas* atlas_enemy_left = nullptr;
Atlas* atlas_enemy_right = nullptr;

// 动态控制分配调度敌对派生实体的生成频率与分布比例
void TryGenerateEnemy(vector<Enemy*>& enemy_list, int current_phase, bool& boss_spawned)
{
    int interval = 150;
    size_t max_enemies = 8;
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

    if (current_phase == 3 && !boss_spawned) {
        enemy_list.push_back(new BossEnemy());
        boss_spawned = true;
    }

    if (enemy_list.size() >= max_enemies) return;

    static int counter = 0;
    if ((++counter) % interval == 0)
    {
        if (current_phase >= 2 && (rand() % 100 < 30)) {
            enemy_list.push_back(new MachineEnemy());
        }
        else {
            enemy_list.push_back(new NormalEnemy());
        }
    }
}


// 分层驱动绘制界面数据叠加态 HUD 面板内容
void DrawHUD(int score, int hp, int max_hp, int survival_time_sec)
{
    static TCHAR score_text[64];
    _stprintf_s(score_text, _T("当前分数：%d"), score);

    settextstyle(32, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
    setbkmode(TRANSPARENT);

    settextcolor(RGB(50, 50, 50));
    outtextxy(17, 17, score_text);
    settextcolor(RGB(255, 180, 20));
    outtextxy(15, 15, score_text);

    static TCHAR time_text[64];
    _stprintf_s(time_text, _T("存活时间：%d 秒"), survival_time_sec);
    settextstyle(24, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
    settextcolor(RGB(50, 50, 50));
    outtextxy(17, 57, time_text);
    settextcolor(RGB(150, 200, 255));
    outtextxy(15, 55, time_text);

    int bar_x = 15;
    int bar_y = 95;
    int bar_width = 300;
    int bar_height = 28;

    setlinecolor(RGB(30, 30, 30));
    setfillcolor(RGB(50, 50, 50));
    fillroundrect(bar_x, bar_y, bar_x + bar_width, bar_y + bar_height, 8, 8);

    if (hp > 0)
    {
        int fill_width = (int)((double)hp / max_hp * bar_width);

        COLORREF hp_color;
        double hp_ratio = (double)hp / max_hp;

        if (hp_ratio > 0.5)
            hp_color = RGB(50, 220, 50);
        else if (hp_ratio > 0.25)
            hp_color = RGB(255, 200, 50);
        else
            hp_color = RGB(255, 50, 50);

        setfillcolor(hp_color);
        solidroundrect(bar_x + 1, bar_y + 1, bar_x + fill_width - 1, bar_y + bar_height - 1, 8, 8);
    }

    static TCHAR hp_text[64];
    _stprintf_s(hp_text, _T("HP: %d / %d"), hp, max_hp);

    settextstyle(20, 0, _T("Arial"), 0, 0, FW_BOLD, false, false, false);

    int text_w = textwidth(hp_text);
    int text_h = textheight(hp_text);
    int text_x = bar_x + (bar_width - text_w) / 2;
    int text_y = bar_y + (bar_height - text_h) / 2;

    settextcolor(RGB(30, 30, 30));
    outtextxy(text_x + 2, text_y + 2, hp_text);
    settextcolor(RGB(255, 255, 255));
    outtextxy(text_x, text_y, hp_text);

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

int main()
{
    // 初始化图形展示运行层画布界面空间配置
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);

    // 初始化并读取图形图集素材贴图加载工作组
    atlas_xiongda_right = new Atlas(_T("img/XiongDa_right_%d.png"), 8, 150, 150, false);
    atlas_xiongda_left = new Atlas(_T("img/XiongDa_right_%d.png"), 8, 150, 150, true);
    atlas_xionger_right = new Atlas(_T("img/XiongEr_right_%d.png"), 8, 150, 150, false);
    atlas_xionger_left = new Atlas(_T("img/XiongEr_right_%d.png"), 8, 150, 150, true);
    atlas_qiang_right = new Atlas(_T("img/GuangtouQiang_right_%d.png"), 8, 150, 150, false);
    atlas_qiang_left = new Atlas(_T("img/GuangtouQiang_right_%d.png"), 8, 150, 150, true);

    atlas_enemy_right = new Atlas(_T("img/LoggingWorker_right_%d.png"), 8, 80, 80, false);
    atlas_enemy_left = new Atlas(_T("img/LoggingWorker_right_%d.png"), 8, 80, 80, true);
    atlas_machine_right = new Atlas(_T("img/MechanicalLoggingWorker_right_%d.png"), 8, 120, 120, false);
    atlas_machine_left = new Atlas(_T("img/MechanicalLoggingWorker_right_%d.png"), 8, 120, 120, true);
    atlas_boss_right = new Atlas(_T("img/Boss_right_%d.png"), 8, 200, 200, false);
    atlas_boss_left = new Atlas(_T("img/Boss_right_%d.png"), 8, 200, 200, true);

    loadimage(&img_bullet_right, _T("img/bullet_right.png"), 25, 10, true);

    // 内存反向投影构建子弹数据阵列图像
    img_bullet_left = img_bullet_right;

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

    // 装载底层多媒体播放控制项命令流
    mciSendString(_T("open mus/bgm.mp3 alias bgm"), NULL, 0, NULL);
    mciSendString(_T("open mus/hit.mp3 alias hit"), NULL, 0, NULL);
    mciSendString(_T("setaudio bgm volume to 500"), NULL, 0, NULL);

    mciSendString(_T("play bgm repeat"), NULL, 0, NULL);

    int score = 0;
    Player* player = new XiongEr();
    ExMessage msg;
    IMAGE img_menu;
    IMAGE img_background;
    vector<Enemy*> enemy_list;

    // 初始化注册响应交互区域矩形控制块
    RECT region_btn_start_game, region_btn_quit_game;
    region_btn_start_game.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2;
    region_btn_start_game.right = region_btn_start_game.left + BUTTON_WIDTH;
    region_btn_start_game.top = 430;
    region_btn_start_game.bottom = region_btn_start_game.top + BUTTON_HEIGHT;

    region_btn_quit_game.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2;
    region_btn_quit_game.right = region_btn_quit_game.left + BUTTON_WIDTH;
    region_btn_quit_game.top = 550;
    region_btn_quit_game.bottom = region_btn_quit_game.top + BUTTON_HEIGHT;

    StartGameButton btn_start_game = StartGameButton(region_btn_start_game, _T("img/ui_start_idle.png"),
        _T("img/ui_start_hovered.png"), _T("img/ui_start_pushed.png"));
    QuitGameButton btn_quit_game = QuitGameButton(region_btn_quit_game, _T("img/ui_quit_idle.png"),
        _T("img/ui_quit_hovered.png"), _T("img/ui_quit_pushed.png"));

    loadimage(&img_menu, _T("img/menu.png"));
    loadimage(&img_background, _T("img/background.png"));

    // 启用帧缓冲以抹平帧间撕裂等渲染同步问题
    BeginBatchDraw();

    bool is_game_paused = false;
    DWORD pause_start_time = 0;
    DWORD level_up_start_time = 0;
    static DWORD game_over_time = 0;

    // 构建分配卡牌卡池及强化属性配置管理器
    vector<ExpDrop> exp_list;
    bool is_leveling_up = false;
    int current_upgrade_options[3];

    LoadGameData();
    int survival_time_ms = 0;

    int current_phase = 0;
    DWORD phase_announce_time = 0;
    bool is_game_won = false;
    bool boss_spawned = false;

    // 开始接管执行应用程序的活动主循环调度系统
    while (running)
    {
        DWORD start_time = GetTickCount();

        // 处理执行排队拦截分配及提取消息监听机制
        while (peekmessage(&msg))
        {
            if (msg.message == WM_KEYDOWN && msg.vkcode == 'P')
            {
                if (is_game_started && !is_game_over && !is_leveling_up)
                {
                    is_game_paused = !is_game_paused;
                    if (is_game_paused)
                    {
                        pause_start_time = GetTickCount();
                    }
                    else
                    {
                        DWORD pause_duration = GetTickCount() - pause_start_time;
                        player->AddPauseTime(pause_duration);
                        if (phase_announce_time > 0) phase_announce_time += pause_duration;
                    }
                }
            }

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

                    TCHAR cmd[64];
                    _stprintf_s(cmd, _T("setaudio bgm volume to %d"), current_volume);
                    mciSendString(cmd, NULL, 0, NULL);
                }
            }

            if (msg.message == WM_KEYDOWN && msg.vkcode == VK_TAB)
            {
                if (!is_game_started)
                {
                    POINT old_pos = player->GetPosition();

                    delete player;

                    int next_char = ((int)selected_character + 1) % 3;
                    selected_character = (CharacterType)next_char;

                    if (selected_character == CharacterType::XiongDa)
                        player = new XiongDa();
                    else if (selected_character == CharacterType::XiongEr)
                        player = new XiongEr();
                    else
                        player = new GuangtouQiang();

                    player->SetPosition(old_pos);
                }
            }

            if (msg.message == WM_LBUTTONDOWN)
            {
                if (is_leveling_up)
                {
                    int card_w = 220, card_h = 320, gap = 50;
                    int start_x = (WINDOW_WIDTH - (3 * card_w + 2 * gap)) / 2;
                    int start_y = (WINDOW_HEIGHT - card_h) / 2;

                    for (int i = 0; i < 3; i++)
                    {
                        int cx = start_x + i * (card_w + gap);
                        if (msg.x >= cx && msg.x <= cx + card_w && msg.y >= start_y && msg.y <= start_y + card_h)
                        {
                            player->ApplyUpgrade(current_upgrade_options[i]);
                            is_leveling_up = false;

                            DWORD pause_duration = GetTickCount() - level_up_start_time;
                            player->AddPauseTime(pause_duration);
                            if (phase_announce_time > 0) phase_announce_time += pause_duration;
                            break;
                        }
                    }
                }

                if (is_game_started && !is_game_over && !is_leveling_up)
                {
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
                            player->AddPauseTime(pause_duration);
                            if (phase_announce_time > 0) phase_announce_time += pause_duration;
                        }
                    }
                }

                else if (!is_game_started)
                {
                    btn_start_game.ProcessEvent(msg);
                    btn_quit_game.ProcessEvent(msg);

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

            if (is_game_started && !is_game_over)
            {
                player->ProcessEvent(msg);

                if (msg.message == WM_KEYDOWN && msg.vkcode == VK_SPACE)
                {
                    if (!is_game_paused && !is_leveling_up)
                    {
                        DWORD current_time = GetTickCount();
                        if (current_time - player->last_skill_time >= player->SKILL_CD)
                        {
                            player->UseSkill(enemy_list);
                            player->last_skill_time = current_time;
                        }
                    }
                }
            }
            else if (!is_game_started)
            {
                btn_start_game.ProcessEvent(msg);
                btn_quit_game.ProcessEvent(msg);
            }
        }

        // 停止运算暂停界面的物理计算和逻辑推导状态挂载
        if (is_game_started && !is_game_over && !is_game_paused && !is_leveling_up)
        {
            survival_time_ms += (1000 / 144);

            // 进行自动抓取收集物体的引力系统寻路计算判定
            for (size_t i = 0; i < exp_list.size(); i++)
            {
                if (!exp_list[i].active) continue;

                double dx = (player->GetPosition().x + player->FRAME_WIDTH / 2) - exp_list[i].x;
                double dy = (player->GetPosition().y + player->FRAME_HEIGHT / 2) - exp_list[i].y;
                double dist = sqrt(dx * dx + dy * dy);

                if (dist < 180)
                {
                    exp_list[i].x += (dx / dist) * 10.0;
                    exp_list[i].y += (dy / dist) * 10.0;
                }
                if (dist < 35)
                {
                    exp_list[i].active = false;
                    player->exp += 1;
                }
            }

            for (size_t i = 0; i < exp_list.size(); i++) {
                if (!exp_list[i].active) {
                    swap(exp_list[i], exp_list.back());
                    exp_list.pop_back();
                    i--;
                }
            }

            if (player->exp >= player->max_exp)
            {
                player->exp -= player->max_exp;
                player->level++;
                player->max_exp += 3;

                is_leveling_up = true;
                level_up_start_time = GetTickCount();

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

            int target_phase = 1;
            if (survival_time_ms >= 150000) target_phase = 3;
            else if (survival_time_ms >= 60000) target_phase = 2;

            if (target_phase > current_phase) {
                current_phase = target_phase;
                phase_announce_time = GetTickCount();
            }

            player->Move();
            player->UpdateAttacks();
            player->UpdateExtraSkills(enemy_list);
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
                    double diff_multiplier = 1.0;

                    if (current_difficulty == Difficulty::Easy)
                        diff_multiplier = 0.6;
                    else if (current_difficulty == Difficulty::Normal)
                        diff_multiplier = 1.0;
                    else if (current_difficulty == Difficulty::Hard)
                        diff_multiplier = 2.0;

                    int dmg = (int)(enemy->collision_damage * diff_multiplier);

                    player->TakeDamage(dmg);

                    if (player->GetHP() <= 0 && !is_game_over)
                    {
                        is_game_over = true;
                        game_over_time = GetTickCount();
                        SaveGameData(score, survival_time_ms / 1000);
                    }
                }
            }

            for (Enemy* enemy : enemy_list)
            {
                for (const Bullet& bullet : player->GetBullets())
                {
                    if (enemy->CheckBulletCollision(bullet))
                    {
                        if (enemy->Hurt(player->GetAttackDamage(), bullet.pos))
                        {
                            mciSendString(_T("play hit from 0"), NULL, 0, NULL);
                        }
                    }
                }
            }

            for (size_t i = 0; i < enemy_list.size(); i++)
            {
                Enemy* enemy = enemy_list[i];
                if (!enemy->CheckAlive())
                {
                    if (enemy->is_boss)
                    {
                        is_game_won = true;
                        is_game_over = true;
                        game_over_time = GetTickCount();
                        SaveGameData(score, survival_time_ms / 1000);

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
                        score++;

                        ExpDrop exp_drop;
                        exp_drop.x = enemy->GetPosition().x + 40;
                        exp_drop.y = enemy->GetPosition().y + 40;
                        exp_list.push_back(exp_drop);
                    }

                    swap(enemy_list[i], enemy_list.back());
                    enemy_list.pop_back();
                    delete enemy;
                    i--;
                }
            }
        }

        else if (is_game_over)
        {
            if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
            {
                running = false;
            }
            if (GetAsyncKeyState('R') & 0x8000)
            {
                is_game_over = false;
                is_game_paused = false;
                score = 0;
                survival_time_ms = 0;
                for (Enemy* enemy : enemy_list) delete enemy;
                enemy_list.clear();
                player->Reset();
                Sleep(150);

                current_phase = 0;
                phase_announce_time = 0;
                is_game_won = false;
                boss_spawned = false;

                exp_list.clear();
                is_leveling_up = false;
            }
            if (GetAsyncKeyState('M') & 0x8000)
            {
                is_game_started = false;
                is_game_over = false;
                is_game_paused = false;
                score = 0;
                survival_time_ms = 0;
                for (Enemy* enemy : enemy_list) delete enemy;
                enemy_list.clear();
                player->Reset();
                Sleep(150);

                current_phase = 0;
                phase_announce_time = 0;
                is_game_won = false;
                boss_spawned = false;

                exp_list.clear();
                is_leveling_up = false;
            }
        }

        // 接管渲染管线的图层执行并控制图层叠加先后状态优先级顺序
        cleardevice();
        if (is_game_started)
        {
            putimage(0, 0, &img_background);

            int current_delta = (is_game_paused || is_leveling_up) ? 0 : (1000 / 144);

            DWORD tick = GetTickCount();
            for (const ExpDrop& e : exp_list)
            {
                int breath = (tick / 100) % 4;

                setlinecolor(RGB(170, 230, 255));
                setfillcolor(RGB(170, 230, 255));
                solidcircle((int)e.x, (int)e.y, 8 + breath);

                setfillcolor(RGB(220, 245, 255));
                solidcircle((int)e.x, (int)e.y, 5 + breath / 2);

                setfillcolor(RGB(255, 255, 255));
                solidcircle((int)e.x, (int)e.y, 2);
            }

            player->Draw(current_delta);

            for (Enemy* enemy : enemy_list)
            {
                if (is_game_over && is_game_won && (GetTickCount() - game_over_time > 1500))
                {
                    continue;
                }
                enemy->Draw(current_delta);
            }

            player->DrawAttacks();
            player->orbital_skill.Draw(is_game_paused, pause_start_time);

            if (is_game_over && is_game_won && (GetTickCount() - game_over_time < 1500))
            {
                DWORD elapsed = GetTickCount() - game_over_time;
                double progress = elapsed / 1500.0;

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

                setlinestyle(PS_SOLID, 1);
            }

            DrawHUD(score, player->GetHP(), player->GetMaxHP(), survival_time_ms / 1000);

            int exp_bar_w = 300;
            int exp_bar_h = 16;
            int exp_bar_x = 15;
            int exp_bar_y = 135;

            setlinecolor(RGB(50, 50, 50));
            setfillcolor(RGB(30, 30, 30));
            fillroundrect(exp_bar_x, exp_bar_y, exp_bar_x + exp_bar_w, exp_bar_y + exp_bar_h, 3, 3);

            if (player->exp > 0)
            {
                int exp_fill = (int)((double)player->exp / player->max_exp * exp_bar_w);
                setfillcolor(RGB(0, 180, 255));
                solidroundrect(exp_bar_x + 1, exp_bar_y + 1, exp_bar_x + exp_fill - 1, exp_bar_y + exp_bar_h - 1, 3, 3);
            }

            TCHAR lvl_text[32];
            _stprintf_s(lvl_text, _T("Lv.%d"), player->level);
            settextstyle(16, 0, _T("黑体"), 0, 0, FW_BOLD, false, false, false);

            settextcolor(RGB(20, 20, 20));
            outtextxy(exp_bar_x + 8 + 1, exp_bar_y + 1, lvl_text);
            settextcolor(RGB(255, 215, 0));
            outtextxy(exp_bar_x + 8, exp_bar_y, lvl_text);

            settextcolor(RGB(20, 20, 20));
            outtextxy(exp_bar_x + exp_bar_w - 35 + 1, exp_bar_y + 1, _T("EXP"));
            settextcolor(RGB(150, 200, 255));
            outtextxy(exp_bar_x + exp_bar_w - 35, exp_bar_y, _T("EXP"));

            int prog_bar_w = 500;
            int prog_bar_h = 16;
            int prog_bar_x = (WINDOW_WIDTH - prog_bar_w) / 2;
            int prog_bar_y = 40;

            double phase_progress = 0.0;
            LPCTSTR phase_target_text = _T("");
            int time_left_sec = 0;

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

            settextcolor(RGB(30, 30, 30));
            outtextxy(tx - 1, ty - 1, prog_text);
            outtextxy(tx + 1, ty - 1, prog_text);
            outtextxy(tx - 1, ty + 1, prog_text);
            outtextxy(tx + 1, ty + 1, prog_text);

            outtextxy(tx, ty + 2, prog_text);

            settextcolor(RGB(255, 220, 50));
            outtextxy(tx, ty, prog_text);

            if (is_leveling_up)
            {
                int panel_w = 900, panel_h = 500;
                int px = (WINDOW_WIDTH - panel_w) / 2;
                int py = (WINDOW_HEIGHT - panel_h) / 2;

                setlinecolor(RGB(200, 150, 50));
                setlinestyle(PS_SOLID, 2);
                setfillcolor(RGB(30, 30, 30));
                fillroundrect(px, py, px + panel_w, py + panel_h, 15, 15);
                setlinestyle(PS_SOLID, 1);

                settextstyle(42, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
                settextcolor(RGB(255, 200, 50));
                LPCTSTR up_title = _T("等级提升！请选择一项强化：");
                outtextxy((WINDOW_WIDTH - textwidth(up_title)) / 2, py + 30, up_title);

                int card_w = 220, card_h = 320, gap = 50;
                int start_x = px + (panel_w - (3 * card_w + 2 * gap)) / 2;
                int start_y = py + 120;

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
                    else if (opt == 4)
                    {
                        title = _T("缩减cd");
                        desc = _T("技能冷却减少 20%");
                        c_color = RGB(200, 100, 255);
                    }
                    else if (opt == 5)
                    {
                        title = _T("钢铁之躯");
                        desc = _T("减免 20% 受到伤害");
                        c_color = RGB(180, 180, 180);
                    }
                    else if (opt == 6)
                    {
                        title = _T("天降正义");

                        static TCHAR orbital_desc[64];

                        if (player->orbital_skill.level == 0)
                        {
                            desc = _T("自动召唤高伤害落雷");
                        }
                        else
                        {
                            _stprintf_s(orbital_desc, _T("升级落雷 (Lv.%d) : 伤害+ 冷却-"), player->orbital_skill.level + 1);
                            desc = orbital_desc;
                        }
                        c_color = RGB(0, 255, 255);
                    }

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

            DWORD current_time = GetTickCount();
            if (is_game_paused)
            {
                current_time = pause_start_time;
            }

            DWORD elapsed = current_time - player->last_skill_time;

            int cd_w = 260;
            int cd_h = 26;
            int cd_x = (WINDOW_WIDTH - cd_w) / 2;
            int cd_y = WINDOW_HEIGHT - 40;

            setlinecolor(RGB(50, 50, 50));
            setfillcolor(RGB(30, 30, 30));
            fillroundrect(cd_x, cd_y, cd_x + cd_w, cd_y + cd_h, 5, 5);

            if (elapsed >= player->SKILL_CD)
            {
                setfillcolor(RGB(50, 255, 255));
                solidroundrect(cd_x + 1, cd_y + 1, cd_x + cd_w - 1, cd_y + cd_h - 1, 5, 5);
                settextstyle(16, 0, _T("黑体"), 0, 0, FW_BOLD, false, false, false);
                settextcolor(RGB(0, 0, 0));

                LPCTSTR ready_text = _T("");
                if (selected_character == CharacterType::XiongDa)
                {
                    ready_text = _T("[空格键] 释放：熊掌拍击");
                }
                else if (selected_character == CharacterType::XiongEr)
                {
                    ready_text = _T("[空格键] 释放：蜂蜜回血");
                }
                else
                {
                    ready_text = _T("[空格键] 释放：火力全开");
                }

                int tx = cd_x + (cd_w - textwidth(ready_text)) / 2;
                outtextxy(tx, cd_y + 5, ready_text);
            }
            else
            {
                int fill_w = (int)((double)elapsed / player->SKILL_CD * cd_w);

                setfillcolor(RGB(150, 100, 50));
                solidroundrect(cd_x + 1, cd_y + 1, cd_x + fill_w - 1, cd_y + cd_h - 1, 5, 5);
                settextstyle(16, 0, _T("黑体"), 0, 0, FW_BOLD, false, false, false);
                settextcolor(RGB(200, 200, 200));

                LPCTSTR cd_text = _T("技能充能中...");
                int tx = cd_x + (cd_w - textwidth(cd_text)) / 2;
                outtextxy(tx, cd_y + 5, cd_text);
            }

            if (current_phase > 0 && (GetTickCount() - phase_announce_time < 3500) && !is_game_over && !is_leveling_up)
            {
                DWORD elapsed = GetTickCount() - phase_announce_time;
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

            if (is_game_over && (GetTickCount() - game_over_time > 2000))
            {
                setlinecolor(RGB(0, 0, 0));
                setfillcolor(RGB(40, 40, 40));
                int panel_width = 400;
                int panel_height = 250;
                int px = (WINDOW_WIDTH - panel_width) / 2;
                int py = (WINDOW_HEIGHT - panel_height) / 2;
                fillroundrect(px, py, px + panel_width, py + panel_height, 20, 20);

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
        else
        {
            putimage(0, 0, &img_menu);
            btn_start_game.Draw();
            btn_quit_game.Draw();


            int preview_x = 140;
            int preview_y = 60;

            int base_center_x = preview_x + 30;
            int base_center_y = preview_y + 108;

            if (selected_character == CharacterType::GuangtouQiang)
            {
                base_center_x -= 20;
            }

            setlinecolor(RGB(255, 200, 50));
            setlinestyle(PS_SOLID, 2);
            ellipse(base_center_x - 45, base_center_y - 8, base_center_x + 45, base_center_y + 8);

            setlinecolor(RGB(200, 150, 20));
            setlinestyle(PS_SOLID, 1);
            ellipse(base_center_x - 35, base_center_y - 4, base_center_x + 35, base_center_y + 4);

            POINT old_pos = player->GetPosition();
            player->SetPosition({ preview_x, preview_y });
            player->Draw(1000 / 144);
            player->SetPosition(old_pos);

            int ui_x1 = 40, ui_y1 = 40, ui_x2 = 340, ui_y2 = 300;
            setlinecolor(RGB(255, 200, 50));
            setlinestyle(PS_SOLID, 3);
            int corner_len = 25;

            line(ui_x1, ui_y1, ui_x1 + corner_len, ui_y1);
            line(ui_x1, ui_y1, ui_x1, ui_y1 + corner_len);
            line(ui_x2 - corner_len, ui_y1, ui_x2, ui_y1);
            line(ui_x2, ui_y1, ui_x2, ui_y1 + corner_len);
            line(ui_x1, ui_y2 - corner_len, ui_x1, ui_y2);
            line(ui_x1, ui_y2, ui_x1 + corner_len, ui_y2);
            line(ui_x2, ui_y2 - corner_len, ui_x2, ui_y2);
            line(ui_x2 - corner_len, ui_y2, ui_x2, ui_y2);

            setlinestyle(PS_SOLID, 1);

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

            setbkmode(TRANSPARENT);

            settextstyle(28, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
            settextcolor(RGB(20, 20, 20));
            outtextxy(72, 192, name_text);
            settextcolor(RGB(255, 200, 50));
            outtextxy(70, 190, name_text);

            settextstyle(18, 0, _T("黑体"), 0, 0, FW_NORMAL, false, false, false);
            settextcolor(RGB(20, 20, 20));
            outtextxy(62, 232, desc_text);
            settextcolor(RGB(230, 230, 230));
            outtextxy(60, 230, desc_text);

            settextstyle(20, 0, _T("黑体"), 0, 0, FW_BOLD, false, false, false);
            settextcolor(RGB(20, 20, 20));
            outtextxy(52, 277, _T("[按下 TAB 键切换角色]"));
            settextcolor(RGB(100, 255, 100));
            outtextxy(50, 275, _T("[按下 TAB 键切换角色]"));

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

            TCHAR history_text[128];
            _stprintf_s(history_text, _T(" 历史最高纪录 - 分数: %d 罐   存活: %d 秒 👑"), high_score, high_survival_time);

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

        FlushBatchDraw();

        DWORD end_time = GetTickCount();
        DWORD delta_time = end_time - start_time;
        if (delta_time < 1000 / 144)
        {
            Sleep(1000 / 144 - delta_time);
        }
    }

    delete atlas_player_left;
    delete atlas_player_right;
    delete atlas_enemy_left;
    delete atlas_enemy_right;

    EndBatchDraw();
    return 0;
}