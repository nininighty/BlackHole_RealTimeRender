#pragma once
#include "stdafx.h"
#include <memory> // 支持 std::shared_ptr

// 为黑洞显示模式生成一份独一无二的GUID
static const ON_UUID& BlackHoleDisplayModeId() {
    static const GUID id = { 0xa1b2c3d4, 0xe5f6, 0x4a5b, { 0x8c, 0x9d, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab } };
    return id;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// 1. 渲染器类：负责后台算像素
class CBlackHole_RealTimeRenderer {
public:
    // 构造函数：初始化成员变量，设置初始运行状态为 false
    CBlackHole_RealTimeRenderer(RhRdk::Realtime::ISignalUpdate* pSignalUpdateInterface = nullptr);
    // 析构函数：确保线程安全退出
    ~CBlackHole_RealTimeRenderer();

    // 启动渲染流程：初始化缓冲区并开启后台线程
    bool StartRenderProcess(const ON_2iSize& frameSize);
    // 停止渲染流程：安全销毁线程
    void StopRenderProcess();
    // 状态检查
    bool Running() const { return m_bRunning; }
    // 获取渲染窗口接口
    IRhRdkRenderWindow* RenderWindow() const { return m_pRenderWnd; }

private:
    // 后台线程函数
    static unsigned int RenderProcess(void* pData);

    bool m_bRunning = false;
    CWinThread* m_pRenderThread = nullptr;
    IRhRdkRenderWindow* m_pRenderWnd = nullptr;
    RhRdk::Realtime::ISignalUpdate* m_pSignalUpdateInterface = nullptr;
};

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// 2. 显示模式类：对接视窗管线
class CBlackHole_RealTimeDisplayMode : public RhRdk::Realtime::DisplayMode {
public:
    CBlackHole_RealTimeDisplayMode(const CRhinoDisplayPipeline& pipeline);
    virtual ~CBlackHole_RealTimeDisplayMode();

    virtual const UUID& ClassId(void) const override { return BlackHoleDisplayModeId(); }

    // --- 核心生命周期 ---
    virtual bool StartRenderer(const ON_2iSize& frameSize, const CRhinoDoc& doc, const ON_3dmView& view, const ON_Viewport& vp, const DisplayMode* pParent) override;
    virtual bool OnRenderSizeChanged(const ON_2iSize& newFrameSize) override;
    virtual void ShutdownRenderer() override;

    // --- 绘制同步 ---
    virtual bool IsFrameBufferAvailable(const ON_3dmView& vp) const override;
    virtual bool DrawOrLockRendererFrameBuffer(const FRAME_BUFFER_INFO_INPUTS& input, FRAME_BUFFER_INFO_OUTPUTS& outputs) override;
    virtual void UnlockRendererFrameBuffer() override;

    // --- 必须补全的虚函数，否则会报 C2259 错误 ---
    virtual void CreateWorld(const CRhinoDoc& doc, const ON_3dmView& view, const CDisplayPipelineAttributes& attributes) override;
    virtual int LastRenderedPass(void) const override;
    virtual bool ShowCaptureProgress(void) const override;
    virtual double Progress(void) const override;
    virtual bool IsCompleted(void) const override;
    virtual bool UseFastDraw() override;

    virtual bool RendererIsAvailable(void) const override { return true; }
    virtual bool IsRendererStarted(void) const override;

private:
    CBlackHole_RealTimeRenderer m_Renderer;
};

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// 3. 工厂类
class CBlackHole_RealTimeDisplayModeFactory : public RhRdk::Realtime::DisplayMode::Factory, public CRhRdkObject {
public:
    virtual ON_wString Name(void) const override { return L"Black Hole Realtime"; }
    virtual const UUID& ClassId(void) const override { return BlackHoleDisplayModeId(); }
    virtual std::shared_ptr<RhRdk::Realtime::DisplayMode> MakeDisplayEngine(const CRhinoDisplayPipeline& pipeline) const override {
        return std::make_shared<CBlackHole_RealTimeDisplayMode>(pipeline);
    }
    void* EVF(const wchar_t*, void*) override { return nullptr; }
};