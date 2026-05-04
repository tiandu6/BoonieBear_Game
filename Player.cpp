#include "Player.h"

Player::Player()
{
    // 初始化技能时钟偏置，保证开局进场能无视 CD 立刻发起第一次技能判定释放
    last_skill_time = GetTickCount() - SKILL_CD;
}

Player::~Player()
{
    if (anim_left) delete anim_left;
    if (anim_right) delete anim_right;
    if (current_weapon) delete current_weapon;
}

// 复合状态集事件侦听与键位路由分发器
// 【重构】：运用 current_game_mode 参数对输入设备实施硬件隔离分频探测，防单双设备通道串扰
void Player::ProcessEvent(const ExMessage& msg, int current_game_mode)
{
    // ================== 下推压燃映射矩阵 ==================
    if (msg.message == WM_KEYDOWN)
    {
        // 1P 键桥映射：使用传统的左侧 WASD 控制盘 (在单机模式下也放行全局上下左右做替补操作)
        if (current_game_mode == 1 || player_id == 1)
        {
            switch (msg.vkcode) {
            case 'W': is_move_up = true; break;
            case 'S': is_move_down = true; break;
            case 'A': is_move_left = true; break;
            case 'D': is_move_right = true; break;
            case VK_UP: if (current_game_mode == 1) is_move_up = true; break;
            case VK_DOWN: if (current_game_mode == 1) is_move_down = true; break;
            case VK_LEFT: if (current_game_mode == 1) is_move_left = true; break;
            case VK_RIGHT: if (current_game_mode == 1) is_move_right = true; break;
            }
        }
        // 2P 键桥映射：专机独立抽调采用右侧方向键盘操作驱动，严密封锁防乱窜
        if (current_game_mode == 2 && player_id == 2)
        {
            switch (msg.vkcode) {
            case VK_UP: is_move_up = true; break;
            case VK_DOWN: is_move_down = true; break;
            case VK_LEFT: is_move_left = true; break;
            case VK_RIGHT: is_move_right = true; break;
            }
        }
    }
    // ================== 弹起释放解除矩阵 ==================
    else if (msg.message == WM_KEYUP)
    {
        if (current_game_mode == 1 || player_id == 1)
        {
            switch (msg.vkcode) {
            case 'W': is_move_up = false; break;
            case 'S': is_move_down = false; break;
            case 'A': is_move_left = false; break;
            case 'D': is_move_right = false; break;
            case VK_UP: if (current_game_mode == 1) is_move_up = false; break;
            case VK_DOWN: if (current_game_mode == 1) is_move_down = false; break;
            case VK_LEFT: if (current_game_mode == 1) is_move_left = false; break;
            case VK_RIGHT: if (current_game_mode == 1) is_move_right = false; break;
            }
        }
        if (current_game_mode == 2 && player_id == 2)
        {
            switch (msg.vkcode) {
            case VK_UP: is_move_up = false; break;
            case VK_DOWN: is_move_down = false; break;
            case VK_LEFT: is_move_left = false; break;
            case VK_RIGHT: is_move_right = false; break;
            }
        }
    }
}

// 将按键布尔布防器转译处理为 2D 正交基础物理平面运动矢量并向内反馈应用
void Player::Move()
{
    // 死后拦截物理移动逻辑防诈尸
    if (is_dead) return;

    // 数学布尔运算提取单向坐标极性：(-1 / 0 / 1) 
    int dir_x = is_move_right - is_move_left;
    int dir_y = is_move_down - is_move_up;

    // 通过平方开根抽取二维斜边对角方向力距长度模型
    double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
    if (len_dir != 0)
    {
        // 若产生了合力，执行向量归一化操作防止由勾股定理导致的斜向走位超速 BUG
        double normalized_x = dir_x / len_dir;
        double normalized_y = dir_y / len_dir;

        // 绑定固有移动速率推算帧下发物理像素坐标
        pos.x += (int)(speed * normalized_x);
        pos.y += (int)(speed * normalized_y);
    }

    // 采用硬边界包围盒进行边界碰撞处理强制复位钳制
    if (pos.x < 0) pos.x = 0;
    if (pos.y < 0) pos.y = 0;
    if (pos.x + FRAME_WIDTH > WINDOW_WIDTH) pos.x = WINDOW_WIDTH - FRAME_WIDTH;
    if (pos.y + FRAME_HEIGHT > WINDOW_HEIGHT) pos.y = WINDOW_HEIGHT - FRAME_HEIGHT;
}

// 玩家总控制台级联渲染流
void Player::Draw(int delta)
{
    DWORD current_time = GetTickCount();
    int draw_x = pos.x - 35;
    int draw_y = pos.y - 35;

    // 【重要核心限制】：死亡流程保护判断期，3.5 秒（3500毫秒）彻底执行强制销毁截断退出逻辑
    // 以防止留存在场地内的死尸遮挡活体操作或污染视野
    if (is_dead)
    {
        if (current_time - death_start_time > 3500)
        {
            return;
        }
    }

    // 当获取了移动增量下发指令且存活状况完好时，接管动画图集的水平左右拨盘朝向开关
    if (delta > 0 && !is_dead)
    {
        int dir_x = is_move_right - is_move_left;
        if (dir_x < 0) facing_left = true;
        else if (dir_x > 0) facing_left = false;
    }

    // ================= 生存特供级渲染管线 =================
    // 切断一切死后不必要的幽灵底板绘制污染
    if (!is_dead)
    {
        // 处理并对齐渲染阴影映射坐标，辅以前置指定的补偿量修正因拿枪偏移等重力场不均衡产生的错位
        int pos_shadow_x = pos.x + (FRAME_WIDTH / 2 - SHADOW_WIDTH / 2);
        if (facing_left) pos_shadow_x += shadow_offset_value;
        else pos_shadow_x -= shadow_offset_value;

        int pos_shadow_y = pos.y + 100;
        putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow_player);

        // 如果正式切入了战斗，则为其挂载并实时反馈脚部状态追踪生命浮动槽
        if (is_game_started)
        {
            int bar_width = 70;
            int bar_height = 8;
            int bar_x = pos.x + (FRAME_WIDTH - bar_width) / 2;
            int bar_y = pos.y + FRAME_HEIGHT + 20;

            // 绘制底漆
            setlinecolor(RGB(30, 30, 30));
            setfillcolor(RGB(60, 60, 60));
            fillroundrect(bar_x, bar_y, bar_x + bar_width, bar_y + bar_height, 4, 4);

            if (hp > 0)
            {
                // 计算乘区占有率并填充状态血量
                int fill_width = (int)((double)hp / max_hp * bar_width);
                COLORREF hp_color = (hp > max_hp / 2) ? RGB(50, 220, 50) : ((hp > max_hp / 4) ? RGB(255, 200, 50) : RGB(255, 50, 50));
                setfillcolor(hp_color);
                solidroundrect(bar_x + 1, bar_y + 1, bar_x + fill_width - 1, bar_y + bar_height - 1, 4, 4);
            }
        }
    }

    // ================= 玩家动画系统双线分流调度 =================
    if (is_dead)
    {
        // 阵亡执行线分支
        DWORD elapsed = current_time - death_start_time;
        int center_x = pos.x + FRAME_WIDTH / 2;
        int center_y = pos.y + FRAME_HEIGHT / 2;

        // 【特效一】：阵亡前半段 800ms 内迸发的能量崩溃与双层光环溢出动画扩散效果
        if (elapsed < 800) {
            double wave_progress = elapsed / 800.0;
            int wave_radius = (int)(wave_progress * 600);

            setlinestyle(PS_SOLID, max(1, 20 - (int)(wave_progress * 20))); // 内缩光圈变细
            setlinecolor(RGB(255, 50, 50));
            circle(center_x, center_y, wave_radius);

            setlinestyle(PS_SOLID, max(1, 10 - (int)(wave_progress * 10)));
            setlinecolor(RGB(255, 200, 50));
            circle(center_x, center_y, (int)(wave_radius * 0.8));

            setlinestyle(PS_SOLID, 1);
        }

        // 【特效二】：物理重力下沉坍塌与机体临终短时高频颤抖
        int body_drop = min((int)(elapsed / 10), 120);
        if (elapsed < 1000) {
            draw_x += (rand() % 21 - 10);
            draw_y += (rand() % 11 - 5);
        }
        draw_y += body_drop; // 应用纵向物理入土下压量

        // 停止推进动画播放时间戳轴，仅渲染被抽取的临终冻结残影作为停尸面相
        if (facing_left) anim_left->Play(draw_x, draw_y, 0);
        else anim_right->Play(draw_x, draw_y, 0);

        // 【特效三】：伴随着崩溃爆碎四处乱飞的血浆/蜂蜜核心粘液模拟演算粒子系统
        if (elapsed < 2500) {
            for (int i = 0; i < 20; i++) {
                // 圆周分配角度，引入跳级变速随机乘数产生参差不齐的立体崩坏视觉感
                double angle = i * (3.14159 * 2.0 / 20.0);
                double speed = 1.0 + (i % 4) * 0.6;
                double radius = min((double)elapsed * speed, 350.0);

                // 给散落粒子赋予二次抛物线落地坠落拟态参数补偿重力表现
                double drop_y = (elapsed > 100) ? ((elapsed - 100) * (elapsed - 100) / 700.0) : 0;

                int px = center_x + (int)(radius * cos(angle));
                int py = center_y + (int)(radius * sin(angle)) + (int)drop_y;

                // 地面物理碰壁拦截锁死限制
                if (py > pos.y + FRAME_HEIGHT + 50) py = pos.y + FRAME_HEIGHT + 50;

                int size = max(1, 12 - (int)(elapsed / 200) + (i % 3));
                if (elapsed < 2000) {
                    setlinecolor(RGB(255, 120, 0));
                    setfillcolor((i % 2 == 0) ? RGB(255, 200, 50) : RGB(255, 150, 0));
                    fillcircle(px, py, size);
                }
            }
        }

        // 【特效四】：头顶盘旋的眩晕死星渲染组件构建
        for (int i = 0; i < 5; i++) {
            double angle = (elapsed / 80.0) + (i * 1.256);
            int star_x = center_x + (int)(55 * cos(angle));
            int star_y = center_y - 70 + body_drop + (int)(20 * sin(angle));
            COLORREF star_color = (elapsed / 40 % 2 == 0) ? RGB(255, 255, 0) : RGB(255, 50, 50);
            setlinecolor(star_color);
            setfillcolor(star_color);
            fillcircle(star_x, star_y, 6 + (i % 2) * 2);
        }

        // 【特效五】：读取人物对象的濒死遗言独立串做缓慢上浮文本飘字
        if (elapsed < 3000) {
            int text_y = center_y - 100 - (int)(elapsed / 12);
            settextstyle(36, 0, _T("黑体"), 0, 0, FW_BOLD, false, false, false);
            setbkmode(TRANSPARENT);
            int text_w = textwidth(dead_text);

            // 下方底层粗体投影构建
            settextcolor(RGB(20, 20, 20));
            outtextxy(center_x - text_w / 2 + 3, text_y + 3, dead_text);

            // 上方白字高亮反差构建
            settextcolor(RGB(255, 255, 255));
            outtextxy(center_x - text_w / 2, text_y, dead_text);
        }
    }
    else
    {
        // 存活执行线分支
        bool is_invulnerable = (current_time - last_hurt_time < 500); // 半秒内的无敌保护判定
        bool should_flicker_hide = false;

        // 解析挂起并阻断本次调用以伪装出跳频缺帧闪烁的隐身护甲反馈
        if (is_invulnerable)
        {
            if ((current_time / 50) % 2 == 0) should_flicker_hide = true;
        }

        if (!should_flicker_hide)
        {
            // 在受击后的前 200 毫秒内，附加物理随机震动，增强打击感
            if (is_invulnerable && (current_time - last_hurt_time < 200))
            {
                draw_x += (rand() % 11 - 5);
                draw_y += (rand() % 11 - 5);
            }

            // 推进并反馈最新的下发图像渲染指针切帧操作
            if (facing_left) anim_left->Play(draw_x, draw_y, delta);
            else anim_right->Play(draw_x, draw_y, delta);

            // 受击后的前 150 毫秒在腹部位置额外喷出红色的物理警报光斑血洞标识
            if (current_time - last_hurt_time < 150)
            {
                setlinecolor(RGB(255, 50, 50));
                setfillcolor(RGB(200, 30, 30));
                fillcircle(pos.x + FRAME_WIDTH / 2, pos.y + FRAME_HEIGHT / 2 + 10, 35);
            }
        }
    }

    // 5. 伤害飘字组件（在最终层渲染避免被覆盖）
    if (is_popup_active)
    {
        DWORD elapsed = current_time - popup_start_time;
        if (elapsed < 800)
        {
            int text_y = popup_pos.y - (elapsed / 15); // Y轴逆向飘升递增衰退

            // 字符串负号拼装承载扣减逻辑数值传递
            TCHAR text[16];
            _stprintf_s(text, _T("-%d"), popup_damage);

            settextstyle(28, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
            setbkmode(TRANSPARENT);

            int text_w = textwidth(text);
            int text_x = popup_pos.x - text_w / 2; // X轴动态重定位挂载居中计算

            settextcolor(RGB(20, 20, 20));
            outtextxy(text_x + 2, text_y + 2, text);
            settextcolor(RGB(255, 50, 50));
            outtextxy(text_x, text_y, text);
        }
        else is_popup_active = false; // 封存显示器锁
    }
}

const POINT& Player::GetPosition() const { return pos; }
int Player::GetMaxHP() const { return max_hp; }
int Player::GetHP() const { return hp; }

// 从怪端实体向玩家实例本体转发承接的受击降维解算业务流
void Player::TakeDamage(int damage)
{
    if (is_dead) return;

    DWORD current_time = GetTickCount();

    // 二重锁拦截防穿透判定体系：距离上一次受击流转必须超过 500ms，以防即死式伤害黏连
    if (current_time - last_hurt_time > 500)
    {
        // 解调由属性带来的免伤减益系数修正最终掉血标定值
        int final_damage = damage - (int)(damage * damage_reduction);
        if (final_damage < 1 && damage > 0) final_damage = 1;

        // 限制飘字显示的伤害数值
        // 如果最终伤害超过了玩家当前的血量，飘字最大只显示当前剩余血量
        popup_damage = (final_damage > hp) ? hp : final_damage;

        hp -= final_damage;
        last_hurt_time = current_time;

        // 同步配置上方 UI 飘字属性面板与坐标锚点信息
        popup_pos.x = pos.x + FRAME_WIDTH / 2;
        popup_pos.y = pos.y - 20;
        popup_start_time = current_time;
        is_popup_active = true;

        if (hp <= 0)
        {
            hp = 0;
            is_dead = true;
            death_start_time = current_time;

            // 强制擦除清空残余的物理运动意图参数，直接停尸截断防打滑位移
            is_move_up = is_move_down = is_move_left = is_move_right = false;
        }
    }
}

// “再来一局”场景下调用的物理参数及增益重置复位器
void Player::Reset()
{
    hp = max_hp;
    pos = { 500, 500 };
    is_move_up = false;
    is_move_down = false;
    is_move_left = false;
    is_move_right = false;
    is_popup_active = false;
    last_hurt_time = 0;
    is_dead = false;
    death_start_time = 0;
    facing_left = false;
    last_skill_time = GetTickCount() - SKILL_CD;
    skill_end_time = 0;

    if (current_weapon) current_weapon->Reset();

    level = 1;
    exp = 0;
    max_exp = 5;
    SKILL_CD = 8000;
    damage_reduction = 0.0;
    orbital_skill.Reset();
}

void Player::SetPosition(POINT p) { pos = p; }
int Player::GetAttackDamage() const { return attack_damage; }
vector<Bullet>& Player::GetBullets() { return current_weapon->GetBullets(); }

// 周期派发底层火力更新
void Player::UpdateAttacks()
{
    if (current_weapon) {
        // 解算并传递是否挂载爆发光环锁
        bool is_skill_active = (GetTickCount() < skill_end_time);
        current_weapon->Update(pos, facing_left, FRAME_WIDTH, FRAME_HEIGHT, is_skill_active);
    }
}

// 周期派发底层火力渲染挂载
void Player::DrawAttacks()
{
    if (current_weapon) {
        bool is_skill_active = (GetTickCount() < skill_end_time);
        current_weapon->Draw(pos, facing_left, FRAME_WIDTH, FRAME_HEIGHT, is_skill_active);
    }
}

void Player::UseSkill(vector<Enemy*>& enemy_list) {}

// 给全局提供时间冻结暂停补偿修正回调
void Player::AddPauseTime(DWORD pause_duration)
{
    last_skill_time += pause_duration;
    if (skill_end_time > 0) skill_end_time += pause_duration;
    if (last_hurt_time > 0) last_hurt_time += pause_duration;
    if (popup_start_time > 0) popup_start_time += pause_duration;
    if (death_start_time > 0) death_start_time += pause_duration;
}

// 等级跃迁：解构对应被选取 ID 的实际执行强化面条
void Player::ApplyUpgrade(int upgrade_id)
{
    // 死人不能接受强化和治疗
    if (is_dead) 
        return;

    switch (upgrade_id)
    {
    case 0:
        max_hp += 20;
        hp += 20;
        break;
    case 1:
        attack_damage += 1;
        break;
    case 2:
        speed += 1;
        break;
    case 3:
        hp += (max_hp / 2);
        if (hp > max_hp) hp = max_hp;
        break;
    case 4:
        SKILL_CD = (int)(SKILL_CD * 0.8);
        if (SKILL_CD < 1000) SKILL_CD = 1000;
        break;
    case 5:
        // 通过浮点系数追加承伤护甲池
        damage_reduction += 0.20;
        if (damage_reduction > 0.80) damage_reduction = 0.80; // 提供最高 80% 硬底拦截
        break;
    case 6:
        orbital_skill.Upgrade(); // 下发雷击阵列子系统等级拉升
        break;
    }
}

// 将额外特化技能组件（挂载的轨道炮）更新指令抛出
void Player::UpdateExtraSkills(vector<Enemy*>& enemy_list)
{
    orbital_skill.Update(enemy_list, attack_damage, pos);
}