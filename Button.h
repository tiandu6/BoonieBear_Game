#pragma once
#include <easyx.h>
#include <iostream>
#include "common.h"
using namespace std;

// UI 控件基础框架：提供事件驱动机制与三态图像展示功能的交互按钮基类
class Button
{
public:
    // 构造注入：响应盒模型坐标、常态贴图、悬浮高亮贴图、按压激活贴图
    Button(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed);
    virtual ~Button();

    // 接收系统派发的鼠标外设结构体，执行状态机运算
    void ProcessEvent(const ExMessage& msg);

    // 调用贴图模块向显示缓冲投射当前枚举阶段匹配的材质
    void Draw();

protected:
    // 事件回调勾子：纯虚函数，强约束要求派生类实现具体的业务流触发逻辑
    virtual void OnClick() = 0;

private:
    // 枚举化控制按钮内部的三段式流转体系
    enum class Status
    {
        Idle = 0,    // 默认游离态
        Hovered,     // 光标接触悬停态
        Pushed       // 左键下压激活态
    };

private:
    RECT region;                   // GUI 响应作用域
    IMAGE img_idle;                // 闲置图层实体
    IMAGE img_hovered;             // 触碰图层实体
    IMAGE img_pushed;              // 按击图层实体
    Status status = Status::Idle;  // 初始化起始态

    // AABB 盒碰撞检测：【优化】标记为 const 以保证此探测方法绝对独立，不会篡改类结构
    bool CheckCursorHit(int x, int y) const;
};