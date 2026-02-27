// stdafx.h : 用于包含标准系统头文件，
// 或项目中经常使用但很少改动的头文件

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN  // 从 Windows 头文件中排除不常用内容，加快编译
#endif

// 该插件支持 Rhino 6
#define RHINO_V6_READY

// 插件必须使用 Rhino 所使用的 MFC Release 版本。
// 如果需要调试信息，应定义 RHINO_DEBUG_PLUGIN。
#if defined(RHINO_DEBUG_PLUGIN) && defined(_DEBUG)
// Rhino 6 调试插件应定义 RHINO_DEBUG_PLUGIN，
// 但不要在 .vcxproj 文件中定义 _DEBUG。
#error Do not define _DEBUG - use RHINO_DEBUG_PLUGIN instead
#endif

// Rhino SDK 预处理头
#include "RhinoSdkStdafxPreamble.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS  // 某些 CString 构造函数设为 explicit

#include <afxwin.h>   // MFC 核心和标准组件
#include <afxext.h>   // MFC 扩展

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>   // MFC OLE 类
#include <afxodlgs.h> // MFC OLE 对话框类
#include <afxdisp.h>  // MFC 自动化类
#endif

#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>    // MFC ODBC 数据库类
#endif

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>   // MFC DAO 数据库类
#endif

#include <afxdtctl.h> // MFC 对 IE4 公共控件的支持
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>   // MFC 对 Windows 公共控件的支持
#endif

// 可在此处添加常用头文件
#include <afxmt.h>    // MFC 多线程扩展（同步对象）

#if defined(_M_X64) && defined(WIN32) && defined(WIN64)
// afxwin.h -> afx.h -> afxver_.h 会无条件定义 WIN32，这是个问题。
// 所有 Windows 构建（32/64位）都会定义 _WIN32。
// 只有 64 位构建会定义 _WIN64。
// 不要手动定义/取消定义 _WIN32 或 _WIN64。
// 只能定义 WIN32 或 WIN64 其中之一。
// 详见 MSDN “Predefined Macros” 文档。
#undef WIN32
#endif

// Rhino SDK 类
#include "RhinoSdk.h"

// Rhino Render Development Kit (RDK) 类
#include "RhRdkHeaders.h"

#if defined(RHINO_DEBUG_PLUGIN)
// 系统头文件读取完成后再定义 _DEBUG，
// 方便开发者在代码中使用 _DEBUG 判断
#define _DEBUG
#endif

// Rhino SDK 链接指令
#include "rhinoSdkPlugInLinkingPragmas.h"