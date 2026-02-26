#include "stdafx.h"
#include "SampleRdkRendererRdkPlugIn.h"
#include "SampleRdkRendererPlugIn.h"

/*
在 2026 年 2 月 26 日我们确立的 “ML 加速神经渲染” 架构下，此文件的作用如下：
功能的“修剪与修饰”：
	逻辑映射：通过 SupportsFeature 函数，你可以告诉 Rhino ：“在我的黑洞引擎模式下，不需要太阳光，也不需要传统地平面”。
	效果：这能让你的插件看起来更像是一个严谨的物理实验室工具。

异步协作的中断点：
	逻辑映射：AbortRender 是 C++ 层与 Python 层的紧急断点。
	实战方案：如果你正在通过管道（Pipe）等待 Python 返回一万个像素的推理方向，用户突然取消了。你必须在 AbortRender 里关闭管道读取，或者发送一个特定的“取消信号”给 Python 进程。

预览生成的“轻量化”：
	实战方案：利用 CreatePreview。在材质球编辑器里，你不需要进行全精度的 4 阶龙格-库塔积分。你可以专门训练一个超小型模型专门负责生成 128x128 像素的缩略图，并在此函数中调用。
*/


// 获取插件的唯一标识符
UUID CSampleRdkRendererRdkPlugIn::PlugInId() const
{
	return ::SampleRdkRendererPlugIn().PlugInID();
}

// 获取主插件对象的引用
CRhinoPlugIn& CSampleRdkRendererRdkPlugIn::RhinoPlugIn() const
{
	return ::SampleRdkRendererPlugIn();
}

// 初始化 RDK 插件部分
bool CSampleRdkRendererRdkPlugIn::Initialize()
{
	// TODO: 在此处初始化你的插件。失败时返回 false。

	return __super::Initialize();
}

// 卸载 RDK 插件部分
void CSampleRdkRendererRdkPlugIn::Uninitialize()
{
	// TODO: 在此处执行任何必要的插件清理工作。

	__super::Uninitialize();
}

// 注册扩展
void CSampleRdkRendererRdkPlugIn::RegisterExtensions() const
{
	// TODO: 通过调用 AddExtension(new MyFactory) 来添加材质/环境/纹理工厂。
	// 详见 SampleRdkMaterialCustomUI 项目。

	/** 【Gemini 注解】：
	 这里对你的黑洞项目至关重要！如果你想为黑洞开发专属的“引力透镜纹理”或
	 “物理吸积盘材质”，你就需要在这里注册自定义的纹理工厂。
	 这能让用户在材质面板里直接选中你定义的物理参数。
	*/

	__super::RegisterExtensions();
}

// 中止渲染
void CSampleRdkRendererRdkPlugIn::AbortRender()
{
	// TODO: 编写中止渲染的逻辑

	/** 【Gemini 注解】：
	 这对应了我们之前提到的 SampleAsyncCommand 中的“中止机制”。
	 当用户点击取消按钮时，这里会被触发。你需要在此处设置标志位，
	 让后台正在跑 RK4 积分或等待 Python ML 预测的线程立刻停下来，防止软件假死。
	*/
}

// 创建预览（场景级别）
bool CSampleRdkRendererRdkPlugIn::CreatePreview(const ON_2iSize& sizeImage, RhRdkPreviewQuality quality, const IRhRdkPreviewSceneServer* pSceneServer, IRhRdkPreviewCallbacks* pNotify, CRhinoDib& dibOut)
{
	// TODO: 根据指定的尺寸和质量，创建该场景的渲染预览。
	UNREFERENCED_PARAMETER(sizeImage);
	UNREFERENCED_PARAMETER(quality);
	UNREFERENCED_PARAMETER(pSceneServer);
	UNREFERENCED_PARAMETER(pNotify);
	UNREFERENCED_PARAMETER(dibOut);

	return NULL;
}

/** 【Gemini 注解】：
 上面的函数负责渲染材质球上的那个小缩略图。
 对于黑洞渲染器，如果不想让预览窗口因为复杂的 RK4 积分而卡顿，
 你可以返回 false，让 RDK 使用默认的简单方式生成预览，
 或者在此处调用你的 Python ML 模型生成一个极速的预测图。
*/

// 创建预览（纹理级别）
bool CSampleRdkRendererRdkPlugIn::CreatePreview(const ON_2iSize& sizeImage, const CRhRdkTexture& texture, CRhinoDib& dibOut)
{
	// TODO: 可选择性地创建纹理预览。
	// 返回 false 以允许 RDK 生成所有纹理预览。
	UNREFERENCED_PARAMETER(sizeImage);
	UNREFERENCED_PARAMETER(texture);
	UNREFERENCED_PARAMETER(dibOut);

	return NULL;
}

// 渲染器功能支持声明
bool CSampleRdkRendererRdkPlugIn::SupportsFeature(const UUID& uuidFeature) const
{
	// TODO: 确定当此渲染器为当前渲染器时，暴露 RDK 的哪些功能。

	if (uuidFeature == uuidFeatureCustomRenderMeshes)
		return true; // 此渲染器支持自定义渲染网格（因为它使用了迭代器）。

	if (uuidFeature == uuidFeatureDecals)
		return false; // 此渲染器不支持贴图（Decals）。

	if (uuidFeature == uuidFeatureGroundPlane)
		return false; // 此渲染器不支持地平面。

	if (uuidFeature == uuidFeatureSun)
		return false; // 此渲染器不支持太阳。

	return true;
}

/** 【Gemini 注解】：
 在黑洞物理渲染中，通常我们会禁用“太阳（Sun）”或“地平面（GroundPlane）”，
 因为克尔黑洞的时空弯曲是由度规决定的，而不是简单的点光源或平面反射。
 通过在这里返回 false，你可以精简 Rhino 的 UI，只给用户留下与黑洞相关的参数。
*/