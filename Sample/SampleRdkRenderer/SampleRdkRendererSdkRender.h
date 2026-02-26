#pragma once

/**
 * CSampleRdkRendererSdkRender 类
 * 继承自 CRhRdkSdkRender。
 * 这是真正执行渲染任务的核心类，负责管理计算线程和像素缓冲区。
 */
class CSampleRdkRendererSdkRender : public CRhRdkSdkRender
{
public:
	// 构造函数：初始化渲染上下文、插件引用、标题、图标以及是否为预览模式
	CSampleRdkRendererSdkRender(const CRhinoCommandContext& context, CRhinoRenderPlugIn& pPlugin,
		const ON_wString& sCaption, UINT idIcon, bool bPreview);

	virtual ~CSampleRdkRendererSdkRender();

	/**
	 * 线程化渲染函数
	 * 这是后台线程执行的入口。所有的物理计算或 ML 推理都应在此函数触发的流中运行。
	 */
	int ThreadedRender(void);

	// 设置是否继续保持模态状态（控制渲染窗口是否关闭）
	void SetContinueModal(bool b);

public:
	// --- CRhRdkSdkRender 重写方法 ---

	// 即使场景中没有网格也允许渲染（对于黑洞这种基于数学度规的渲染非常有用）
	virtual BOOL RenderSceneWithNoMeshes(void) override { return true; }

	// 进入/继续/退出模态循环（控制标准渲染窗口的生命周期）
	virtual BOOL RenderEnterModalLoop() override { return true; }
	virtual BOOL RenderContinueModal()  override { return m_bContinueModal; }
	virtual BOOL RenderExitModalLoop()  override { return true; }

	// 询问是否需要处理灯光表或几何体表
	virtual BOOL NeedToProcessLightTable() override;
	virtual BOOL NeedToProcessGeometryTable() override;

	// 在创建渲染窗口前的预处理
	virtual BOOL RenderPreCreateWindow() override;

	// 是否忽略某个特定的 Rhino 物体
	virtual BOOL IgnoreRhinoObject(const CRhinoObject*) override { return false; }

	// 核心启动与停止接口
	virtual BOOL StartRenderingInWindow(CRhinoView* pView, const LPCRECT pRect) override;
	virtual void StartRendering() override;
	virtual void StopRendering() override;

	// 是否复用现有的渲染窗口
	virtual bool ReuseRenderWindow(void) const override;

	// 渲染执行入口（针对完整图像或局部窗口）
	virtual CRhinoSdkRender::RenderReturnCodes Render(const ON_2iSize& sizeRender) override;
	virtual CRhinoSdkRender::RenderReturnCodes RenderWindow(CRhinoView* pView, const LPRECT pRect, bool bInPopupWindow) override;

protected:
	class CAsyncRenderContext* AsyncRC(void) const;
	// 渲染前的准备工作（如构建加速结构）
	bool SetUpRender(CRhinoView* pView, bool bQuiet);
	// 核心渲染逻辑（像素循环所在）
	bool RenderCore(void);

private:
	CRect m_RectRender;        // 渲染总区域
	CRect m_Region;            // 当前渲染区域
	HANDLE m_hRenderThread = NULL; // 后台渲染线程句柄
	ON_3dmRenderSettings m_RenderSettings; // Rhino 渲染设置

	/** 【Gemini 注解】：像素通道指针
	 * 这里的 IChannel 是你存储计算结果的地方。
	 * 除了 RGBA，你还定义了 Z 缓冲和法线通道。
	 */
	IRhRdkRenderWindow::IChannel* m_pChanRGBA = nullptr;
	IRhRdkRenderWindow::IChannel* m_pChanZ = nullptr;
	IRhRdkRenderWindow::IChannel* m_pChanNormalX = nullptr;
	IRhRdkRenderWindow::IChannel* m_pChanNormalY = nullptr;
	IRhRdkRenderWindow::IChannel* m_pChanNormalZ = nullptr;

	bool m_bPreview = false;         // 是否为预览模式
	bool m_bContinueModal = true;    // 模态循环控制标志
	volatile bool m_bCancel = false; // 线程取消标志（必须是 volatile 确保多线程可见性）
};