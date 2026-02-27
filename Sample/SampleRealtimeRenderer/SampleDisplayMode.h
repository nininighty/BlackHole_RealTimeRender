#pragma once

#include "SampleRenderer.h"


// 返回本显示模式的唯一 UUID
static const ON_UUID& DisplayModeId(void)
{
	// {2F7325CD-98A8-4628-B96C-75131D02CC18}
	static const GUID id = { 0x2f7325cd, 0x98a8, 0x4628, { 0xb9, 0x6c, 0x75, 0x13, 0x1d, 0x2, 0xcc, 0x18 } };
	return id;
}


// 自定义显示模式类，继承自实时显示模式基类
class CSampleDisplayMode : public RhRdk::Realtime::DisplayMode
{
public:
	CSampleDisplayMode(const CRhinoDisplayPipeline& pipeline);
	virtual ~CSampleDisplayMode();

public:
	// 返回该显示模式的类 ID
	virtual const UUID& ClassId(void) const override
	{
		return DisplayModeId();
	}

	// 显示管线成功请求锁定渲染器后调用，
	// 此时可以使用渲染器对视口进行渲染。
	// 如果用于视图捕获（以新尺寸渲染已有视图），
	// pParent 指向原始渲染引擎。
	virtual bool StartRenderer(const ON_2iSize& frameSize, const CRhinoDoc& doc, const ON_3dmView& view, const ON_Viewport& vp, const DisplayMode* pParent) override;

	// 场景尺寸改变时调用，不必从头开始，只需更新并继续
	virtual bool OnRenderSizeChanged(const ON_2iSize& newFrameSize) override;
	virtual void ShutdownRenderer() override;

	// 系统对 StartRenderer 是否会成功的最佳判断
	virtual bool RendererIsAvailable(void) const override;

	// 处理场景变化队列（构建或更新渲染世界）
	virtual void CreateWorld(const CRhinoDoc& doc, const ON_3dmView& view, const CDisplayPipelineAttributes& attributes) override;

	// 用于向克隆渲染器（视图捕获）传递当前应渲染的通道信息
	virtual int LastRenderedPass(void) const override;

	// 是否支持内置的视图捕获进度对话框
	virtual bool ShowCaptureProgress(void) const override;
	virtual double Progress(void) const override;

	// 渲染器已成功启动并完成初始化
	virtual bool IsRendererStarted(void) const override;

	// 渲染器是否已完成所有请求的渲染通道
	virtual bool IsCompleted(void) const override;

	// 在绘制帧时调用，用于判断在调用 LockRendererFrameBuffer 时
	// 是否已有可用数据。
	// 若图像仍在渲染中，应返回 false。
	virtual bool IsFrameBufferAvailable(const ON_3dmView& vp) const override;

	// 若已将帧绘制到当前 OpenGL 视图，
	// 或通过 outputs 提供了其它绘制方式，则返回 true。
	// 若帧尚不可用，应返回 false，让框架等待。
	virtual bool DrawOrLockRendererFrameBuffer(const FRAME_BUFFER_INFO_INPUTS& input, FRAME_BUFFER_INFO_OUTPUTS& outputs) override;
	virtual void UnlockRendererFrameBuffer() override;

	// 若返回 true，则启用快速绘制机制。
	// 默认行为是旧机制：管线会在多次短暂等待后
	// 尝试获取帧缓冲。
	// 此函数在 StartRenderer() 之前调用。
	virtual bool UseFastDraw() override;

private:
	CSampleRenderer m_Renderer;
};


// ALB 2018.8.29 该对象必须继承自 CRhRdkObject，
// 以确保能够被正确清理。
// 这是由于 SDK 中的一个缺陷：
// RhRdk::Realtime::DisplayMode::Factory 缺少 Delete 函数。
// 相关问题：https://mcneel.myjetbrains.com/youtrack/issue/RH-46405
class CDisplayModeFactory : public RhRdk::Realtime::DisplayMode::Factory, public CRhRdkObject
{
public:
	// 返回显示模式名称
	virtual ON_wString Name(void) const override
	{
		return L"SampleRealtimeRenderer";
	}

	// 返回显示模式类 ID
	virtual const UUID& ClassId(void) const override
	{
		return DisplayModeId();
	}

	// 创建一个显示引擎实例
	virtual std::shared_ptr<RhRdk::Realtime::DisplayMode> MakeDisplayEngine(const CRhinoDisplayPipeline& pipeline) const override
	{
		return std::make_shared<CSampleDisplayMode>(pipeline);
	}

	// 扩展虚函数接口（此处未使用）
	void* EVF(const wchar_t*, void*) override { return nullptr; }
};