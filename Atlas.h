#pragma once
#include <iostream>
#include <easyx.h>
#include <vector>
using namespace std;

// 序列帧图集数据层容器：负责将磁盘上的图片序列载入显存并进行批处理
class Atlas
{
public:
    // 构造参数说明：
    // path: 文件路径格式化模板（如 "img_%d.png"）
    // num: 序列帧总数
    // width/height: 指定缩放尺寸（传 0 则保留图像原始分辨率）
    // flip_h: 布尔开关，标识是否在加载时进行水平镜像翻转预处理
    Atlas(LPCTSTR path, int num, int width = 0, int height = 0, bool flip_h = false);
    ~Atlas();

public:
    vector<IMAGE*> frame_list;  // 显存图元池：存储所有处理完毕的帧实体指针
};