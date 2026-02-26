#pragma once

/**
 * CSampleRdkRendererRdkPlugIn 类
 * 继承自 CRhRdkRenderPlugIn。
 * 它是主插件类在 RDK 内部的代理，专门负责处理与渲染管线相关的底层逻辑。
 */
class CSampleRdkRendererRdkPlugIn : public CRhRdkRenderPlugIn
{
public:
	CSampleRdkRendererRdkPlugIn() = default;

	// --- 基础生命周期管理 ---

	// 初始化 RDK 插件部分
	virtual bool Initialize(void) override;

	// 卸载 RDK 插件部分
	virtual void Uninitialize(void) override;

	// 返回 RDK 插件的唯一标识符（通常与主插件 ID 一致）
	virtual UUID PlugInId(void) const override;

	// --- 渲染控制接口 ---

	/**
	 * 中止渲染
	 * 当用户在 UI 界面点击“取消”或“停止”按钮时，RDK 会调用此函数。
	 */
	virtual void AbortRender(void) override;

	/** 【Gemini 注解】：
	 在你的黑洞渲染中，这是一个关键的中断开关。
	 当你的后台线程正在解 Kerr 度规下的测地线偏微分方程：
	 \[\frac{d^2x^\mu}{d\tau^2} + \Gamma^\mu_{\alpha\beta} \frac{dx^\alpha}{d\tau} \frac{dx^\beta}{d\tau} = 0\]
	 如果用户中途取消，你必须在 AbortRender 中设置原子标志位，
	 强制停止 RK4 积分循环或断开与 Python 端 ML 推理的 IPC 连接。
	*/

	// 返回主插件对象的引用
	virtual CRhinoPlugIn& RhinoPlugIn(void) const override;

protected:
	// --- 扩展功能注册 ---

	/**
	 * 注册扩展
	 * 用于向 Rhino 注册自定义的材质、纹理或环境。
	 */
	virtual void RegisterExtensions(void) const override;

	/** 【Gemini 注解】：
	 如果你为黑洞专门设计了一种“引力透镜纹理（Gravitational Lensing Texture）”，
	 你需要在这里通过 AddExtension 注册它的工厂类。
	*/

	// --- 预览渲染器（生成材质球缩略图） ---

	/**
	 * 创建场景预览
	 * 用于在材质编辑器中渲染包含该材质的预览场景。
	 */
	virtual bool CreatePreview(const ON_2iSize& sizeImage, RhRdkPreviewQuality quality, const IRhRdkPreviewSceneServer* pSceneServer, IRhRdkPreviewCallbacks* pNotify, CRhinoDib& dibOut) override;

	/**
	 * 创建纹理预览
	 * 用于在纹理面板中生成该纹理的 2D 预览图。
	 */
	virtual bool CreatePreview(const ON_2iSize& sizeImage, const CRhRdkTexture& texture, CRhinoDib& dibOut) override;

	/** 【Gemini 注解】：
	 材质球的实时预览通常需要极高的响应速度。
	 在这里，你可以调用你的 Python ML 模型生成一个低分辨率（如 128x128）的预测画面。
	 这样即便是复杂的弯曲时空效果，在材质编辑器里也能流畅预览。
	*/

	// --- 功能支持声明 ---

	/**
	 * 声明渲染器支持的功能
	 * 用于开启或关闭 Rhino 界面中的特定渲染选项。
	 */
	virtual bool SupportsFeature(const UUID& uuidFeature) const override;
};