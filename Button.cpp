#include "Button.h"

// 构造解析与材质实例化载入
Button::Button(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
{
    region = rect; // 初始化包围盒坐标集合

    // 逆向计算宽高以适配图像加载引擎需求
    int width = region.right - region.left;
    int height = region.bottom - region.top;

    // 拉取磁盘文件并构建到内存对象中，配置布尔真开启抗锯齿通道
    loadimage(&img_idle, path_img_idle, width, height, true);
    loadimage(&img_hovered, path_img_hovered, width, height, true);
    loadimage(&img_pushed, path_img_pushed, width, height, true);
}

Button::~Button() = default;

// 焦点状态机路由分发控制核心
void Button::ProcessEvent(const ExMessage& msg)
{
    switch (msg.message)
    {
    case WM_MOUSEMOVE: // 捕获光标无按键位移事件
        // 当光标进入边界时切为悬浮，离开边界则回退为空闲
        if (status == Status::Idle && CheckCursorHit(msg.x, msg.y))
            status = Status::Hovered;
        else if (status == Status::Hovered && !CheckCursorHit(msg.x, msg.y))
            status = Status::Idle;
        break;

    case WM_LBUTTONDOWN: // 捕获左键下压动作
        // 验证锚点合法性后进入压燃状态
        if (CheckCursorHit(msg.x, msg.y))
            status = Status::Pushed;
        break;

    case WM_LBUTTONUP: // 捕获左键抬起动作
        // 唯有经历过正确按压流程的控件，在释放瞬间才能触发有效击发逻辑
        if (status == Status::Pushed)
            OnClick();
        break;

    default:
        break;
    }
}

// GUI 映射渲染函数
void Button::Draw()
{
    // 利用状态机枚举路由对应的内存图层对象
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

// 严谨的区域数学运算，测算指针是否落入合法响应矩阵内部
bool Button::CheckCursorHit(int x, int y) const // 补充了 const
{
    return x >= region.left && x <= region.right && y >= region.top && y <= region.bottom;
}