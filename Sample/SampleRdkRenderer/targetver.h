#pragma once

// 包含 SDKDDKVer.h 会定义当前可用的最高版本 Windows 平台。
// 如果您希望为之前的 Windows 平台构建应用程序，请在包含 SDKDDKVer.h 之前，
// 先包含 WinSDKVer.h 并将 _WIN32_WINNT 宏设置为您希望支持的平台。

#include "rhinoSdkWindowsVersion.h"

#include <SDKDDKVer.h>

/** 【Gemini 注解】：
    虽然代码只有寥寥几行，但它在工程架构中扮演着“版本过滤器”的角色。

    1. rhinoSdkWindowsVersion.h：这是 McNeel 官方提供的头文件，它会自动帮你
       把宏设定为 Rhino 8 所需的最低 Windows 环境（通常是 Windows 10/11）。

    2. SDKDDKVer.h：这是微软 Windows SDK 的标准头文件。它会根据宏定义
       （如 _WIN32_WINNT）来开启或关闭系统 API 的可见性。
*/