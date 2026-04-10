#include "Atlas.h"

// 加载图集：按路径模板批量加载帧图片
Atlas::Atlas(LPCTSTR path, int num, int width, int height)
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