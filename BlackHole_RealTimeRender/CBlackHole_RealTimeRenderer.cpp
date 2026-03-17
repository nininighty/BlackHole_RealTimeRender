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
    // 设置启动时间
    m_startTime = std::chrono::high_resolution_clock::now();
    Instrumentor::Get().BeginSession("BlackHoleRenderProfile");
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
        m_pRenderWnd->PreAllocateChannel(IRhRdkRenderWindow::chanRGBA); //在主线程提前分配 RGBA 通道内存
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

    // FPS 统计专用变量 
    auto fpsLastTime = std::chrono::high_resolution_clock::now();
    int frameCount = 0;

    while (pR->m_bRunning) {
        PROFILE_SCOPE("RenderProcess1 :: Frame");
        if (pR->m_bIsDirty || pR->m_bAnimate) {
            auto now = high_resolution_clock::now();
            auto duration = duration_cast<milliseconds>(now - lastRenderTime).count();

            // 如果距离上一帧还不到 33ms，就稍微等一下，不要立刻发给 GPU
            if (duration < frameTimeMs) {
                Sleep(1);
                continue;
            }
            lastRenderTime = high_resolution_clock::now();
            if (pR->m_bIsDirty) pR->m_bIsDirty = false;
            std::chrono::duration<float> runTime = now - pR->m_startTime;
            float currentTimeF = runTime.count();

            // 实时获取当前 Rhino 渲染窗口的物理像素尺寸
            const ON_2iSize sz = pR->m_pRenderWnd->Size();

            // 1. 拷贝一份安全的相机参数与物理参数，马上释放锁，防止阻塞主线程
            CameraParameters safeCam;
            float safeMass = 1.0f;
            float safeSpin = 0.5f;
            OrbitSphereParameters safeSphere; // 球体参数缓存
            {
                std::lock_guard<std::mutex> lock(pR->m_camMutex);
                safeCam = pR->m_currentCam;
                safeMass = pR->m_targetMass;  
                safeSpin = pR->m_targetSpin; 
            }
            // 加锁并拷贝天体数据
            {
                std::lock_guard<std::mutex> lockSphere(pR->m_sphereMutex);
                safeSphere = pR->m_targetSphere;
            }
            ON_3dPoint currentSpherePos = safeSphere.initialCenter;
            if (safeSphere.active) {
                double R = safeSphere.initialCenter.DistanceTo(ON_3dPoint::Origin);
                //  使用 Y 和 X 计算初始赤道面夹角
                double init_angle = std::atan2(safeSphere.initialCenter.y, safeSphere.initialCenter.x);
                double current_angle = init_angle + currentTimeF * safeSphere.orbitSpeed;

                double x_prime = R * std::cos(current_angle);
                double y_prime = R * std::sin(current_angle);

                double s_inc = std::sin(safeSphere.inclination);
                double c_inc = std::cos(safeSphere.inclination);

                // 应用轨道倾角，产生 Z 轴方向的高度变化
                currentSpherePos = ON_3dPoint(x_prime, y_prime * c_inc, y_prime * s_inc);
            }

            // 3. GPU 渲染管线
            double pureGPUTimeMs = 0.0;
            if (pR->m_gpu.Initialize(sz.cx, sz.cy)) {
                
                {
                    // 监控上传常量的耗时
                    PROFILE_SCOPE("RenderProcess2 :: GPU_UpdateParams");
                    pR->m_gpu.UpdateParams(safeCam, safeMass, safeSpin, currentTimeF, sz.cx, sz.cy, safeSphere, currentSpherePos);
                }
                {
                    // 监控 Compute Shader 调度的耗时
                    PROFILE_SCOPE("RenderProcess2 :: GPU_Dispatch");
                    pureGPUTimeMs = pR->m_gpu.Dispatch(sz.cx, sz.cy);
                }

                // 3. 映射结果给 Rhino
                UINT pitch = 0;
                {
                    // 监控从显存把画面回传给内存的耗时
                    PROFILE_SCOPE("RenderProcess3 :: GPU_MapResult");
                    void* pRaw = pR->m_gpu.MapResult(pitch);

                    // 确保GPU计算结束
                    if (pRaw) {
                            std::lock_guard<std::mutex> lock(pR->m_bufferMutex);
                        {
                            PROFILE_SCOPE("RenderProcess3 :: GPU_MapResult :: OpenChannel + SetValueRect + CloseChannel");
                            auto* pCh = pR->m_pRenderWnd->OpenChannel(IRhRdkRenderWindow::chanRGBA);
                            // 确保Rhino画板正常打开
                            if (pCh) {
                                PROFILE_SCOPE("RenderProcess3 :: GPU_MapResult :: SetValueRect");
                                pCh->SetValueRect(0, 0, sz.cx, sz.cy, pitch, ComponentOrder::RGBA, pRaw);
                                {
                                    PROFILE_SCOPE("RenderProcess3 :: GPU_MapResult :: CloseChannel");
                                    pCh->Close();
                                }
                            }
                        }
                        PROFILE_SCOPE("RenderProcess3 :: GPU_MapResult :: UnmapResult");
                        pR->m_gpu.UnmapResult();
                    }
                }
            }

            // 统计帧率
            frameCount++;
            auto fpsNow = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> fpsElapsed = fpsNow - fpsLastTime;

            // 每隔 2.0 秒刷新一次帧率显示，避免疯狂刷新导致界面卡顿
            if (fpsElapsed.count() >= 2.0) {
                double fps = frameCount / fpsElapsed.count();

                // 格式化输出字符串
                wchar_t msg[256];
                swprintf_s(msg, L"【黑洞引擎】 帧率: %.1f FPS | 纯GPU渲染耗时: %.2f ms\n", fps, pureGPUTimeMs);

                // 将帧率实时打印到 Rhino 的主命令行提示区
                RhinoApp().Print(msg);

                // 重置计数器
                frameCount = 0;
                fpsLastTime = fpsNow;
            }

            pR->m_pSignalUpdateInterface->SignalUpdate();
        }
        else {
            Sleep(5); // 没有任务，睡大觉😴
        }
    }
    return 0;
}

// Rhino命令更改黑洞参数
void CBlackHole_RealTimeRenderer::UpdatePhysicsParams(double mass, double spin) {
    // 复用 m_camMutex 保证线程安全，防止一边渲染一边修改参数导致程序崩溃
    std::lock_guard<std::mutex> lock(m_camMutex);

    // 负数代表不修改该参数
    if (mass >= 0.0) m_targetMass = static_cast<float>(mass);
    if (spin >= 0.0) m_targetSpin = static_cast<float>(spin);

    m_bIsDirty = true; // 标记脏数据，唤醒后台线程重新计算画面
}

// 更新天体
void CBlackHole_RealTimeRenderer::UpdateOrbitSphere(const OrbitSphereParameters& params) {
    std::lock_guard<std::mutex> lock(m_sphereMutex);
    m_targetSphere = params;
    m_bIsDirty = true; // 唤醒后台线程重新计算画面
}