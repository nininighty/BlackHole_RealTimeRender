#include "stdafx.h"
#include "CBlackHole_RealTimeRenderer.h"

// -----------------------------------------------------------------------------------------
// 1. 构造与析构：初始化状态
// -----------------------------------------------------------------------------------------
CBlackHole_RealTimeRenderer::CBlackHole_RealTimeRenderer(RhRdk::Realtime::ISignalUpdate* pSignalUpdateInterface)
    : m_pSignalUpdateInterface(pSignalUpdateInterface),
    m_pRenderThread(nullptr),
    m_pRenderWnd(nullptr),
    m_bRunning(false),
    m_nResolutionLevel(0),
    m_bIsDirty(false)
{
    m_currentCam.pos = ON_3dPoint::Origin;
    m_currentCam.dir = ON_3dVector::ZAxis;
    m_currentCam.up = ON_3dVector::YAxis;
    m_currentCam.viewAngle = 0.8;

    m_lastCameraMatrix = ON_Xform::ZeroTransformation;
}

CBlackHole_RealTimeRenderer::~CBlackHole_RealTimeRenderer() {
    StopRenderProcess();
}

// -----------------------------------------------------------------------------------------
// 2. 多线程管理：启动与停止渲染器
// -----------------------------------------------------------------------------------------
bool CBlackHole_RealTimeRenderer::StartRenderProcess(const ON_2iSize& frameSize) {
    if (nullptr == m_pRenderWnd) {
        m_pRenderWnd = IRhRdkRenderWindow::New();
    }

    if (m_pRenderWnd) {
        m_pRenderWnd->SetSize(frameSize);
        m_pRenderWnd->EnsureDib();
    }

    if (nullptr == m_pRenderThread) {
        m_bRunning = true;
        // 开启后台线程，并将 this 指针传入供静态函数使用
        m_pRenderThread = AfxBeginThread(RenderProcess, (void*)this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, 0);
        m_pRenderThread->m_bAutoDelete = FALSE;
        m_pRenderThread->ResumeThread();
    }
    return true;
}

void CBlackHole_RealTimeRenderer::StopRenderProcess() {
    if (m_pRenderThread) {
        m_bRunning = false;
        m_bIsDirty = true; // 唤醒可能的等待
        WaitForSingleObject(m_pRenderThread->m_hThread, INFINITE);
        delete m_pRenderThread;
        m_pRenderThread = nullptr;
    }
    if (m_pRenderWnd) {
        delete m_pRenderWnd;
        m_pRenderWnd = nullptr;
    }
}

// -----------------------------------------------------------------------------------------
// 3. 相机同步：实现“移动归零”的核心触发
// -----------------------------------------------------------------------------------------
void CBlackHole_RealTimeRenderer::UpdateCamera(const ON_Viewport& vp) {
    // 1. 获取当前相机矩阵
    ON_Xform currentMatrix;
    // 获取从世界坐标到相机坐标的变换矩阵
    vp.GetXform(ON::world_cs, ON::camera_cs, currentMatrix);

    // 2. 对比判定
    if (currentMatrix != m_lastCameraMatrix) {
        m_nResolutionLevel = 0;
        m_bIsDirty = true;

        // 【修正】根据编译器建议使用新的标识符
        m_lastCameraMatrix = currentMatrix;

        // 3. 提取物理参数
        m_currentCam.pos = vp.CameraLocation();
        m_currentCam.dir = vp.CameraDirection();
        m_currentCam.up = vp.CameraUp();

        // 【修正】Rhino 8 获取视角的方法
        double half_angle = 0;
        if (vp.GetCameraAngle(&half_angle)) {
            m_currentCam.viewAngle = half_angle * 2.0; // 得到完整的视角
        }
    }
}

// -----------------------------------------------------------------------------------------
// 4. 核心计算：跳格采样渲染循环 (渐进式渲染)
// -----------------------------------------------------------------------------------------
unsigned int CBlackHole_RealTimeRenderer::RenderProcess(void* pData) {
    CBlackHole_RealTimeRenderer* pRenderer = static_cast<CBlackHole_RealTimeRenderer*>(pData);
    if (!pRenderer || !pRenderer->m_pRenderWnd) return 0;

    while (pRenderer->m_bRunning) {
        // 计算当前步长：Level 0 -> 8, Level 1 -> 4, Level 2 -> 2, Level 3 -> 1
        int step = 8 >> pRenderer->m_nResolutionLevel;
        if (step < 1) step = 1;

        const ON_2iSize sz = pRenderer->m_pRenderWnd->Size();
        IRhRdkRenderWindow::IChannel* pChannel = pRenderer->m_pRenderWnd->OpenChannel(IRhRdkRenderWindow::chanRGBA);

        if (pChannel) {
            pRenderer->m_bIsDirty = false; // 重置脏标记，开始渲染

            for (int y = 0; y < sz.cy; y += step) {
                // 如果期间相机又动了，立刻跳出当前的双层循环，重新从 Level 0 开始画
                if (pRenderer->m_bIsDirty || !pRenderer->m_bRunning) break;

                for (int x = 0; x < sz.cx; x += step) {
                    // --- 物理计算区 ---
                    // 1. 将像素坐标归一化到 [-1, 1] 空间 (NDC)
                    float normalizedX = (2.0f * x) / sz.cx - 1.0f;
                    float normalizedY = 1.0f - (2.0f * y) / sz.cy; // Y轴反转，因为屏幕坐标向下增长

                    // 2. 考虑长宽比 (Aspect Ratio)
                    float aspect = (float)sz.cx / (float)sz.cy;

                    // 3. 考虑视野张角 (FOV)
                    // viewAngle 是弧度，tan(FOV/2) 决定了视平面的外扩程度
                    double fovScale = tan(pRenderer->m_currentCam.viewAngle * 0.5);

                    // 4. 在相机局部坐标系下构建光线方向 (假设相机看向 -Z 轴)
                    // 这里我们构建一个指向“虚拟屏幕”的向量
                    ON_3dVector rayLocal((double)(normalizedX * aspect * fovScale), (double)(normalizedY * fovScale), -1.0);
                    rayLocal.Unitize(); // 归一化

                    // 5. 将局部光线方向转换到 Rhino 世界坐标系
                    // 我们需要相机的：前(dir)、上(up)、右(right) 三个基向量
                    ON_3dVector camForward = pRenderer->m_currentCam.dir;
                    ON_3dVector camUp = pRenderer->m_currentCam.up;
                    ON_3dVector camRight = ON_CrossProduct(camForward, camUp); // 叉乘得到右向量
                    camRight.Unitize();

                    // 最终世界空间光线方向 = x*Right + y*Up + z*Forward
                    ON_3dVector rayDirWorld = (rayLocal.x * camRight) + (rayLocal.y * camUp) + (rayLocal.z * -camForward);
                    rayDirWorld.Unitize();

                    // 6. 可视化测试：将 XYZ 方向映射为 RGB 颜色
                    // 这样你转动视角时，颜色会根据方向平滑变化，以此证明相机同步成功
                    float fColor[4];
                    fColor[0] = (float)(rayDirWorld.x * 0.5 + 0.5); // R
                    fColor[1] = (float)(rayDirWorld.y * 0.5 + 0.5); // G
                    fColor[2] = (float)(rayDirWorld.z * 0.5 + 0.5); // B
                    fColor[3] = 1.0f;
                    // ------------------

                    // 渐进式填充：低分辨率时把周围的像素块一并涂了，避免黑屏
                    for (int i = 0; i < step && (x + i) < sz.cx; ++i) {
                        for (int j = 0; j < step && (y + j) < sz.cy; ++j) {
                            pChannel->SetValue(x + i, y + j, ComponentOrder::RGBA, fColor);
                        }
                    }
                }
            }
            pChannel->Close();
            // 算完一层（或被中断），通知 Rhino 刷新
            if (pRenderer->m_pSignalUpdateInterface) pRenderer->m_pSignalUpdateInterface->SignalUpdate();
        }

        // 如果一整层顺利画完且相机没动，则提升下一轮的精度
        if (!pRenderer->m_bIsDirty && pRenderer->m_nResolutionLevel < 3) {
            pRenderer->m_nResolutionLevel++;
        }

        // 如果脏了（用户在快速旋转），极速响应 (1ms)；如果静止了，按 30FPS 节奏跑
        Sleep(pRenderer->m_bIsDirty ? 1 : 33);
    }
    return 0;
}