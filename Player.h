#pragma once
#include <iostream>
#include <easyx.h>
#include <vector>
#include "Animation.h"
#include "common.h"
#include "Weapon.h" 
#include "OrbitalStrike.h"
using namespace std;

class Enemy;

// 玩家控制的英雄实体基类：负责封装输入捕获、状态机、背包武器与物理运动组件
class Player
{
public:
    const int FRAME_WIDTH = 80;     // 物理逻辑碰撞盒宽度
    const int FRAME_HEIGHT = 80;    // 物理逻辑碰撞盒高度

    Player();
    virtual ~Player();

    // 核心事件分发器：传入 current_game_mode 以执行严格的双人按键设备硬件隔离
    void ProcessEvent(const ExMessage& msg, int current_game_mode);

    // 物理引擎推进层
    void Move();
    void Draw(int delta);

    // 状态查询器（严格加 const 保护）
    const POINT& GetPosition() const;
    void SetPosition(POINT p);

    int GetMaxHP() const;
    int GetHP() const;
    int GetAttackDamage() const;

    // 状态覆写器
    void Reset();
    void TakeDamage(int damage);
    void AddPauseTime(DWORD pause_duration);

    // RPG 数值养成体系参数
    int level = 1;
    int exp = 0;
    int max_exp = 5;

    // 接收强化面板下发的属性修改器
    void ApplyUpgrade(int upgrade_id);

    // 虚函数留作多态扩展：武器更新、渲染与大招触发
    virtual void UpdateAttacks();
    virtual void DrawAttacks();
    virtual void UseSkill(vector<Enemy*>& enemy_list);

    // 外挂式特种兵装组件（轨道打击系统）
    OrbitalStrikeSkill orbital_skill;
    void UpdateExtraSkills(vector<Enemy*>& enemy_list);

    // 获取当前武器产生的物理弹道集指针
    vector<Bullet>& GetBullets();

    // 技能冷却控制系统
    DWORD last_skill_time = 0;      // 记录上一次发火的时间戳
    int SKILL_CD = 8000;            // 默认技能冷却时间（8秒）

    // 多人模式标识器（1 代表 1P，2 代表 2P）
    int player_id = 1;

protected:
    const int SHADOW_WIDTH = 80;    // 底部投影视觉尺寸
    int speed = 3;                  // 基础刚体移速
    int attack_damage = 1;          // 基础白字攻击力

    LPCTSTR dead_text = _T("啊..."); // 临终遗言默认文案
    int shadow_offset_value = 15;   // 投影位置微调补偿量

    Animation* anim_left;           // 左行动画状态机
    Animation* anim_right;          // 右行动画状态机
    POINT pos = { 500,500 };        // 屏幕绝对坐标

    // 键盘映射的运动意图记录器
    bool is_move_up = false;
    bool is_move_down = false;
    bool is_move_left = false;
    bool is_move_right = false;
    bool facing_left = false;       // 玩家朝向流

    int hp = 100;                   // 当前血量
    int max_hp = 100;               // 血量上限
    DWORD last_hurt_time = 0;       // 受击无敌保护帧判定戳

    // 飘字伤害 UI 挂载参数
    int popup_damage = 0;
    POINT popup_pos = { 0, 0 };
    DWORD popup_start_time = 0;
    bool is_popup_active = false;

    // 生死状态流锁
    bool is_dead = false;
    DWORD death_start_time = 0;

    // 策略模式应用的武器接口指针
    Weapon* current_weapon = nullptr;

    DWORD skill_end_time = 0;       // 用于光头强等角色的 Buff 持续时间标记
    double damage_reduction = 0.0;  // 护甲减伤系数（0.0 ~ 1.0）
};