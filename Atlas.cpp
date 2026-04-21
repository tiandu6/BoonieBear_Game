#include "Atlas.h"

// 加载图集：按路径模板批量加载帧图片（支持代码级水平翻转）
Atlas::Atlas(LPCTSTR path, int num, int width, int height, bool flip_h)
{
    TCHAR path_file[256];
    for (size_t i = 0; i < num; i++)
    {
        // 格式化路径（替换%d为帧序号）
        _stprintf_s(path_file, path, i);

        IMAGE* frame = new IMAGE();
        // 指定尺寸则按尺寸加载，否则用原图尺寸，启用透明
        if (width > 0 && height > 0) {
            loadimage(frame, path_file, width, height, true);
        }
        else {
            loadimage(frame, path_file);
        }

        // 【新增硬核操作】：如果要求翻转，则直接操作底层像素显存！
        if (flip_h)
        {
            int w = frame->getwidth();
            int h = frame->getheight();
            // 获取 EasyX 图像底层的显存指针（每个元素是一个 DWORD，即 ARGB 四通道像素点）
            DWORD* buffer = GetImageBuffer(frame);

            // 遍历图像的每一行
            for (int y = 0; y < h; y++)
            {
                // 只遍历半行，将左边和右边对称位置的像素互换
                for (int x = 0; x < w / 2; x++)
                {
                    int left_idx = y * w + x;                 // 左侧像素的数组下标
                    int right_idx = y * w + (w - 1 - x);      // 对应的右侧对称像素的数组下标

                    // 交换像素值（完美保留颜色和 Alpha 透明度）
                    DWORD temp = buffer[left_idx];
                    buffer[left_idx] = buffer[right_idx];
                    buffer[right_idx] = temp;
                }
            }
        }

        frame_list.push_back(frame);
    }
}

// 释放所有帧图片资源
Atlas::~Atlas()
{
    for (size_t i = 0; i < frame_list.size(); i++)
    {
        delete frame_list[i];
    }
}