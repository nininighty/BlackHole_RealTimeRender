#include "stdafx.h"
#include "BlackHole_RealTimeDisplayMode.h"

// --- 渲染器实现 ---

CBlackHole_RealTimeRenderer::CBlackHole_RealTimeRenderer(RhRdk::Realtime::ISignalUpdate* pSignalUpdateInterface)
    : m_pRenderThread(0), m_pSignalUpdateInterface(pSignalUpdateInterface) {
    m_bRunning = false;
}

CBlackHole_RealTimeRenderer::~CBlackHole_RealTimeRenderer() {
    StopRenderProcess();
}

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
        m_pRenderThread = AfxBeginThread(RenderProcess, (void*)this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, 0);
        m_pRenderThread->m_bAutoDelete = FALSE;
        m_pRenderThread->ResumeThread();
    }
    return true;
}

unsigned int CBlackHole_RealTimeRenderer::RenderProcess(void* pData) {
    CBlackHole_RealTimeRenderer* pRenderer = static_cast<CBlackHole_RealTimeRenderer*>(pData);
    if (!pRenderer || !pRenderer->m_pRenderWnd) return 0;
    const ON_2iSize sz = pRenderer->m_pRenderWnd->Size();

    while (pRenderer->m_bRunning) {
        IRhRdkRenderWindow::IChannel* pChannel = pRenderer->m_pRenderWnd->OpenChannel(IRhRdkRenderWindow::chanRGBA);
        if (pChannel) {
            for (int y = 0; y < sz.cy; y++) {
                if (!pRenderer->m_bRunning) break;
                for (int x = 0; x < sz.cx; x++) {
                    float fColor[4] = { 0.1f, 0.0f, 0.2f, 1.0f }; // 暗紫色背景
                    pChannel->SetValue(x, y, ComponentOrder::RGBA, fColor);
                }
            }
            pChannel->Close();
            if (pRenderer->m_pSignalUpdateInterface) pRenderer->m_pSignalUpdateInterface->SignalUpdate();
        }
        Sleep(33);
    }
    return 0;
}

void CBlackHole_RealTimeRenderer::StopRenderProcess() {
    if (m_pRenderThread) {
        m_bRunning = false;
        WaitForSingleObject(m_pRenderThread->m_hThread, INFINITE);
        delete m_pRenderThread;
        m_pRenderThread = nullptr;
    }
    if (m_pRenderWnd) {
        delete m_pRenderWnd;
        m_pRenderWnd = nullptr;
    }
}

// --- 显示模式实现 ---

CBlackHole_RealTimeDisplayMode::CBlackHole_RealTimeDisplayMode(const CRhinoDisplayPipeline& pipeline)
    : m_Renderer(this), RhRdk::Realtime::DisplayMode(pipeline) {
}

CBlackHole_RealTimeDisplayMode::~CBlackHole_RealTimeDisplayMode() {
    ShutdownRenderer();
}

bool CBlackHole_RealTimeDisplayMode::StartRenderer(const ON_2iSize& frameSize, const CRhinoDoc& doc, const ON_3dmView& view, const ON_Viewport& vp, const DisplayMode* pParent) {
    if (m_Renderer.Running()) m_Renderer.StopRenderProcess();
    return m_Renderer.StartRenderProcess(frameSize);
}

bool CBlackHole_RealTimeDisplayMode::OnRenderSizeChanged(const ON_2iSize& newFrameSize) {
    if (m_Renderer.Running()) m_Renderer.StopRenderProcess();
    return m_Renderer.StartRenderProcess(newFrameSize);
}

void CBlackHole_RealTimeDisplayMode::ShutdownRenderer() {
    m_Renderer.StopRenderProcess();
}

bool CBlackHole_RealTimeDisplayMode::IsRendererStarted(void) const {
    return m_Renderer.Running();
}

bool CBlackHole_RealTimeDisplayMode::IsFrameBufferAvailable(const ON_3dmView& vp) const {
    return true;
}

bool CBlackHole_RealTimeDisplayMode::DrawOrLockRendererFrameBuffer(const FRAME_BUFFER_INFO_INPUTS& input, FRAME_BUFFER_INFO_OUTPUTS& outputs) {
    auto* pRW = m_Renderer.RenderWindow();
    if (nullptr == pRW) return false;
    return DrawOrLockRendererFrameBufferImpl(*this, *pRW, input, outputs);
}

void CBlackHole_RealTimeDisplayMode::UnlockRendererFrameBuffer() {
    auto* pRW = m_Renderer.RenderWindow();
    if (nullptr != pRW) UnlockRendererFrameBufferImpl(*this, *pRW);
}

// --- 补全缺失的接口 ---

void CBlackHole_RealTimeDisplayMode::CreateWorld(const CRhinoDoc& doc, const ON_3dmView& view, const CDisplayPipelineAttributes& attributes) {
}

int CBlackHole_RealTimeDisplayMode::LastRenderedPass(void) const {
    return 0;
}

bool CBlackHole_RealTimeDisplayMode::ShowCaptureProgress(void) const {
    return false;
}

double CBlackHole_RealTimeDisplayMode::Progress(void) const {
    return 0.0;
}

bool CBlackHole_RealTimeDisplayMode::IsCompleted(void) const {
    return false;
}

bool CBlackHole_RealTimeDisplayMode::UseFastDraw() {
    return false;
}