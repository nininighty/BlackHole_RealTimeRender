#include "stdafx.h"
#include "rhinoSdkPlugInDeclare.h"
#include "SampleRdkRendererPlugIn.h"
#include "SampleRdkRendererRdkPlugIn.h"
#include "SampleRdkRendererSdkRender.h"
#include "Resource.h"

// 插件对象必须在任何派生自 CRhinoCommand 的插件类之前构造。
// #pragma init_seg(lib) 确保了这一顺序。
#pragma warning(push)
#pragma warning(disable : 4073)
#pragma init_seg(lib)
#pragma warning(pop)

// Rhino 插件声明
RHINO_PLUG_IN_DECLARE

// Rhino 插件名称
// 为此插件提供一个简短、易懂的名称。
RHINO_PLUG_IN_NAME(L"SampleRdkRenderer");

// Rhino 插件 ID
// 为此插件提供一个唯一的 UUID。
RHINO_PLUG_IN_ID(L"E3476DAB-0118-4D22-9F42-63EC01B39E4B");

/** 【Gemini 注解】：
 这里非常关键！ 你的黑洞插件需要一个独一无二的身份证（UUID）。
 在复试演示时，如果你修改了这个 ID 和名字，它就会在 Rhino 的“插件管理器”里
 以你自定义的名字显示，这能体现你对项目的深度掌握。
*/

// Rhino 插件版本
// 为此插件提供一个版本号字符串。
RHINO_PLUG_IN_VERSION(__DATE__ "  " __TIME__)

// Rhino 插件描述
// 为此插件提供一段描述文字。
RHINO_PLUG_IN_DESCRIPTION(L"SampleRdkRenderer plug-in for Rhinoceros®");

// Rhino 插件图标资源 ID
// 关联插件图标。图标资源应包含 16, 24, 32, 48 和 256 像素尺寸的图像。
RHINO_PLUG_IN_ICON_RESOURCE_ID(IDI_ICON);

// Rhino 插件开发者声明
// 填写开发者的信息。注意，所有这些声明必须存在，否则插件将无法加载。
RHINO_PLUG_IN_DEVELOPER_ORGANIZATION(L"Robert McNeel & Associates");
RHINO_PLUG_IN_DEVELOPER_ADDRESS(L"3670 Woodland Park Avenue North\r\nSeattle WA 98103");
RHINO_PLUG_IN_DEVELOPER_COUNTRY(L"United States");
RHINO_PLUG_IN_DEVELOPER_PHONE(L"206-545-6877");
RHINO_PLUG_IN_DEVELOPER_FAX(L"206-545-7321");
RHINO_PLUG_IN_DEVELOPER_EMAIL(L"devsupport@mcneel.com");
RHINO_PLUG_IN_DEVELOPER_WEBSITE(L"http://www.rhino3d.com");
RHINO_PLUG_IN_UPDATE_URL(L"https://github.com/mcneel/rhino-developer-samples");

// 唯一的 CSampleRdkRendererPlugIn 对象
static class CSampleRdkRendererPlugIn thePlugIn;

CSampleRdkRendererPlugIn& SampleRdkRendererPlugIn(void)
{
	// 返回唯一插件对象的引用
	return thePlugIn;
}

CSampleRdkRendererPlugIn::CSampleRdkRendererPlugIn()
{
	// 描述：
	//   构造函数。当插件被加载且 "thePlugIn" 被构造时调用。
	//   插件加载后，将调用 OnLoadPlugIn()。
	//   构造函数应保持简单且稳健。任何可能失败的操作请放在 OnLoadPlugIn() 中。

	m_plugin_version = RhinoPlugInVersion();

	m_pRdkPlugIn = NULL;
}

const wchar_t* CSampleRdkRendererPlugIn::PlugInName() const
{
	// 描述：
	//   插件名称显示字符串。此名称在 Rhino 加载插件、帮助菜单
	//   以及管理插件的界面中显示。

	return RhinoPlugInName();
}

const wchar_t* CSampleRdkRendererPlugIn::PlugInVersion() const
{
	// 描述：
	//   插件版本显示字符串。

	return m_plugin_version;
}

GUID CSampleRdkRendererPlugIn::PlugInID() const
{
	// 描述：
	//   插件唯一标识符。Rhino 使用此标识符来管理插件。

	return ON_UuidFromString(RhinoPlugInId());
}

BOOL CSampleRdkRendererPlugIn::OnLoadPlugIn()
{
	// 描述：
	//   在插件加载完成且构造函数运行后调用。
	//   适合进行重要初始化、许可证检查等操作。
	//   此函数必须返回 TRUE 插件才能继续加载。

	// 备注：
	//   插件在 Rhino 启动并创建默认文档之后才会加载。
	//   因为默认文档已存在，CRhinoEventWatcher::On????Document() 函数
	//   不会被默认文档调用。如果需要进行文档初始化或同步，请在此处处理。
	//   派生类不强制要求调用 CPlugIn::OnLoadPlugIn()。

	// 如果此处触发断言，很可能是 RDK 尚未被 Rhino 加载。
	// 如果发生这种情况，请加载保护你的插件，重启 Rhino 并通过拖拽或
	// 插件管理器手动加载 rdk.rhp，然后取消加载保护。
	ASSERT(RhRdkIsAvailable());

	/** 【Gemini 注解】：
	 这里是渲染器心脏起搏的地方。 你的黑洞引擎（m_pRdkPlugIn）
	 在这里被实例化。如果初始化失败，插件就不会在渲染引擎列表中出现。
	 同时也启动了我们之前分析过的“感知器官”（m_event_watcher）。
	*/

	m_pRdkPlugIn = new CSampleRdkRendererRdkPlugIn;
	ON_wString str;
	if (!m_pRdkPlugIn->Initialize())
	{
		delete m_pRdkPlugIn;
		m_pRdkPlugIn = NULL;
		str.Format(L"Failed to load %ls, version %ls. RDK initialization failed\n", PlugInName(), PlugInVersion());
		RhinoApp().Print(str);
		return FALSE;
	}

	str.Format(L"Loading %ls, version %ls\n", PlugInName(), PlugInVersion());
	RhinoApp().Print(str);

	m_event_watcher.Register();
	m_event_watcher.Enable(TRUE);

	CRhinoDoc* doc = RhinoApp().ActiveDoc();
	if (doc)
		m_event_watcher.OnNewDocument(*doc);

	return TRUE;
}

void CSampleRdkRendererPlugIn::OnUnloadPlugIn()
{
	// 描述：
	//   当插件即将卸载时调用一次。此时，Rhino 的主窗口已被销毁，
	//   部分 SDK 管理器已被删除。此时也没有活动的文档或视图。
	//   因此，在此处应仅操作你自己的对象或工具。

	m_event_watcher.Enable(FALSE);
	m_event_watcher.UnRegister();

	if (NULL != m_pRdkPlugIn)
	{
		m_pRdkPlugIn->Uninitialize();
		delete m_pRdkPlugIn;
		m_pRdkPlugIn = NULL;
	}
}

CRhinoCommand::result CSampleRdkRendererPlugIn::Render(const CRhinoCommandContext& context, bool bPreview)
{
	// 描述：
	//   如果此插件是渲染插件且被设为默认引擎，点击“渲染”或“渲染预览”时将调用此函数。
	// 参数：
	//   context: 传递给渲染命令的命令参数。
	//   bPreview: 如果为 true，则期望进行更快速、质量较低的渲染。

	/** 【Gemini 注解】：
	 这是“导演”喊开始的地方！
	 当用户点击 Rhino 的渲染按钮，它会创建一个 CSampleRdkRendererSdkRender 实例，
	 并调用它的 Render 方法。对于你来说，这里就是开启 RK4 积分或 ML 模型推理的触发器。
	*/

	const CRhinoDoc* doc = context.Document();
	if (nullptr == doc)
		return CRhinoCommand::failure;

	CSampleRdkRendererSdkRender render(context, *this, L"SampleRdkRenderer", IDI_RENDER, bPreview);
	const auto size = render.RenderSize(*doc, true);
	if (CRhinoSdkRender::render_ok != render.Render(size))
		return CRhinoCommand::failure;

	return CRhinoCommand::success;
}

CRhinoCommand::result CSampleRdkRendererPlugIn::RenderWindow(
	const CRhinoCommandContext& context,
	bool render_preview,
	CRhinoView* view,
	const LPRECT rect,
	bool bInWindow,
	bool bBlowUp
)
{
	UNREFERENCED_PARAMETER(bBlowUp);

	// 描述：
	//   局部渲染窗口命令。
	// 参数：
	//   rect: 视口窗口坐标中的渲染矩形区域。

	CSampleRdkRendererSdkRender render(context, *this, L"SampleRdkRenderer", IDI_RENDER, render_preview);
	if (CRhinoSdkRender::render_ok == render.RenderWindow(view, rect, bInWindow))
		return CRhinoCommand::success;

	return CRhinoCommand::failure;
}

CRhinoCommand::result CSampleRdkRendererPlugIn::RenderQuiet(const CRhinoCommandContext& context, bool bPreview)
{
	UNREFERENCED_PARAMETER(context);
	UNREFERENCED_PARAMETER(bPreview);

	return CRhinoCommand::failure;
}

BOOL CSampleRdkRendererPlugIn::SaveRenderedImage(ON_wString filename)
{
	// 描述：
	//   脚本发送的保存渲染图像到文件的消息。
	return FALSE;
}

BOOL CSampleRdkRendererPlugIn::CloseRenderWindow()
{
	// 描述：
	//   关闭渲染窗口通知。
	return FALSE;
}

// 渲染相关方法

UINT CSampleRdkRendererPlugIn::MainFrameResourceID() const
{
	return IDR_RENDER;
}

BOOL CSampleRdkRendererPlugIn::SceneChanged() const
{
	// 通过感知器官（event_watcher）判断场景是否改变
	return m_event_watcher.RenderSceneModified();
}

void CSampleRdkRendererPlugIn::SetSceneChanged(BOOL bChanged)
{
	m_event_watcher.SetRenderMeshFlags(bChanged);
	m_event_watcher.SetMaterialFlags(bChanged);
}

BOOL CSampleRdkRendererPlugIn::LightingChanged() const
{
	// 判断光照是否改变
	return m_event_watcher.RenderLightingModified();
}

void CSampleRdkRendererPlugIn::SetLightingChanged(BOOL bChanged)
{
	m_event_watcher.SetLightFlags(bChanged);
}