#pragma once
#include <iostream>
#include <easyx.h>
#include "Animation.h"
#include "common.h"
using namespace std;

// 玩家实体类
class Player
{
public:
    // 玩家帧尺寸
    const int FRAME_WIDTH = 80;
    const int FRAME_HEIGHT = 80;

    Player();
    virtual ~Player(); // 【极其重要】：改为虚析构函数，满足大作业多态要求

    // 处理玩家输入事件（键鼠）
    void ProcessEvent(const ExMessage& msg);
    // 玩家移动逻辑
    void Move();
    // 绘制玩家（含动画、阴影）
    void Draw(int delta);
    // 获取玩家当前坐标
    const POINT& GetPosition() const;
    void SetPosition(POINT p); // 用于无缝切换时传递坐标
    int GetMaxHP() const;  // 获取最大血量
    int GetHP() const;// 获取当前血量
    int GetAttackDamage() const; // 【新增】：获取当前角色的攻击力
    void Reset(); // 重置玩家状态（用于再来一局）
    void TakeDamage(int damage);

protected:
    // 玩家阴影宽度、移动速度
    const int SHADOW_WIDTH = 80;
    int speed = 3;
    int attack_damage = 1; // 【新增】：基础攻击力，默认为 1

    LPCTSTR dead_text = _T("啊..."); // 角色的专属死亡遗言

    // 角色的专属阴影偏移量，默认值为 15
    int shadow_offset_value = 15;

    IMAGE img_shadow;          // 玩家阴影图片
    Animation* anim_left;      // 向左的动画
    Animation* anim_right;     // 向右的动画
    POINT pos = { 500,500 };   // 玩家初始坐标

    // 玩家移动状态（上下左右）
    bool is_move_up = false;
    bool is_move_down = false;
    bool is_move_left = false;
    bool is_move_right = false;

    // 玩家当前的朝向
    bool facing_left = false;

    int hp = 100;              // 当前生命值
    int max_hp = 100;          // 最大生命值
    DWORD last_hurt_time = 0;  // 记录上次受击时间（用于实现无敌帧）

    // --- 浮动伤害数字相关变量 ---
    int popup_damage = 0;           // 弹出的伤害数值
    POINT popup_pos = { 0, 0 };     // 伤害数字的初始位置
    DWORD popup_start_time = 0;     // 弹出开始的时间
    bool is_popup_active = false;   // 是否正在显示伤害数字

    // --- 死亡动画相关变量 ---
    bool is_dead = false;           // 是否已经死亡
    DWORD death_start_time = 0;     // 记录死亡瞬间的时间


};