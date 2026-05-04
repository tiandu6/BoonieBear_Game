#include "Atlas.h"

// IO 加载与显存预处理核心引擎
Atlas::Atlas(LPCTSTR path, int num, int width, int height, bool flip_h)
{
    // 预分配 Vector 内存容量，避免 push_back 时引发昂贵的动态扩容拷贝开销
    frame_list.reserve(num);

    TCHAR path_file[256];
    for (size_t i = 0; i < num; i++)
    {
        // 动态注入帧序号到格式化字符串中
        _stprintf_s(path_file, path, i);

        IMAGE* frame = new IMAGE();

        // 依据传入尺寸参数进行条件编译级的动态拉伸加载，并开启透明图层支持
        if (width > 0 && height > 0) {
            loadimage(frame, path_file, width, height, true);
        }
        else {
            loadimage(frame, path_file);
        }

        // 图像预处理管线：利用显存缓冲区指针进行纯物理层的像素级镜像操作
        if (flip_h)
        {
            int w = frame->getwidth();
            int h = frame->getheight();
            // 提取 EasyX 底层的 ARGB 四通道像素显存数据块
            DWORD* buffer = GetImageBuffer(frame);

            // 以中心轴为界，遍历每一行像素进行双指针首尾调换
            for (int y = 0; y < h; y++)
            {
                for (int x = 0; x < w / 2; x++)
                {
                    int left_idx = y * w + x;
                    int right_idx = y * w + (w - 1 - x);

                    // 交换内存数据（完整保留颜色色阶与 Alpha 透明通道）
                    DWORD temp = buffer[left_idx];
                    buffer[left_idx] = buffer[right_idx];
                    buffer[right_idx] = temp;
                }
            }
        }

        frame_list.push_back(frame);
    }
}

// 析构与 GC：销毁容器前必须释放其中所有的堆内存实体以防止内存泄漏
Atlas::~Atlas()
{
    for (size_t i = 0; i < frame_list.size(); i++)
    {
        delete frame_list[i];
    }
}