// SampleRealtimeRendererPlugIn.cpp : 定义插件的初始化相关函数
//

#include "StdAfx.h"
#include "rhinoSdkPlugInDeclare.h"
#include "SampleRealtimeRendererPlugIn.h"
#include "SampleDisplayMode.h"
#include "Resource.h"

// RhRdk::Realtime::DisplayMode::Factory 缺少虚析构函数，
// 若修复将破坏 RDK SDK 的兼容性，因此关闭相关警告。
#pragma warning(disable : 5205) // 删除一个没有虚析构函数的抽象类会导致未定义行为

// 插件对象必须在任何继承自 CRhinoCommand 的插件类之前构造。
// #pragma init_seg(lib) 保证这一点。
#pragma warning( push )
#pragma warning( disable : 4073 )
#pragma init_seg( lib )
#pragma warning( pop )

// Rhino 插件声明
RHINO_PLUG_IN_DECLARE

// Rhino 插件名称
// 为该插件提供一个简短、友好的名称。
RHINO_PLUG_IN_NAME(L"SampleRealtimeRenderer");

// Rhino 插件 ID
// 为该插件提供一个唯一的 UUID。
RHINO_PLUG_IN_ID(L"063A34A5-4D31-401D-B0CD-692FDB089B06");

// Rhino 插件版本
// 为该插件提供一个版本号字符串。
RHINO_PLUG_IN_VERSION(__DATE__ "  " __TIME__)

// Rhino 插件描述
// 为该插件提供描述信息。
RHINO_PLUG_IN_DESCRIPTION(L"Rhino SDK Sample - SampleRealtimeRenderer");

// Rhino 插件图标资源 ID
// 指定插件图标的资源 ID。
RHINO_PLUG_IN_ICON_RESOURCE_ID(IDI_ICON1);

// Rhino 插件开发者信息声明
RHINO_PLUG_IN_DEVELOPER_ORGANIZATION(L"Robert McNeel & Associates");
RHINO_PLUG_IN_DEVELOPER_ADDRESS(L"3670 Woodland Park Avenue North\r\nSeattle WA 98103");
RHINO_PLUG_IN_DEVELOPER_COUNTRY(L"United States");
RHINO_PLUG_IN_DEVELOPER_PHONE(L"206-545-6877");
RHINO_PLUG_IN_DEVELOPER_FAX(L"206-545-7321");
RHINO_PLUG_IN_DEVELOPER_EMAIL(L"devsupport@mcneel.com");
RHINO_PLUG_IN_DEVELOPER_WEBSITE(L"http://www.rhino3d.com");
RHINO_PLUG_IN_UPDATE_URL(L"https://github.com/mcneel/rhino-developer-samples");

// 唯一的 CSampleRealtimeRendererPlugIn 全局对象
static CSampleRealtimeRendererPlugIn thePlugIn;

/////////////////////////////////////////////////////////////////////////////
// CSampleRealtimeRendererPlugIn 定义

CSampleRealtimeRendererPlugIn& SampleRealtimeRendererPlugIn()
{
	// 返回唯一的 CSampleRealtimeRendererPlugIn 对象引用
	return thePlugIn;
}

CSampleRealtimeRendererPlugIn::CSampleRealtimeRendererPlugIn()
{
	// 说明：
	//   构造函数在插件加载时被调用，同时构造 "thePlugIn" 对象。
	//   插件加载完成后会调用 CSampleRealtimeRendererPlugIn::OnLoadPlugIn()。
	//   构造函数应保持简单、稳定。
	//   任何可能失败的操作应放在 OnLoadPlugIn() 中执行。

	// TODO: 在此添加构造代码
	m_plugin_version = RhinoPlugInVersion();
}

CSampleRealtimeRendererPlugIn::~CSampleRealtimeRendererPlugIn()
{
	// 说明：
	//   析构函数在插件卸载时被调用，用于销毁 "thePlugIn"。
	//   在 DLL 卸载之前会先调用 OnUnloadPlugIn()。
	//   不要在这里做过多操作。
	//   请确保释放所有通过 onmalloc()、onrealloc()、oncalloc()、
	//   或 onstrdup() 分配的内存。

	// TODO: 在此添加析构代码
}

/////////////////////////////////////////////////////////////////////////////
// 必须重写的函数

const wchar_t* CSampleRealtimeRendererPlugIn::PlugInName() const
{
	// 说明：
	//   插件名称显示字符串。
	//   Rhino 在加载插件、插件帮助菜单以及插件管理界面中显示此名称。

	// TODO: 返回一个简短、友好的插件名称。
	return RhinoPlugInName();
}

const wchar_t* CSampleRealtimeRendererPlugIn::PlugInVersion() const
{
	// 说明：
	//   插件版本显示字符串。
	//   Rhino 在加载插件和插件管理界面中显示此版本信息。

	// TODO: 返回插件版本号。
	return m_plugin_version;
}

GUID CSampleRealtimeRendererPlugIn::PlugInID() const
{
	// 说明：
	//   插件唯一标识符。
	//   Rhino 使用此标识符管理插件。

	// TODO: 返回插件的唯一标识符。
	// {336403E8-9B17-41C5-910F-7D951D99E607}
	return ON_UuidFromString(RhinoPlugInId());
}

BOOL CSampleRealtimeRendererPlugIn::OnLoadPlugIn()
{
	// 说明：
	//   在插件加载并执行完构造函数后调用。
	//   适合在此进行重要初始化、许可证检查等操作。
	//   必须返回 TRUE 插件才会继续加载。

	// 备注：
	//   插件在 Rhino 启动并创建默认文档后才会加载。
	//   由于默认文档已经存在，
	//   CRhinoEventWatcher::On????Document() 不会为默认文档调用。
	//   如果需要进行文档初始化或同步，应在此函数中完成。
	//   无需调用基类的 CPlugIn::OnLoadPlugIn()。

	// TODO: 在此添加插件初始化代码。


	// 注册实时显示模式工厂
	RhRdk::Realtime::DisplayMode::Factory::Register(std::make_unique<CDisplayModeFactory>());

	//// 检查显示模式是否已经添加...
	DisplayAttrsMgrListDesc* pDisplayAttrsMgrListDesc = CRhinoDisplayAttrsMgr::FindDisplayAttrsDesc(DisplayModeId());
	// ...如果没有添加，则进行添加。
	if (0 == pDisplayAttrsMgrListDesc)
	{
		pDisplayAttrsMgrListDesc = CRhinoDisplayAttrsMgr::AppendNewEntry();
		if (0 != pDisplayAttrsMgrListDesc && 0 != pDisplayAttrsMgrListDesc->m_pAttrs)
		{
			// 在下拉菜单中显示该显示模式
			pDisplayAttrsMgrListDesc->m_bAddToMenu = true;

			// 设置其 GUID、名称和渲染管线类
			pDisplayAttrsMgrListDesc->m_pAttrs->SetUuid(DisplayModeId());
			pDisplayAttrsMgrListDesc->m_pAttrs->SetName(PlugInName());
			pDisplayAttrsMgrListDesc->m_pAttrs->SetRealtimeDisplayId(DisplayModeId());
			pDisplayAttrsMgrListDesc->m_pAttrs->SetPipeline(&ON_CLASS_RTTI(CRhinoDisplayPipeline_OGL));

			// 保存修改
			CRhinoDisplayAttrsMgr::SaveProfile(RhinoApp().ProfileContext());
		}
	}

	return TRUE;
}

void CSampleRealtimeRendererPlugIn::OnUnloadPlugIn()
{
	// 说明：
	//   在插件即将卸载时调用一次。
	//   此时 Rhino 主窗口已经销毁，
	//   某些 SDK 管理器对象也已被删除。
	//   此时不存在活动文档或视图。
	//   因此这里只应操作你自己的对象或工具。

	// TODO: 在此添加插件清理代码。
}

/////////////////////////////////////////////////////////////////////////////
// 加载时机重写

CRhinoPlugIn::plugin_load_time CSampleRealtimeRendererPlugIn::PlugInLoadTime()
{
	// 该插件必须在启动时加载，以便注册渲染管线。
	return CRhinoPlugIn::load_plugin_at_startup;
}