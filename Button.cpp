#include "Button.h"

// 按钮初始化：加载各状态图片并设置区域
Button::Button(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
{
    region = rect;

    // 计算按钮宽高
    int width = region.right - region.left;
    int height = region.bottom - region.top;

    // 加载按钮图片，指定尺寸并启用透明
    loadimage(&img_idle, path_img_idle, width, height, true);
    loadimage(&img_hovered, path_img_hovered, width, height, true);
    loadimage(&img_pushed, path_img_pushed, width, height, true);
}

Button::~Button() = default;

// 处理按钮的鼠标事件
void Button::ProcessEvent(const ExMessage& msg)
{
    switch (msg.message)
    {
    case WM_MOUSEMOVE:
        // 鼠标移入/移出：切换悬浮/闲置状态
        if (status == Status::Idle && CheckCursorHit(msg.x, msg.y))
            status = Status::Hovered;
        else if (status == Status::Hovered && !CheckCursorHit(msg.x, msg.y))
            status = Status::Idle;
        break;
    case WM_LBUTTONDOWN:
        // 鼠标按下：切换为按下状态
        if (CheckCursorHit(msg.x, msg.y))
            status = Status::Pushed;
        break;
    case WM_LBUTTONUP:
        // 鼠标抬起：触发点击回调（仅当按下状态时）
        if (status == Status::Pushed)
            OnClick();
        break;
    default:
        break;
    }
}

// 绘制按钮：根据当前状态选择对应图片
void Button::Draw()
{
    switch (status)
    {
    case Status::Idle:
        putimage_alpha(region.left, region.top, &img_idle);
        break;
    case Status::Hovered:
        putimage_alpha(region.left, region.top, &img_hovered);
        break;
    case Status::Pushed:
        putimage_alpha(region.left, region.top, &img_pushed);
        break;
    }
}

// 检测鼠标坐标是否在按钮区域内
bool Button::CheckCursorHit(int x, int y)
{
    return x >= region.left && x <= region.right && y >= region.top && y <= region.bottom;
}