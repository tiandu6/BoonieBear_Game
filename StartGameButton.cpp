#include "StartGameButton.h"

// 将预配置好的布局资源和纹理数据交由其基类实例化引擎处理
StartGameButton::StartGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
    :Button(rect, path_img_idle, path_img_hovered, path_img_pushed)
{

}

StartGameButton::~StartGameButton() = default;

// 配置状态切换器转入核心运行进程图景
void StartGameButton::OnClick()
{
    is_game_started = true;
}