#pragma once
#include <iostream>
#include "Button.h"
#include "common.h"
using namespace std;

// 开始游戏按钮类：继承自 GUI 基础按钮类
class StartGameButton : public Button
{
public:
    // 构造函数：直传坐标区域与三态贴图路径给基类
    StartGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed);
    ~StartGameButton();

protected:
    // 复写点击回调，触发游戏进程的切换
    void OnClick() override; // 补充 override 关键字
};