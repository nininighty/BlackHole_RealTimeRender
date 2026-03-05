#pragma once
#include "stdafx.h"

// 专门用于显卡传输的结构体，16 字节对齐
struct GPU_Buffer_Data {
    float camPos[3];    float pad1;      // 相机坐标，16字节
    float camDir[3];    float pad2;      // 面朝方向，16字节
    float camUp[3];     float fov;       // 上方向量 + fov，16字节
    float width;        float height;    float mass;  float spin; 
};

// 后台逻辑使用的相机参数
struct CameraParameters {
    ON_3dPoint  pos;
    ON_3dVector dir;
    ON_3dVector up;
    double      viewAngle;
};