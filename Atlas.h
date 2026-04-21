#pragma once
#include <iostream>
#include <easyx.h>
#include <vector>
using namespace std;

// 图集类：管理一组连续的动画帧图片
class Atlas
{
public:
    // 构造函数：path为图片路径模板（含%d），num为帧数量，width/height为帧尺寸（0则用原图尺寸），flip_h为是否水平翻转
    Atlas(LPCTSTR path, int num, int width = 0, int height = 0, bool flip_h = false);
    ~Atlas();

public:
    vector<IMAGE*> frame_list;  // 存储所有帧图片
};