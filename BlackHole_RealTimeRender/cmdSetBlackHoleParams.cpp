#include "stdafx.h"
#include "cmdSetBlackHoleParams.h"
#include "BlackHole_RealTimeRenderPlugIn.h"
#include "CBlackHole_RealTimeRenderer.h"

// 实例化我们自己的命令，向 Rhino 引擎注册
static class CCommandSetBlackHoleParams theSetBlackHoleParamsCommand;

// BH命名域，实现命令模式
namespace BH_Render {

    void UpdateMassCommand::execute() {
        if (m_renderer) {
            // 调用渲染器接口修改质量
            m_renderer->UpdatePhysicsParams(m_newMass, -1.0);
            RhinoApp().Print(L"已将黑洞质量更新为: %.4f\n", m_newMass);
        }
    }

    void UpdateSpinCommand::execute() {
        if (m_renderer) {
            // 调用渲染器接口修改自旋比例
            m_renderer->UpdatePhysicsParams(-1.0, m_newSpin);
            RhinoApp().Print(L"已将黑洞自旋比例参数 a 更新为: %.4f\n", m_newSpin);
        }
    }

} 
// namespace BH_Render


// --- Rhino 命令类实现 ---

CRhinoCommand::result CCommandSetBlackHoleParams::RunCommand(const CRhinoCommandContext& context)
{
    // 获取插件中的渲染器实例
    CBlackHole_RealTimeRenderer* pRenderer = BlackHole_RealTimeRenderPlugIn().GetRenderer();

    if (!pRenderer) {
        RhinoApp().Print(L"错误：渲染器未启动或无法获取实例。\n");
        return CRhinoCommand::failure;
    }

    // 暂存用户的输入数据，初始化为-1.0 
    double finalMass = -1.0;
    double finalSpin = -1.0;

    // ==========================================
    // 1. 串行询问质量

    CRhinoGetNumber gnMass;
    gnMass.SetCommandPrompt(L"输入新的黑洞质量 (按 Enter 保持原样跳过)");
    gnMass.SetDefaultNumber(1.0);
    gnMass.AcceptNothing(true);

    CRhinoGet::result resMass = gnMass.GetNumber();
    if (resMass == CRhinoGet::cancel) {
        return CRhinoCommand::cancel; // 用户按了 Esc，直接撤销整个命令
    }
    if (resMass == CRhinoGet::number) {
        finalMass = gnMass.Number();  // 记录下输入的质量，但先不执行
    }

    // ==========================================
    // 2. 串行询问自旋

    CRhinoGetNumber gnSpin;
    gnSpin.SetCommandPrompt(L"输入新的自旋比例参数 a (按 Enter 保持原样跳过)");
    gnSpin.SetDefaultNumber(0.90); 
    gnSpin.SetLowerLimit(0.0, TRUE);
    gnSpin.SetUpperLimit(0.99, TRUE);
    gnSpin.AcceptNothing(true);

    CRhinoGet::result resSpin = gnSpin.GetNumber();
    if (resSpin == CRhinoGet::cancel) {
        return CRhinoCommand::cancel;
    }
    if (resSpin == CRhinoGet::number) {
        finalSpin = gnSpin.Number();  // 记录下输入的自旋
    }

    // ==========================================
    // 3. 收集完毕，统一应用参数

    BH_Render::CommandManager invoker;

    // 检查暂存的变量，只有用户确实输入了新数值，才构建并压入对应的命令
    if (finalMass >= 0.0) {
        invoker.invoke(std::make_unique<BH_Render::UpdateMassCommand>(pRenderer, finalMass));
    }

    if (finalSpin >= 0.0) {
        invoker.invoke(std::make_unique<BH_Render::UpdateSpinCommand>(pRenderer, finalSpin));
    }

    return CRhinoCommand::success;
}