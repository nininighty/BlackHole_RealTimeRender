#include "stdafx.h"
#include "SampleDisplayMode.h"

CSampleDisplayMode::CSampleDisplayMode(const CRhinoDisplayPipeline& pipeline)
	:
	m_Renderer(this),
	RhRdk::Realtime::DisplayMode(pipeline)
{
}

CSampleDisplayMode::~CSampleDisplayMode()
{
	// 析构时，如果渲染器仍在运行，则停止渲染进程
	if (m_Renderer.Running())
	{
		m_Renderer.StopRenderProcess();
	}
}

bool CSampleDisplayMode::StartRenderer(const ON_2iSize& frameSize, const CRhinoDoc& doc, const ON_3dmView& view, const ON_Viewport& vp, const DisplayMode* pParent)
{
	// 如果已经在运行，先停止当前渲染进程
	if (m_Renderer.Running())
	{
		m_Renderer.StopRenderProcess();
	}

	// 启动新的渲染进程
	return m_Renderer.StartRenderProcess(frameSize);
}

bool CSampleDisplayMode::OnRenderSizeChanged(const ON_2iSize& newFrameSize)
{
	// 当渲染尺寸发生变化时，重新启动渲染进程
	if (m_Renderer.Running())
	{
		m_Renderer.StopRenderProcess();
	}

	return m_Renderer.StartRenderProcess(newFrameSize);
}

void CSampleDisplayMode::ShutdownRenderer()
{
	// 关闭渲染器时，确保渲染进程被停止
	if (m_Renderer.Running())
	{
		m_Renderer.StopRenderProcess();
	}
}

bool CSampleDisplayMode::RendererIsAvailable(void) const
{
	// 指示该渲染器是否可用（此处始终返回可用）
	return true;
}

void CSampleDisplayMode::CreateWorld(const CRhinoDoc& doc, const ON_3dmView& view, const CDisplayPipelineAttributes& attributes)
{
	// 创建渲染世界（场景构建），当前示例中未实现
}

int CSampleDisplayMode::LastRenderedPass(void) const
{
	// 返回最后一次完成的渲染通道编号（此示例固定返回 0）
	return 0;
}

bool CSampleDisplayMode::ShowCaptureProgress(void) const
{
	// 是否显示捕获进度（此示例不显示）
	return false;
}

double CSampleDisplayMode::Progress(void) const
{
	// 返回当前渲染进度（0.0 表示无进度信息）
	return 0.0;
}

bool CSampleDisplayMode::IsRendererStarted(void) const
{
	// 判断渲染器是否已经启动
	return m_Renderer.Running();
}

bool CSampleDisplayMode::IsCompleted(void) const
{
	// 是否渲染完成（此示例始终返回未完成）
	return false;
}

bool CSampleDisplayMode::IsFrameBufferAvailable(const ON_3dmView& vp) const
{
	// 帧缓冲区是否可用（此示例始终可用）
	return true;
}

bool CSampleDisplayMode::DrawOrLockRendererFrameBuffer(const FRAME_BUFFER_INFO_INPUTS& input, FRAME_BUFFER_INFO_OUTPUTS& outputs)
{
	// 获取渲染窗口对象
	auto* pRW = m_Renderer.RenderWindow();
	if (nullptr == pRW)
		return false;

	// 调用基础实现以绘制或锁定帧缓冲区
	return DrawOrLockRendererFrameBufferImpl(*this, *pRW, input, outputs);
}

void CSampleDisplayMode::UnlockRendererFrameBuffer()
{
	// 获取渲染窗口对象
	auto* pRW = m_Renderer.RenderWindow();
	if (nullptr != pRW)
	{
		// 解锁帧缓冲区
		UnlockRendererFrameBufferImpl(*this, *pRW);
	}
}

bool CSampleDisplayMode::UseFastDraw()
{
	// 是否使用快速绘制模式（此示例不使用）
	return false;
}