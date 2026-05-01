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

// 玩家主控实体类抽象模型构建基础声明
class Player
{
public:
    const int FRAME_WIDTH = 80;
    const int FRAME_HEIGHT = 80;

    Player();
    virtual ~Player();

    void ProcessEvent(const ExMessage& msg);
    void Move();
    void Draw(int delta);

    const POINT& GetPosition() const;
    void SetPosition(POINT p);

    int GetMaxHP() const;
    int GetHP() const;
    int GetAttackDamage() const;

    void Reset();
    void TakeDamage(int damage);
    void AddPauseTime(DWORD pause_duration);

    int level = 1;
    int exp = 0;
    int max_exp = 5;

    void ApplyUpgrade(int upgrade_id);

    virtual void UpdateAttacks();
    virtual void DrawAttacks();
    virtual void UseSkill(vector<Enemy*>& enemy_list);

    OrbitalStrikeSkill orbital_skill;
    void UpdateExtraSkills(vector<Enemy*>& enemy_list);

    vector<Bullet>& GetBullets();

    DWORD last_skill_time = 0;
    int SKILL_CD = 8000;

protected:
    const int SHADOW_WIDTH = 80;
    int speed = 3;
    int attack_damage = 1;

    LPCTSTR dead_text = _T("啊...");

    int shadow_offset_value = 15;

    IMAGE img_shadow;
    Animation* anim_left;
    Animation* anim_right;
    POINT pos = { 500,500 };

    bool is_move_up = false;
    bool is_move_down = false;
    bool is_move_left = false;
    bool is_move_right = false;

    bool facing_left = false;

    int hp = 100;
    int max_hp = 100;
    DWORD last_hurt_time = 0;

    int popup_damage = 0;
    POINT popup_pos = { 0, 0 };
    DWORD popup_start_time = 0;
    bool is_popup_active = false;

    bool is_dead = false;
    DWORD death_start_time = 0;

    Weapon* current_weapon = nullptr;
    DWORD skill_end_time = 0;

    double damage_reduction = 0.0;
};