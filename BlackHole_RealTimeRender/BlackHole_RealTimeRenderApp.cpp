// BlackHole_RealTimeRender.cpp : 定义该 DLL 的初始化例程。
//

#include "stdafx.h"
#include "BlackHole_RealTimeRenderApp.h"

//
// 注意！
//
//    Rhino 插件是一个 MFC DLL。
//
//    如果这个 DLL 是动态链接到 MFC DLL 的，
//    那么任何从该 DLL 导出的函数，只要调用了 MFC，
//    都必须在函数最开始添加 AFX_MANAGE_STATE 宏。
//
//    例如：
//
//    extern "C" BOOL PASCAL EXPORT ExportedFunction()
//    {
//        AFX_MANAGE_STATE(AfxGetStaticModuleState());
//        // 正常的函数体
//    }
//
//    非常重要：这个宏必须出现在每一个相关函数中，
//    并且必须在任何 MFC 调用之前。
//    这意味着它必须是函数体中的第一条语句，
//    甚至要在任何对象变量声明之前，
//    因为对象构造函数可能会调用 MFC DLL。
//
//    更多细节请参考 MFC 技术文档 33 和 58。
//

// CBlackHole_RealTimeRenderApp

BEGIN_MESSAGE_MAP(CBlackHole_RealTimeRenderApp, CWinApp)
END_MESSAGE_MAP()

// 唯一的 CBlackHole_RealTimeRenderApp 对象
CBlackHole_RealTimeRenderApp theApp;

// CBlackHole_RealTimeRenderApp 初始化

BOOL CBlackHole_RealTimeRenderApp::InitInstance()
{
	// 关键：不要在这里调用 Rhino SDK 函数！
	// 这里只能进行标准的 MFC DLL 实例初始化。
	// 所有其他重要初始化逻辑应放在
	// CBlackHole_RealTimeRenderPlugIn::OnLoadPlugIn() 中。

	CWinApp::InitInstance();

	return TRUE;
}

int CBlackHole_RealTimeRenderApp::ExitInstance()
{
	// 关键：不要在这里调用 Rhino SDK 函数！
	// 这里只能进行标准的 MFC DLL 清理工作。
	// 所有其他重要的清理逻辑应放在
	// CBlackHole_RealTimeRenderPlugIn::OnSaveAllSettings() 或
	// CBlackHole_RealTimeRenderPlugIn::OnUnloadPlugIn() 中。
	return CWinApp::ExitInstance();
}