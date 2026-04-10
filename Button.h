#pragma once
#include <easyx.h>
#include <iostream>
#include "common.h"
using namespace std;

// 按钮基类（抽象类）
class Button
{
public:
    // 构造函数：传入按钮区域、各状态图片路径
    Button(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed);
    ~Button();
    // 处理按钮输入事件（鼠标）
    void ProcessEvent(const ExMessage& msg);
    // 绘制按钮（根据当前状态）
    void Draw();

protected:
    // 点击回调（子类实现）
    virtual void OnClick() = 0;

private:
    // 按钮状态枚举
    enum class Status
    {
        Idle = 0,    // 闲置
        Hovered,     // 鼠标悬浮
        Pushed       // 鼠标按下
    };

private:
    RECT region;          // 按钮区域（坐标+尺寸）
    IMAGE img_idle;       // 闲置状态图片
    IMAGE img_hovered;    // 悬浮状态图片
    IMAGE img_pushed;     // 按下状态图片
    Status status = Status::Idle;  // 当前状态

    // 检测鼠标是否在按钮区域内
    bool CheckCursorHit(int x, int y);
};