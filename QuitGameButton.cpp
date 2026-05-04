#include "QuitGameButton.h"

// 构造函数，使用初始化列表将参数透传调用父类构造初始化
QuitGameButton::QuitGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
    :Button(rect, path_img_idle, path_img_hovered, path_img_pushed)
{
}

QuitGameButton::~QuitGameButton() = default;

// 触发器回调：点击退出游戏按钮的逻辑
void QuitGameButton::OnClick()
{
    // 抹除游戏运行状态，并拉下全局主循环进程电闸，直接退出程序
    is_game_started = false;
    running = false;
}