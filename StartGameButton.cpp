#include "StartGameButton.h"

// 构造函数，调用父类构造初始化
StartGameButton::StartGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
    :Button(rect, path_img_idle, path_img_hovered, path_img_pushed)
{

}

StartGameButton::~StartGameButton() = default;

// 点击开始游戏按钮的逻辑
void StartGameButton::OnClick()
{
    // 标记游戏开始
    is_game_started = true;
    // 循环播放背景音乐
    mciSendString(_T("play bgm repeat from 0"), NULL, 0, NULL);
}