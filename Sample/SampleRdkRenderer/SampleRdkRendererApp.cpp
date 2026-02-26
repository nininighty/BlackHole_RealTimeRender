/** 此示例演示如何创建一个异步 RDK 渲染器插件。

	异步渲染在后台运行，用户可以在渲染进行时继续使用 Rhino。
	如果需要，可以同时运行多个渲染任务。

	【Gemini 注解】：
	这正是你“黑洞项目”的核心基石。异步（Asynchronous）意味着你的 4 阶龙格-库塔积分
	或者 Python ML 模型推理是在后台悄悄进行的。这样用户在旋转视角观察黑洞时，
	Rhino 的界面才不会卡成幻灯片。
*/

#include "stdafx.h"
#include "SampleRdkRendererApp.h"

//	注意！
//
//    Rhino 插件本质上是一个 MFC DLL（动态链接库）。
//
//		如果此 DLL 是与 MFC DLL 动态链接的，那么从此 DLL 导出的、
//		任何调用了 MFC 内部功能的函数，都必须在函数的最开头
//		添加 AFX_MANAGE_STATE 宏。
//
//		示例：
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// 这里是正常的函数体
//		}
//
//		非常重要的一点是：这个宏必须出现在每个函数中，且位于任何调用 MFC 
//		的操作之前。这意味着它必须作为函数内的第一条语句出现，甚至要放在
//		任何对象变量声明之前，因为这些对象的构造函数可能会调用 MFC DLL。
//
//		更多详情请参阅 MFC 技术笔记 33 和 58。
//
//    【Gemini 注解】：
//    这是开发 Rhino C++ 插件最容易踩的“底层坑”。如果你以后发现插件在弹出对话框、
//    或者处理某些界面点击时莫名其妙崩溃，多半是忘了检查这个宏。
//

// CSampleRdkRendererApp 消息映射

BEGIN_MESSAGE_MAP(CSampleRdkRendererApp, CWinApp)
END_MESSAGE_MAP()

// 唯一的 CSampleRdkRendererApp 对象（单例模式）
CSampleRdkRendererApp theApp;

// CSampleRdkRendererApp 初始化

BOOL CSampleRdkRendererApp::InitInstance()
{
	// 关键提示：请勿在此处调用 RHINO SDK 的函数！
	// 此处仅归属于标准的 MFC DLL 实例初始化。
	// 所有其他重要的初始化工作（如注册渲染器、初始化黑洞参数）
	// 都应该在 CSampleRdkRendererPlugIn::OnLoadPlugIn() 中进行。

	// 【Gemini 注解】：
	// 此时 DLL 刚被 Windows 加载，Rhino 的核心环境还没准备好“接纳”你的插件。
	// 如果你急着在这里初始化黑洞引擎，会导致 Rhino 启动时直接闪退。

	CWinApp::InitInstance();

	return TRUE;
}

int CSampleRdkRendererApp::ExitInstance()
{
	// 关键提示：请勿在此处调用 RHINO SDK 的函数！
	// 此处仅归属于标准的 MFC DLL 实例清理工作。
	// 所有其他重要的清理工作都应该在以下位置进行：
	// CSampleRdkRendererPlugIn::OnSaveAllSettings() 或
	// CSampleRdkRendererPlugIn::OnUnloadPlugIn()。

	// 【Gemini 注解】：
	// 同样的逻辑，当插件退出时，Rhino 的 SDK 环境可能已经提前释放了。
	// 在这里强行去断开与 Python 的进程间通信（IPC）可能会抛出非法访问异常。

	return CWinApp::ExitInstance();
}