// CBlackHole_RealTimeDisplayMode.cpp
// 实时渲染显示模式实现：对接 Rhino 视图引擎回调，控制后台渲染器的启动与画面交接
#include "stdafx.h"
#include "CBlackHole_RealTimeDisplayMode.h"

// 构造函数：建立通讯链路
// 将自身 (this) 作为 ISignalUpdate 接口传给渲染器，完成“呼叫机”绑定
CBlackHole_RealTimeDisplayMode::CBlackHole_RealTimeDisplayMode(const CRhinoDisplayPipeline& p)
    : m_Renderer(this), RhRdk::Realtime::DisplayMode(p) {}

CBlackHole_RealTimeDisplayMode::~CBlackHole_RealTimeDisplayMode() { ShutdownRenderer(); }

// 启动渲染器
bool CBlackHole_RealTimeDisplayMode::StartRenderer
(const ON_2iSize& sz, const CRhinoDoc& d, const ON_3dmView& v, const ON_Viewport& vp, const DisplayMode* pP) {
    m_Renderer.UpdateCamera(vp);             // 初始数据同步
    return m_Renderer.StartRenderProcess(sz); // 激活后台死循环线程
}

// 响应视窗缩放：确保渲染链路的动态适配
bool CBlackHole_RealTimeDisplayMode::OnRenderSizeChanged(const ON_2iSize& sz) {
    // 重启流程确保 GPU 缓冲区无越界风险
    m_Renderer.StopRenderProcess();
    return m_Renderer.StartRenderProcess(sz);
}

void CBlackHole_RealTimeDisplayMode::ShutdownRenderer() { m_Renderer.StopRenderProcess(); }

// 将渲染结果拉取到视窗屏幕
bool CBlackHole_RealTimeDisplayMode::DrawOrLockRendererFrameBuffer(const FRAME_BUFFER_INFO_INPUTS& i, FRAME_BUFFER_INFO_OUTPUTS& o) {
    // 实时反馈：在每一帧刷新请求时，优先同步最新的相机位置
    if (i.pipeline) m_Renderer.UpdateCamera(i.pipeline->VP());

    auto* pRW = m_Renderer.RenderWindow();
    if (!pRW) return false;

    // 非阻塞同步：利用 try_to_lock 策略，如果后台还在写图则跳过此帧，防止 UI 主线程被渲染器卡死
    std::unique_lock<std::mutex> lock(m_Renderer.m_bufferMutex, std::try_to_lock);
    if (!lock.owns_lock()) return false;

    // 画面交接：调用 Rhino RDK 内部实现，将 buffer 里的像素正式推向屏幕显示
    return DrawOrLockRendererFrameBufferImpl(*this, *pRW, i, o);
}

// 释放锁：完成一帧的显示交接，允许后台线程继续往 buffer 里写入新画面
void CBlackHole_RealTimeDisplayMode::UnlockRendererFrameBuffer() {
    auto* pRW = m_Renderer.RenderWindow();
    if (pRW) UnlockRendererFrameBufferImpl(*this, *pRW);
}