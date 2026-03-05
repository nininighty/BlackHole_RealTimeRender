// RealTimeRenderer.cpp
#include "stdafx.h"
#include <chrono>
#include "CBlackHole_RealTimeRenderer.h"


// 构造函数
CBlackHole_RealTimeRenderer::CBlackHole_RealTimeRenderer(RhRdk::Realtime::ISignalUpdate* pS)
    : m_pSignalUpdateInterface(pS), m_pRenderThread(nullptr), m_pRenderWnd(nullptr), m_bRunning(false), m_bIsDirty(false) {
    // 设置默认相机
    m_currentCam.pos = ON_3dPoint::Origin;
    m_currentCam.dir = ON_3dVector::ZAxis;
    m_currentCam.up = ON_3dVector::YAxis;
    m_currentCam.viewAngle = 0.8;
}

// 析构函数，停止渲染
CBlackHole_RealTimeRenderer::~CBlackHole_RealTimeRenderer() {
    StopRenderProcess();
}

// 启动渲染
bool CBlackHole_RealTimeRenderer::StartRenderProcess(const ON_2iSize& frameSize) {
    // 申请一个渲染窗口
    if (nullptr == m_pRenderWnd) m_pRenderWnd = IRhRdkRenderWindow::New();
    if (m_pRenderWnd) {
        m_pRenderWnd->SetSize(frameSize);
        m_pRenderWnd->EnsureDib();  // 申请内存上一片屏幕大小的空间存储DIB（设备无关位图），这个方案已经废弃，实际上没用
    }
    // 建立一个CwinThread负责协调CPU和GPU
    if (nullptr == m_pRenderThread) {
        m_bRunning = true;
        // 分配静态线程函数入口，将本身内存地址传给线程指针，因为CwinThread是C的线程，只接受一个指针，具体可以看博客笔记
        m_pRenderThread = AfxBeginThread(RenderProcess, (void*)this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, 0);
        m_pRenderThread->m_bAutoDelete = FALSE;
        m_pRenderThread->ResumeThread();
    }
    return true;
}

void CBlackHole_RealTimeRenderer::StopRenderProcess() {
    m_bRunning = false;
    m_bIsDirty = true;
    if (m_pRenderThread) {
        // 等待渲染线程结束后才结束
        WaitForSingleObject(m_pRenderThread->m_hThread, INFINITE);
        delete m_pRenderThread;
        m_pRenderThread = nullptr;
    }
    if (m_pRenderWnd) {
        delete m_pRenderWnd;
        m_pRenderWnd = nullptr;
    }
}

// 更新摄像机
void CBlackHole_RealTimeRenderer::UpdateCamera(const ON_Viewport& vp) {
    ON_3dPoint newPos = vp.CameraLocation();
    ON_3dVector newDir = vp.CameraDirection();
    ON_3dVector newUp = vp.CameraUp();

    double half_angle = 0;
    vp.GetCameraAngle(&half_angle);
    double newFov = half_angle * 2.0;

    // 过滤微小抖动
    if (m_currentCam.pos.DistanceTo(newPos) > 1e-5 ||
        (m_currentCam.dir - newDir).Length() > 1e-5)
    {
        // 上锁，安全地更新相机数据
        std::lock_guard<std::mutex> lock(m_camMutex);
        m_currentCam.pos = newPos;
        m_currentCam.dir = newDir;
        m_currentCam.up = newUp;
        m_currentCam.viewAngle = newFov;

        m_bIsDirty = true;
        //当程序执行到这里，变量 lock 离开了它的作用域。
        // 此时，C++ 会自动调用 lock 的析构函数，并在析构函数里自动执行 m_camMutex.unlock()
    }
}

// 渲染入口静态函数
unsigned int CBlackHole_RealTimeRenderer::RenderProcess(void* pData) {
    // 将万能指针解包为Renderer
    CBlackHole_RealTimeRenderer* pR = static_cast<CBlackHole_RealTimeRenderer*>(pData);

    using namespace std::chrono;
    auto lastRenderTime = high_resolution_clock::now();
    const int targetFPS = 30; // 目标帧率 30 帧
    const int frameTimeMs = 1000 / targetFPS;

    while (pR->m_bRunning) {
        if (pR->m_bIsDirty) {
            auto now = high_resolution_clock::now();
            auto duration = duration_cast<milliseconds>(now - lastRenderTime).count();

            // 如果距离上一帧还不到 33ms，就稍微等一下，不要立刻发给 GPU
            if (duration < frameTimeMs) {
                Sleep(1);
                continue;
            }
            lastRenderTime = high_resolution_clock::now();
            pR->m_bIsDirty = false;
            // 实时获取当前 Rhino 渲染窗口的物理像素尺寸
            const ON_2iSize sz = pR->m_pRenderWnd->Size();

            // 1. 拷贝一份安全的相机参数，马上释放锁，防止阻塞主线程
            CameraParameters safeCam;
            {
                // 局部作用域，方便拆锁
                std::lock_guard<std::mutex> lock(pR->m_camMutex);
                safeCam = pR->m_currentCam;
            }

            // 2. GPU 渲染管线
            if (pR->m_gpu.Initialize(sz.cx, sz.cy)) {
                pR->m_gpu.UpdateParams(safeCam, sz.cx, sz.cy);
                pR->m_gpu.Dispatch(sz.cx, sz.cy);

                // 3. 映射结果给 Rhino
                UINT pitch = 0;
                void* pRaw = pR->m_gpu.MapResult(pitch);
                // 确保GPU计算结束
                if (pRaw) {
                    std::lock_guard<std::mutex> lock(pR->m_bufferMutex);
                    auto* pCh = pR->m_pRenderWnd->OpenChannel(IRhRdkRenderWindow::chanRGBA);
                    // 确保Rhino画板正常打开
                    if (pCh) {
                        pCh->SetValueRect(0, 0, sz.cx, sz.cy, pitch, ComponentOrder::RGBA, pRaw);
                        pCh->Close();
                    }
                    pR->m_gpu.UnmapResult();
                }
            }
            pR->m_pSignalUpdateInterface->SignalUpdate();
        }
        else {
            Sleep(5); // 没有任务，睡大觉😴
        }
    }
    return 0;
}