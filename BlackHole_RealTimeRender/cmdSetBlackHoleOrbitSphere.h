// cmdSetBlackHoleOrbitSphere.h
#pragma once

#include "stdafx.h" 
#include "CBlackHole_Common.h"
#include "cmdSetBlackHoleParams.h" // 引入之前的 BH_Render::Command 和 CommandManager
#include <memory>

class CBlackHole_RealTimeRenderer;

// Rhino 指令类
class CCommandSetBlackHoleOrbitSphere : public CRhinoCommand
{
public:
    CCommandSetBlackHoleOrbitSphere() = default;
    ~CCommandSetBlackHoleOrbitSphere() = default;

    virtual UUID CommandUUID() override
    {
        static const GUID uuid =
        { 0x7b0c3d4e, 0x5f31, 0x7b8c, { 0x4d, 0x0e, 0x8f, 0x2a, 0x3b, 0x4c, 0x5d, 0x6e } };
        return uuid;
    }

    virtual const wchar_t* EnglishCommandName() override { return L"SetBlackHoleOrbitSphere"; }
    virtual CRhinoCommand::result RunCommand(const CRhinoCommandContext& context) override;
};

// 继续使用 BH_Render 命名域，保持设计模式一致
namespace BH_Render {

    // 绕行天体更新命令类
    class UpdateOrbitSphereCommand : public Command {
    private:
        CBlackHole_RealTimeRenderer* m_renderer;
        OrbitSphereParameters m_params;
    public:
        UpdateOrbitSphereCommand(CBlackHole_RealTimeRenderer* renderer, const OrbitSphereParameters& params)
            : m_renderer(renderer), m_params(params) {
        }
        void execute() override;
    };

}