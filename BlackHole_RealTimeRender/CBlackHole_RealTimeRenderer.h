// CBlackHole_RealTImeRenderer.h
// 实时渲染器核心控制类 (Renderer 中枢)，负责链接Rhino视窗、管理独立渲染线程并调度GPU进行计算
#pragma once
#include "stdafx.h"
#include <mutex>
#include <atomic>
#include "CBlackHole_GPUManager.h"


// 黑洞实时渲染类
class CBlackHole_RealTimeRenderer {
public:
    CBlackHole_RealTimeRenderer(RhRdk::Realtime::ISignalUpdate* pS = nullptr);
    ~CBlackHole_RealTimeRenderer();

    // ==========================================
    // 1. 生命周期与线程控制

    bool StartRenderProcess(const ON_2iSize& frameSize);
    void StopRenderProcess();
    bool Running() const { return m_bRunning; }

    // ==========================================
    // 2. Rhino 交互接口

    void UpdateCamera(const ON_Viewport& vp);
    IRhRdkRenderWindow* RenderWindow() const { return m_pRenderWnd; }    // 获取 Rhino 提供的渲染画布

    // ==========================================
    // 3. 多线程同步锁

    std::mutex m_bufferMutex;   // 帧缓冲锁

private:
    // ==========================================
    // 4. 线程状态与数据标记

    std::atomic<bool> m_bRunning{ false };  // 控制渲染线程的死循环原子锁开关
    std::atomic<bool> m_bIsDirty{ false };      // 移动视角脏标记位

    // ==========================================
    // 5. 核心渲染数据

    CameraParameters  m_currentCam;  // 当前帧的相机参数
    std::mutex        m_camMutex;               // 相机锁
    ON_Xform          m_lastCameraMatrix;   // 记录上一次的相机变换矩阵

    // ==========================================
    // 6. 后台执行体与底层资源

    static unsigned int RenderProcess(void* pData); // 后台渲染线程的入口函数，由于是 C++ 线程回调，必须声明为 static
    CWinThread* m_pRenderThread = nullptr;  // MFC 线程对象指针
    IRhRdkRenderWindow* m_pRenderWnd = nullptr; // Rhino 渲染窗口对象
    RhRdk::Realtime::ISignalUpdate* m_pSignalUpdateInterface = nullptr; // Rhino 信号接口
    CBlackHole_GPUManager m_gpu;    // GPU 管理器
};