// stdafx.h : 标准系统包含文件或项目中经常使用但不经常更改的包含文件的包含文件。
//

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN                // 从 Windows 头文件中排除极少使用的内容
#endif

// 此插件已适配 Rhino 6（向后兼容性标识）
#define RHINO_V6_READY

// 如果你想使用 Rhino 的 MFC UI 类，请取消下面 #define RHINO_SDK_MFC 语句的注释。
// 请注意，这样做需要使用与构建 Rhino 时相同的 Visual Studio 版本来构建你的插件。
#define RHINO_SDK_MFC

/** 【Gemini 注解】：
    你现在的代码中启用了这个宏。这意味着你可以使用 Rhino 原生的对话框和界面控件。
    在黑洞项目中，如果你打算做一个带有滑动条（如调节黑洞质量 M）的控制面板，
    这个宏是必不可少的。
*/

// 插件必须使用 Rhino 使用的发行版 (Release) MFC。
// 需要调试信息的插件需要定义 RHINO_DEBUG_PLUGIN 进行构建。
#if defined(RHINO_DEBUG_PLUGIN) && defined(_DEBUG)
//  Rhino 6 调试插件应定义 RHINO_DEBUG_PLUGIN，但不应在 .vcxproj 文件中定义 _DEBUG。
#error 不要定义 _DEBUG - 请改用 RHINO_DEBUG_PLUGIN
#endif

/** 【Gemini 注解】：
    这是 Rhino 插件开发中最大的“新手坑”。
    Rhino 插件即使在调试模式下也不允许定义标准的 _DEBUG，
    否则会导致内存分配器冲突（Heap Corrupt）。必须统一使用 RHINO_DEBUG_PLUGIN。
*/

// Rhino SDK 序言
#include "RhinoSdkStdafxPreamble.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS       // 某些 CString 构造函数将是显式的

#include <afxwin.h>                              // MFC 核心和标准组件
#include <afxext.h>                              // MFC 扩展

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>                              // MFC OLE 类
#include <afxodlgs.h>                            // MFC OLE 对话框类
#include <afxdisp.h>                             // MFC 自动化类
#endif // _AFX_NO_OLE_SUPPORT

#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>                               // MFC ODBC 数据库类
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>                              // MFC DAO 数据库类
#endif // _AFX_NO_DAO_SUPPORT

#include <afxdtctl.h>                            // MFC 对 Internet Explorer 4 公共控件的支持
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                              // MFC 对 Windows 公共控件的支持
#endif // _AFX_NO_AFXCMN_SUPPORT

// TODO: 在此处包含其他常用的头文件

#if defined(_M_X64) && defined(WIN32) && defined(WIN64)
//  afxwin.h 包含 afx.h，后者包含 afxver_.h，这会无条件地定义 WIN32。这是一个错误。
//  请注意，所有 Windows 构建（32 位和 64 位）都会定义 _WIN32。
//  只有 64 位构建会定义 _WIN64。永远不要定义/取消定义 _WIN32 或 _WIN64。
//  仅确切定义 WIN32 或 WIN64 之一。
//  详情请参阅 MSDN “预定义宏”帮助文件。
#undef WIN32
#endif

// Rhino SDK 类
#include "RhinoSdk.h" 

// Rhino 渲染开发工具包 (RDK) 类
#include "RhRdkHeaders.h" 

/** 【Gemini 注解】：
    这是最关键的一行！
    它包含了你在之前报错中苦苦寻找的所有 RDK 类定义（包括 IRhRdkRenderPlugIn）。
    只要这个文件在 stdafx.h 里，你的其他 .cpp 文件只要顶部有 #include "stdafx.h"，
    就绝对不应该再手动包含 RhRdkRenderPlugIn.h 这种零碎文件。
*/

#if defined(RHINO_DEBUG_PLUGIN)
// 现在所有系统头文件都已读取，我们可以安全地定义 _DEBUG，
// 以便开发人员可以在其代码中测试 _DEBUG。
#define _DEBUG
#endif

// Rhino SDK 链接指令（Pragmas）
#include "rhinoSdkPlugInLinkingPragmas.h"