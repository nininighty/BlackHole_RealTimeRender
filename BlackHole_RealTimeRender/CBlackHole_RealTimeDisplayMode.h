// CBlackHole_RealTimeDisplayMode.h
// 实时渲染显示模式类 (DisplayMode 门户)，负责对接 Rhino 视窗引擎、触发渲染器开关并管理帧缓冲显示
#pragma once
#include "stdafx.h"
#include "CBlackHole_RealTimeRenderer.h"

// 显示模式静态唯一标识符
static const ON_UUID& BlackHoleDisplayModeId() {
    static const GUID id = { 0xb1a0401e, 0x5146, 0xe710, { 0x8c, 0x9d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 } };
    return id;
}

// 黑洞实时显示模式实现类
class CBlackHole_RealTimeDisplayMode : public RhRdk::Realtime::DisplayMode {
public:
    CBlackHole_RealTimeDisplayMode(const CRhinoDisplayPipeline& p);
    virtual ~CBlackHole_RealTimeDisplayMode();

    // ==========================================
    // 1. 核心生命周期回调 (由 Rhino 引擎调用)

    virtual bool StartRenderer(const ON_2iSize& sz, const CRhinoDoc& d, const ON_3dmView& v, const ON_Viewport& vp, const DisplayMode* pP) override;
    virtual void ShutdownRenderer() override;
    virtual bool IsRendererStarted() const override { return m_Renderer.Running(); }

    // ==========================================
    // 2. 帧缓冲与画面呈现接口

    // 绘制或锁定帧缓冲
    virtual bool DrawOrLockRendererFrameBuffer(const FRAME_BUFFER_INFO_INPUTS& i, FRAME_BUFFER_INFO_OUTPUTS& o) override;
    virtual void UnlockRendererFrameBuffer() override;
    virtual bool OnRenderSizeChanged(const ON_2iSize& sz) override; // 响应窗口缩放
    virtual bool IsFrameBufferAvailable(const ON_3dmView& v) const override { return true; }

    // ==========================================
    // 3. 渲染状态维护 (补全虚函数)

    virtual const UUID& ClassId(void) const override { return BlackHoleDisplayModeId(); }
    virtual void CreateWorld(const CRhinoDoc&, const ON_3dmView&, const CDisplayPipelineAttributes&) override {}
    virtual int LastRenderedPass() const override { return 0; }
    virtual bool ShowCaptureProgress() const override { return false; }
    virtual double Progress() const override { return 0.0; }
    virtual bool IsCompleted() const override { return false; }
    virtual bool UseFastDraw() override { return false; }
    virtual bool RendererIsAvailable() const override { return true; }

private:
    // ==========================================
    // 4. 内部渲染中枢

    CBlackHole_RealTimeRenderer m_Renderer; // DisplayMode管理的渲染器
};

// 显示模式工厂类：负责向 Rhino 注册并创建上述显示模式实例
class CBlackHole_RealTimeDisplayModeFactory : public RhRdk::Realtime::DisplayMode::Factory, public CRhRdkObject {
public:
    // 插件在 Rhino 视图菜单中显示的名称
    virtual ON_wString Name() const override { return L"黑洞实时渲染"; }
    virtual const UUID& ClassId() const override { return BlackHoleDisplayModeId(); }

    // 创建显示引擎实例：Rhino 切换模式时自动调用
    virtual std::shared_ptr<RhRdk::Realtime::DisplayMode> MakeDisplayEngine(const CRhinoDisplayPipeline& p) const override {
        return std::make_shared<CBlackHole_RealTimeDisplayMode>(p);
    }

    void* EVF(const wchar_t*, void*) override { return nullptr; }
};