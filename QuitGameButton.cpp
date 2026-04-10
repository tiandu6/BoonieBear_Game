#include "QuitGameButton.h"

// 构造函数，调用父类构造初始化
QuitGameButton::QuitGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
    :Button(rect, path_img_idle, path_img_hovered, path_img_pushed)
{

}

QuitGameButton::~QuitGameButton() = default;

// 点击退出游戏按钮的逻辑
void QuitGameButton::OnClick()
{
    // 标记游戏结束、退出主循环
    is_game_started = false;
    running = false;
}