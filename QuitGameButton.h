#pragma once
#include <iostream>
#include "Button.h"
#include "common.h"
using namespace std;

// 退出游戏按钮类，继承自基础按钮类
class QuitGameButton :public Button
{
public:
    // 构造函数：传入按钮区域、各状态图片路径
    QuitGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed);
    ~QuitGameButton();

protected:
    // 按钮点击后的处理逻辑
    void OnClick();
};
