// CBlackHole_RealTimeRenderer.h
// 处理黑洞渲染的声明
#pragma once
#include "stdafx.h"
#include <atomic> // 用于处理多线程变量同步

// 暂时定义一个结构体，用于在线程间传递相机参数
struct CameraParameters {
    ON_3dPoint  pos;     // 眼睛位置
    ON_3dVector dir;     // 视线方向
    ON_3dVector up;      // 头部朝向
    double      viewAngle; // FOV
};


class CBlackHole_RealTimeRenderer {
public:
    CBlackHole_RealTimeRenderer(RhRdk::Realtime::ISignalUpdate* pSignalUpdateInterface = nullptr);
    ~CBlackHole_RealTimeRenderer();

    // --- 核心控制接口 ---
    bool StartRenderProcess(const ON_2iSize& frameSize);
    void StopRenderProcess();
    bool Running() const { return m_bRunning; }
    IRhRdkRenderWindow* RenderWindow() const { return m_pRenderWnd; }

    // --- 相机同步接口 ---
    // 经理会在视角变化时调用此函数
    void UpdateCamera(const ON_Viewport& vp); // 直接接收视口对象

private:
    // --- 渲染状态机变量 (使用 atomic 保证线程间读写安全) ---
    std::atomic<bool> m_bRunning{ false };

    // 分辨率层级：移动时设为 0 (最模糊)，静止后逐步 +1, +2...
    // 我们用它来计算循环步长：step = pow(2, MAX_LEVEL - current_level)
    std::atomic<int>  m_nResolutionLevel{ 0 };

    // 脏标记：一旦相机移动，设为 true，提醒渲染线程重置
    std::atomic<bool> m_bIsDirty{ false };

    // 当前相机的物理参数，供渲染循环计算光线
    CameraParameters m_currentCam;

    // 记录上一帧矩阵，用于位移判定
    ON_Xform m_lastCameraMatrix;

    // --- 多线程相关 ---
    static unsigned int RenderProcess(void* pData);
    CWinThread* m_pRenderThread = nullptr;
    IRhRdkRenderWindow* m_pRenderWnd = nullptr;
    RhRdk::Realtime::ISignalUpdate* m_pSignalUpdateInterface = nullptr;

    // --- 内部渲染逻辑 ---
    // 这是一个示意函数，未来会根据 x, y 和相机参数计算黑洞物理颜色
    // float TraceRay(int x, int y, const CameraParameters& cam); 
};