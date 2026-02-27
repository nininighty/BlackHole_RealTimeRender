#include "StdAfx.h"
#include "SampleRealtimeRendererPlugIn.h"

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//
// BEGIN SampleRealtimeRenderer command
// 开始定义 SampleRealtimeRenderer 命令
//

#pragma region SampleRealtimeRenderer command

// 定义一个继承自 CRhinoCommand 的命令类
class CCommandSampleRealtimeRenderer : public CRhinoCommand
{
public:
	CCommandSampleRealtimeRenderer() = default;
	~CCommandSampleRealtimeRenderer() = default;

	// 返回该命令的唯一 UUID（全局唯一标识）
	UUID CommandUUID() override
	{
		// {2597832E-2F14-4F39-8607-6D03DEDD8D33}
		static const GUID id =
		{ 0x2597832e, 0x2f14, 0x4f39, { 0x86, 0x7, 0x6d, 0x3, 0xde, 0xdd, 0x8d, 0x33 } };
		return id;
	}

	// 命令在 Rhino 中输入时使用的英文名称
	const wchar_t* EnglishCommandName() override { return L"SampleRealtimeRenderer"; }

	// 命令执行函数
	CRhinoCommand::result RunCommand(const CRhinoCommandContext& context) override;
};

// 唯一的 CCommandSampleRealtimeRenderer 全局对象
// Rhino 通过这个静态对象注册并管理命令
static class CCommandSampleRealtimeRenderer theSampleRealtimeRendererCommand;

// 命令的具体执行逻辑
CRhinoCommand::result CCommandSampleRealtimeRenderer::RunCommand(const CRhinoCommandContext& context)
{
	// 避免未使用参数的编译警告
	UNREFERENCED_PARAMETER(context);

	// 在 Rhino 命令行中打印插件已加载的信息
	RhinoApp().Print(L"%ls plug-in loaded.\n", SampleRealtimeRendererPlugIn().PlugInName());

	return CRhinoCommand::success;
}

#pragma endregion

//
// END SampleRealtimeRenderer command
// 结束 SampleRealtimeRenderer 命令定义
//
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////