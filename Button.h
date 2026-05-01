#pragma once
#include <easyx.h>
#include <iostream>
#include "common.h"
using namespace std;

// 交互式 UI 控件抽象基类
class Button
{
public:
    // 构造参数配置：响应区域范围, 空闲状态图元, 悬浮状态图元, 按压状态图元
    Button(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed);
    ~Button();

    // 输入事件流处理模块
    void ProcessEvent(const ExMessage& msg);

    // 控件图像帧渲染管线
    void Draw();

protected:
    // 点击事件触发回调接口定义，供继承方实现
    virtual void OnClick() = 0;

private:
    // 按钮状态机定义枚举
    enum class Status
    {
        Idle = 0,    // 默认待机状态
        Hovered,     // 指针悬停状态
        Pushed       // 点击按压状态
    };

private:
    RECT region;                   // 事件有效响应边界
    IMAGE img_idle;                // 待机纹理映射
    IMAGE img_hovered;             // 悬浮纹理映射
    IMAGE img_pushed;              // 按压纹理映射
    Status status = Status::Idle;  // 状态机记录器

    // 矩形边界碰撞检测方法
    bool CheckCursorHit(int x, int y);
};