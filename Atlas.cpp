#include "Atlas.h"

// 构造函数：按模板路径批量加载序列帧数据，并支持水平翻转预处理
Atlas::Atlas(LPCTSTR path, int num, int width, int height, bool flip_h)
{
    TCHAR path_file[256];
    for (size_t i = 0; i < num; i++)
    {
        // 格式化文件路径以匹配当前帧序号
        _stprintf_s(path_file, path, i);

        IMAGE* frame = new IMAGE();
        // 根据指定尺寸执行图像加载逻辑，并默认开启透明度计算
        if (width > 0 && height > 0) {
            loadimage(frame, path_file, width, height, true);
        }
        else {
            loadimage(frame, path_file);
        }

        // 图像预处理逻辑：通过显存层级进行像素级水平翻转
        if (flip_h)
        {
            int w = frame->getwidth();
            int h = frame->getheight();
            // 获取图像显存数据缓冲指针 (包含 ARGB 四通道数据)
            DWORD* buffer = GetImageBuffer(frame);

            // 遍历像素矩阵
            for (int y = 0; y < h; y++)
            {
                // 对称交换左右两侧像素数据
                for (int x = 0; x < w / 2; x++)
                {
                    int left_idx = y * w + x;
                    int right_idx = y * w + (w - 1 - x);

                    // 执行像素级内存交换操作 (保留颜色及 Alpha 属性)
                    DWORD temp = buffer[left_idx];
                    buffer[left_idx] = buffer[right_idx];
                    buffer[right_idx] = temp;
                }
            }
        }

        frame_list.push_back(frame);
    }
}

// 析构函数：清理图集缓存占用的内存资源
Atlas::~Atlas()
{
    for (size_t i = 0; i < frame_list.size(); i++)
    {
        delete frame_list[i];
    }
}