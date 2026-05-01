#include "Button.h"

// 构造函数：解析按钮区域参数，加载多态交互贴图数据
Button::Button(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
{
    region = rect;

    // 解析按钮控件尺寸属性
    int width = region.right - region.left;
    int height = region.bottom - region.top;

    // 根据指定尺寸映射加载各交互状态下的图像数据
    loadimage(&img_idle, path_img_idle, width, height, true);
    loadimage(&img_hovered, path_img_hovered, width, height, true);
    loadimage(&img_pushed, path_img_pushed, width, height, true);
}

Button::~Button() = default;

// 事件分发器：处理鼠标交互状态机切换
void Button::ProcessEvent(const ExMessage& msg)
{
    switch (msg.message)
    {
    case WM_MOUSEMOVE:
        // 处理悬浮态变迁
        if (status == Status::Idle && CheckCursorHit(msg.x, msg.y))
            status = Status::Hovered;
        else if (status == Status::Hovered && !CheckCursorHit(msg.x, msg.y))
            status = Status::Idle;
        break;
    case WM_LBUTTONDOWN:
        // 处理点击按下态变迁
        if (CheckCursorHit(msg.x, msg.y))
            status = Status::Pushed;
        break;
    case WM_LBUTTONUP:
        // 处理释放与触发回调
        if (status == Status::Pushed)
            OnClick();
        break;
    default:
        break;
    }
}

// 渲染器：依据当前状态机输出对应映射贴图
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

// 碰撞检测逻辑：校验输入坐标点是否置于按钮响应域内
bool Button::CheckCursorHit(int x, int y)
{
    return x >= region.left && x <= region.right && y >= region.top && y <= region.bottom;
}