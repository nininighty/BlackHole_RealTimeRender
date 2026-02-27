// BlackHole_RealTimeRenderPlugIn.cpp : 定义插件的初始化流程
//

#include "stdafx.h"
#include "rhinoSdkPlugInDeclare.h"
#include "BlackHole_RealTimeRenderPlugIn.h"
#include "Resource.h"
#include "BlackHole_RealTimeRenderRdkPlugIn.h"
#include "BlackHole_RealTimeRenderSdkRender.h"
#include "BlackHole_RealTimeDisplayMode.h"

// 插件对象必须在任何继承自 CRhinoCommand 的插件类之前构造。
// #pragma init_seg(lib) 确保这一点。
#pragma warning(push)
#pragma warning(disable : 4073)
#pragma init_seg(lib)
#pragma warning(pop)

// Rhino 插件声明
RHINO_PLUG_IN_DECLARE

// Rhino 插件名称
// 为此插件提供一个简短、友好的名称。
RHINO_PLUG_IN_NAME(L"BlackHole_RealTimeRender");

// Rhino 插件 id
// 为此插件提供一个唯一的 uuid。
RHINO_PLUG_IN_ID(L"3FFB4850-AE16-4279-8978-0318FF771DD6");

// Rhino 插件版本
// 为此插件提供一个版本号字符串。
RHINO_PLUG_IN_VERSION(__DATE__ "  " __TIME__)

// Rhino 插件描述
// 为此插件提供一个描述。
RHINO_PLUG_IN_DESCRIPTION(L"BlackHole_RealTimeRender plug-in for Rhinoceros®");

// Rhino 插件图标资源 id
// 为此插件提供一个图标资源。
// 图标资源应包含 16、24、32、48 和 256 像素尺寸的图像。
RHINO_PLUG_IN_ICON_RESOURCE_ID(IDI_ICON);

// Rhino 插件开发者信息声明
// TODO：用你的公司信息填写下面的开发者信息。
// 注意：这些声明必须全部存在，否则插件不会加载。
RHINO_PLUG_IN_DEVELOPER_ORGANIZATION(L"Nighty");
RHINO_PLUG_IN_DEVELOPER_ADDRESS(L"Andromeda");
RHINO_PLUG_IN_DEVELOPER_COUNTRY(L"Andromeda");
RHINO_PLUG_IN_DEVELOPER_PHONE(L"None");
RHINO_PLUG_IN_DEVELOPER_FAX(L"None");
RHINO_PLUG_IN_DEVELOPER_EMAIL(L"1204970335@foxmail.com");
RHINO_PLUG_IN_DEVELOPER_WEBSITE(L"http://nininighty.xin/");
RHINO_PLUG_IN_UPDATE_URL(L"http://nininighty.xin/");

// 唯一的 CBlackHole_RealTimeRenderPlugIn 对象
static class CBlackHole_RealTimeRenderPlugIn thePlugIn;

/////////////////////////////////////////////////////////////////////////////
// CBlackHole_RealTimeRenderPlugIn 定义

CBlackHole_RealTimeRenderPlugIn& BlackHole_RealTimeRenderPlugIn()
{
	// 返回唯一的 CBlackHole_RealTimeRenderPlugIn 对象引用
	return thePlugIn;
}

CBlackHole_RealTimeRenderPlugIn::CBlackHole_RealTimeRenderPlugIn()
{
	// 说明：
	//   CBlackHole_RealTimeRenderPlugIn 构造函数。当插件加载并构造
	//   “thePlugIn”对象时调用。插件加载后会调用
	//   CBlackHole_RealTimeRenderPlugIn::OnLoadPlugIn()。
	//   构造函数应该保持简单和稳定。任何可能失败的操作
	//   应该放在 CBlackHole_RealTimeRenderPlugIn::OnLoadPlugIn() 中。

	// TODO：在这里添加构造代码
	m_plugin_version = RhinoPlugInVersion();
}

/////////////////////////////////////////////////////////////////////////////
// 必须重写的函数

const wchar_t* CBlackHole_RealTimeRenderPlugIn::PlugInName() const
{
	// 说明：
	//   插件名称显示字符串。该名称在 Rhino 加载插件时、
	//   插件帮助菜单中，以及 Rhino 的插件管理界面中显示。

	// TODO：返回插件的简短友好名称。
	return RhinoPlugInName();
}

const wchar_t* CBlackHole_RealTimeRenderPlugIn::PlugInVersion() const
{
	// 说明：
	//   插件版本显示字符串。该字符串在 Rhino 加载插件时
	//   以及插件管理界面中显示。

	// TODO：返回插件版本号。
	return m_plugin_version;
}

GUID CBlackHole_RealTimeRenderPlugIn::PlugInID() const
{
	// 说明：
	//   插件唯一标识符。该标识符用于 Rhino 管理插件。

	// TODO：返回插件的唯一标识符。
	// {3FFB4850-AE16-4279-8978-0318FF771DD6}
	return ON_UuidFromString(RhinoPlugInId());
}

/////////////////////////////////////////////////////////////////////////////
// 额外重写的函数

// 实现加载时间重写，告诉 Rhino 在启动时就激活此插件
// ---------------------------------------------------------
CRhinoPlugIn::plugin_load_time CBlackHole_RealTimeRenderPlugIn::PlugInLoadTime() {
	return CRhinoPlugIn::load_plugin_at_startup;
}

// ---------------------------------------------------------
// 插件加载主入口，在这里完成 RDK 初始化和实时模式注入
// ---------------------------------------------------------
// 插件加载主入口：执行初始化并确保黑洞模式挂载到视窗下拉菜单
BOOL CBlackHole_RealTimeRenderPlugIn::OnLoadPlugIn() {
	ASSERT(RhRdkIsAvailable());

	// 1. 初始化 RDK 插件核心
	m_pRdkPlugIn = new CBlackHole_RealTimeRenderRdkPlugIn;
	if (!m_pRdkPlugIn->Initialize()) {
		delete m_pRdkPlugIn;
		m_pRdkPlugIn = nullptr;
		return FALSE;
	}

	// 2. 注册显示模式工厂：这是实时引擎能被视窗识别的前提
	RhRdk::Realtime::DisplayMode::Factory::Register(std::make_unique<CBlackHole_RealTimeDisplayModeFactory>());

	// 3. 获取或创建显示属性条目
	DisplayAttrsMgrListDesc* pDesc = CRhinoDisplayAttrsMgr::FindDisplayAttrsDesc(BlackHoleDisplayModeId());

	// 如果条目不存在，则创建一个新条目
	if (nullptr == pDesc) {
		pDesc = CRhinoDisplayAttrsMgr::AppendNewEntry();
	}

	// 4. 强制刷新配置：确保菜单标志和实时引擎 ID 始终正确
	if (nullptr != pDesc && nullptr != pDesc->m_pAttrs) {
		// 【关键】：强制开启菜单显示，修正之前运行可能留下的错误配置
		pDesc->m_bAddToMenu = true;

		// 绑定身份标识、显示名称以及关联的实时渲染 ID
		pDesc->m_pAttrs->SetUuid(BlackHoleDisplayModeId());
		pDesc->m_pAttrs->SetName(L"Black Hole Realtime");
		pDesc->m_pAttrs->SetRealtimeDisplayId(BlackHoleDisplayModeId());

		// 绑定底层绘制管线为 OpenGL
		pDesc->m_pAttrs->SetPipeline(&ON_CLASS_RTTI(CRhinoDisplayPipeline_OGL));

		// 保存修改到 Rhino 用户的 Profile，确保重启后依然生效
		CRhinoDisplayAttrsMgr::SaveProfile(RhinoApp().ProfileContext());
	}

	ON_wString str;
	str.Format(L"Loading %s (Realtime Mode Loaded Successfully)\n", PlugInName());
	RhinoApp().Print(str);

	return TRUE;
}

void CBlackHole_RealTimeRenderPlugIn::OnUnloadPlugIn()
{
	// 说明：
	//   当插件即将被卸载时调用一次。
	//   此时 Rhino 主窗口已经被销毁，
	//   并且一些 SDK 管理器也已经被删除。
	//   此时也没有活动文档或活动视图。
	//   因此这里只应该操作你自己的对象或工具。

	// TODO：在这里添加渲染插件清理代码。
	m_event_watcher.Enable(FALSE);
	m_event_watcher.UnRegister();
}

/////////////////////////////////////////////////////////////////////////////
// 渲染相关重写

CRhinoCommand::result CBlackHole_RealTimeRenderPlugIn::Render(const CRhinoCommandContext& context, bool bPreview)
{
	// 说明：
	//   如果该应用既是渲染插件，
	//   又被设置为默认渲染引擎，
	//   当执行 Render 或 RenderPreview 命令时会调用此函数。
	// 参数：
	//   context [输入] 传递给渲染命令的参数。
	//   bPreview [输入] 如果为 true，则期望更快但质量较低的渲染。

	const auto pDoc = context.Document();
	if (nullptr == pDoc)
		return CRhinoCommand::failure;

	CBlackHole_RealTimeRenderSdkRender render(context, *this, L"[BlackHole_RealTimeRender", IDI_RENDER, bPreview);
	const auto size = render.RenderSize(*pDoc, true);
	if (CRhinoSdkRender::render_ok != render.Render(size))
		return CRhinoCommand::failure;

	return CRhinoCommand::success;
}

CRhinoCommand::result CBlackHole_RealTimeRenderPlugIn::RenderWindow(
	const CRhinoCommandContext& context,
	bool render_preview,
	CRhinoView* view,
	const LPRECT rect,
	bool bInWindow,
	bool bBlowUp
)
{
	UNREFERENCED_PARAMETER(bBlowUp);

	// 说明：
	//   如果该应用既是渲染插件，
	//   又被设置为默认渲染引擎，
	//   Render 命令会调用此函数。
	// 参数：
	//   context [输入] 传递给渲染命令的参数。
	//   bPreview [输入] 如果为 true，则期望更快但质量较低的渲染。
	//   view [输入] 要渲染的视图。
	//   rect [输入] 视口窗口坐标中的渲染矩形区域。

	CBlackHole_RealTimeRenderSdkRender render(context, *this, L"BlackHole_RealTimeRender", IDI_RENDER, render_preview);
	if (CRhinoSdkRender::render_ok == render.RenderWindow(view, rect, bInWindow))
		return CRhinoCommand::success;

	return CRhinoCommand::failure;
}

CRhinoCommand::result CBlackHole_RealTimeRenderPlugIn::RenderQuiet(const CRhinoCommandContext& context, bool bPreview)
{
	UNREFERENCED_PARAMETER(context);
	UNREFERENCED_PARAMETER(bPreview);

	return CRhinoCommand::failure;
}

BOOL CBlackHole_RealTimeRenderPlugIn::SaveRenderedImage(ON_wString filename)
{
	// 说明：
	//   从脚本发送的消息，用于将渲染结果保存为文件。
	// 参数：
	//   filename [输入] 要保存的文件名。

	// TODO：在这里添加保存文件代码。
	return FALSE;
}

BOOL CBlackHole_RealTimeRenderPlugIn::CloseRenderWindow()
{
	// 说明：
	//   关闭渲染窗口通知。
	//   当渲染完成并且渲染窗口不再是模态窗口时调用。
	//   （如果你在 RenderCommand 返回后仍然保留窗口）

	return FALSE;
}

// 渲染相关方法

UINT CBlackHole_RealTimeRenderPlugIn::MainFrameResourceID() const
{
	return IDR_RENDER;
}

BOOL CBlackHole_RealTimeRenderPlugIn::SceneChanged() const
{
	return m_event_watcher.RenderSceneModified();
}

void CBlackHole_RealTimeRenderPlugIn::SetSceneChanged(BOOL bChanged)
{
	m_event_watcher.SetRenderMeshFlags(bChanged);
	m_event_watcher.SetMaterialFlags(bChanged);
}

BOOL CBlackHole_RealTimeRenderPlugIn::LightingChanged() const
{
	return m_event_watcher.RenderLightingModified();
}

void CBlackHole_RealTimeRenderPlugIn::SetLightingChanged(BOOL bChanged)
{
	m_event_watcher.SetLightFlags(bChanged);
}
