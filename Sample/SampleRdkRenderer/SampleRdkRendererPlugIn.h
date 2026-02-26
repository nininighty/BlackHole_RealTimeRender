#pragma once

#include "SampleRdkRendererEventWatcher.h"

class CSampleRdkRendererRdkPlugIn;

/**
 * CSampleRdkRendererPlugIn 类
 * 继承自 CRhinoRenderPlugIn，这是所有 Rhino 渲染插件的基类。
 */
class CSampleRdkRendererPlugIn : public CRhinoRenderPlugIn
{
public:
	// CSampleRdkRendererPlugIn 构造函数。
	// 当插件加载且 "thePlugIn" 对象被构造时调用此函数。
	// 插件加载后，将调用 CSampleRdkRendererPlugIn::OnLoadPlugIn()。
	// 构造函数应保持简单且稳健。
	// 请将任何可能失败的操作移至 CSampleRdkRendererPlugIn::OnLoadPlugIn() 中进行。
	CSampleRdkRendererPlugIn();

	// CSampleRdkRendererPlugIn 析构函数。
	// 当插件卸载时用于销毁 "thePlugIn" 对象。
	// 在 DLL 卸载前，会立即调用 CSampleRdkRendererPlugIn::OnUnloadPlugin()。
	// 此处不要做过多操作。确保清理了所有通过 onmalloc()、onrealloc() 等分配的内存。
	virtual ~CSampleRdkRendererPlugIn() = default;

	// --- 必须重写的基类方法 ---

	// 插件名称显示字符串。
	// 此名称将在 Rhino 加载插件时、插件帮助菜单中以及 Rhino 的插件管理界面显示。
	const wchar_t* PlugInName() const override;

	// 插件版本显示字符串。
	// 此名称将在 Rhino 加载插件时以及插件管理界面显示。
	const wchar_t* PlugInVersion() const override;

	// 插件唯一标识符 (GUID)。
	// 此标识符由 Rhino 用于管理插件。
	GUID PlugInID() const override;

	// --- 其他重要重写方法 ---

	// 在插件加载且构造函数运行后调用。
	// 适合进行重要的初始化、许可证检查等。此函数必须返回 TRUE 插件才能继续加载。
	BOOL OnLoadPlugIn() override;

	/** 【Gemini 注解】：
	 对于你的黑洞项目，这里就是“启动引擎”的地方。
	 你可以在此处初始化与后台 Python 推理进程的管道通信（IPC），
	 或者预加载训练好的神经网络权重文件（如 .onnx 模型）。
	*/

	// 当插件即将卸载时调用一次。
	// 此时 Rhino 主窗口已销毁，部分 SDK 管理器已被删除，且没有活动文档或视图。
	// 因此，在此处应仅操作你自己的对象或工具。
	void OnUnloadPlugIn() override;

	// --- 渲染核心重写方法 ---

	// 当用户执行“渲染”或“渲染预览”命令且此插件为默认引擎时调用。
	CRhinoCommand::result Render(const CRhinoCommandContext& context, bool bPreview) override;

	// 局部渲染窗口。
	// 当用户在视口中框选特定区域进行渲染时调用。
	CRhinoCommand::result RenderWindow(
		const CRhinoCommandContext& context,
		bool render_preview,
		CRhinoView* view,
		const LPRECT rect,
		bool bInWindow,
		bool bBlowUp
	) override;

	// 将渲染结果保存为图像文件。
	BOOL SaveRenderedImage(ON_wString filename) override;

	// 关闭渲染窗口。
	BOOL CloseRenderWindow() override;

	// --- 渲染辅助方法 ---

	// 静默渲染（通常用于脚本调用）。
	CRhinoCommand::result RenderQuiet(const CRhinoCommandContext& context, bool bPreview);

	// 询问场景是否发生改变。
	BOOL SceneChanged() const;

	// 设置场景改变状态。
	void SetSceneChanged(BOOL bChanged);

	// 询问光照是否发生改变。
	BOOL LightingChanged() const;

	// 设置光照改变状态。
	void SetLightingChanged(BOOL bChanged);

	// 获取主框架资源 ID。
	UINT MainFrameResourceID() const;

private:
	ON_wString m_plugin_version;                // 插件版本字符串
	CSampleRdkRendererEventWatcher m_event_watcher; // 我们之前分析过的“场景感知器”
	class CSampleRdkRendererRdkPlugIn* m_pRdkPlugIn; // 指向 RDK 具体实现的指针

	/** 【Gemini 注解】：
	 m_event_watcher 是本类的成员变量，这意味着主插件始终持有一个对场景变动的实时监听。
	 它与 m_pRdkPlugIn 协同工作：监视器发现场景变了，就通过主插件告诉 RDK 引擎重新计算黑洞像素。
	*/

	// TODO: 在此处添加额外的类成员信息
};

// 返回指向唯一的 CSampleRdkRendererPlugIn 对象的引用（单例模式）
CSampleRdkRendererPlugIn& SampleRdkRendererPlugIn(void);