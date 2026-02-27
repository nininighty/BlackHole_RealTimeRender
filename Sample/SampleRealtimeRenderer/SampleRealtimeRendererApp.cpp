// SampleRealtimeRenderer.cpp : 定义该 DLL 的初始化相关函数
//

#include "stdafx.h"
#include "SampleRealtimeRendererApp.h"

//
//  注意！
//
//    一个 Rhino 插件本质上是一个 MFC DLL。
//
//      如果该 DLL 是动态链接到 MFC 的
//      DLL，那么从该 DLL 导出的任何函数，
//      只要调用了 MFC，都必须在函数最开头
//      添加 AFX_MANAGE_STATE 宏。
//
//      例如：
//
//      extern "C" BOOL PASCAL EXPORT ExportedFunction()
//      {
//          AFX_MANAGE_STATE(AfxGetStaticModuleState());
//          // 这里是正常的函数体
//      }
//
//      这个宏必须出现在每个函数中，
//      并且要在任何调用 MFC 之前。
//      也就是说，它必须是函数体中的第一条语句，
//      甚至要出现在任何对象变量声明之前，
//      因为对象的构造函数可能会调用 MFC DLL。
//
//      更多细节请参考 MFC 技术文档 33 和 58。
//

// CSampleRealtimeRendererApp

BEGIN_MESSAGE_MAP(CSampleRealtimeRendererApp, CWinApp)
END_MESSAGE_MAP()

// CSampleRealtimeRendererApp 构造函数

CSampleRealtimeRendererApp::CSampleRealtimeRendererApp()
{
    // 关键：不要在这里调用 Rhino SDK 的函数！
    //   这里只能进行标准的 MFC DLL 实例构造。
    //   所有其他重要的初始化操作应当放在
    //   CSampleRealtimeRendererPlugIn::OnLoadPlugIn() 中完成。
}

// 唯一的 CSampleRealtimeRendererApp 全局对象
CSampleRealtimeRendererApp theApp;

// CSampleRealtimeRendererApp 初始化

BOOL CSampleRealtimeRendererApp::InitInstance()
{
    // 关键：不要在这里调用 Rhino SDK 的函数！
    //   这里只能进行标准的 MFC DLL 实例初始化。
    //   所有其他重要的初始化操作应当放在
    //   CSampleRealtimeRendererPlugIn::OnLoadPlugIn() 中完成。

    CWinApp::InitInstance();

    return TRUE;
}

int CSampleRealtimeRendererApp::ExitInstance()
{
    // 关键：不要在这里调用 Rhino SDK 的函数！
    //   这里只能进行标准的 MFC DLL 清理操作。
    //   所有其他重要的清理工作应当放在
    //   CSampleRealtimeRendererPlugIn::OnSaveAllSettings() 或
    //   CSampleRealtimeRendererPlugIn::OnUnloadPlugIn() 中完成。
    return CWinApp::ExitInstance();
}