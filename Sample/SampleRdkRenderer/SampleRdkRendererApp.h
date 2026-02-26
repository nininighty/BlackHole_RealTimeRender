// SampleRdkRenderer.h : SampleRdkRenderer DLL 的主头文件。
//

#pragma once

#ifndef __AFXWIN_H__
#error "在包含此文件之前请先包含 'stdafx.h' 以进行预编译头处理 (PCH)"
#endif

#include "resource.h" // 主符号定义

// CSampleRdkRendererApp
// 有关此类实现的代码，请参阅 SampleRdkRendererApp.cpp
//

class CSampleRdkRendererApp : public CWinApp
{
public:
    // 关键警告：切勿在此处调用 RHINO SDK 函数！
    // 此处仅归属于标准的 MFC DLL 实例构造逻辑。
    // 所有其他重要的初始化工作都应该在 
    // CSampleRdkRendererPlugIn::OnLoadPlugIn() 中进行。
    CSampleRdkRendererApp() = default;

    /** 【Gemini 注解】：
        这是 Windows 应用程序框架（MFC）的起点。对于你的项目，这里的“禁令”非常重要：
        如果你试图在这里初始化你的机器学习推理引擎或者复杂的物理常数，
        Rhino 可能会因为插件尚未完全通过 RDK 认证而导致启动崩溃。
    */

    // 关键警告：切勿在此处调用 RHINO SDK 函数！
    // 此处仅归属于标准的 MFC DLL 实例初始化。
    // 所有其他重要的初始化工作都应该在 
    // CSampleRdkRendererPlugIn::OnLoadPlugIn() 中进行。
    BOOL InitInstance() override;

    /** 【Gemini 注解】：
        在复试演示时，如果导师问你“如何保证插件加载的稳定性”，你可以提到：
        你严格遵守了 MFC 与 Rhino SDK 的解耦原则，将所有涉及物理计算和模型加载的
        逻辑推迟到插件真正被 Rhino 挂载（OnLoadPlugIn）之后。
    */

    // 关键警告：切勿在此处调用 RHINO SDK 函数！
    // 此处仅归属于标准的 MFC DLL 实例清理工作。
    // 所有其他重要的清理工作都应该在以下位置进行：
    // CSampleRdkRendererPlugIn::OnSaveAllSettings() 或
    // CSampleRdkRendererPlugIn::OnUnloadPlugIn()。  
    int ExitInstance() override;

    /** 【Gemini 注解】：
        当 Rhino 关闭时，这里是 DLL 卸载的最后关头。
        对于你的 ML 加速方案，如果你在后台运行了一个独立的 Python 推理进程或服务器，
        请确保不要在这里强制关闭它，以免触发 Windows 的“程序无响应”报错。
    */

    DECLARE_MESSAGE_MAP()
};