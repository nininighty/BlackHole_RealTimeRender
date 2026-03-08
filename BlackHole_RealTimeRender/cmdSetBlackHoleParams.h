// cmdSetBlackHoleParams.h
// 用于完成Rhino中的命令调整黑洞参数
#pragma once

#include "stdafx.h" 
#include <memory>
#include <vector>

// 告知编译器我们有个Renderer类
class CBlackHole_RealTimeRenderer;

// 继承CRhinoCommand类完成我们的指令类
class CCommandSetBlackHoleParams : public CRhinoCommand
{
public:
    CCommandSetBlackHoleParams() = default;
    ~CCommandSetBlackHoleParams() = default;

    // 指令独一的UUID，6677，6677~
    virtual UUID CommandUUID() override
    {
        static const GUID uuid =
        { 0x8a9b2c3d, 0x4e5f, 0x6677, { 0xab, 0xdd, 0xaf, 0x12, 0x34, 0x56, 0x78, 0x90 } };
        return uuid;
    }

    // 指令调用字符串
    virtual const wchar_t* EnglishCommandName() override { return L"SetBlackHoleParams"; }
    // 执行指令入口
    virtual CRhinoCommand::result RunCommand(const CRhinoCommandContext& context) override;
};


// 防止命名冲突，定义BH命令域，实现命令模式
namespace BH_Render {

    class Command {
    public:
        virtual ~Command() = default;
        virtual void execute() = 0;
    };

    // 修改质量类
    class UpdateMassCommand : public Command {
    private:
        CBlackHole_RealTimeRenderer* m_renderer;
        double m_newMass;
    public:
        UpdateMassCommand(CBlackHole_RealTimeRenderer* renderer, double mass)
            : m_renderer(renderer), m_newMass(mass) {
        }
        void execute() override;
    };

    // 修改自旋类
    class UpdateSpinCommand : public Command {
    private:
        CBlackHole_RealTimeRenderer* m_renderer;
        double m_newSpin;
    public:
        UpdateSpinCommand(CBlackHole_RealTimeRenderer* renderer, double spin)
            : m_renderer(renderer), m_newSpin(spin) {
        }
        void execute() override;
    };

    // 命令管理类
    class CommandManager {
    private:
        std::vector<std::unique_ptr<Command>> m_history;
    public:
        void invoke(std::unique_ptr<Command> cmd) {
            if (cmd) {
                cmd->execute();
                m_history.push_back(std::move(cmd));
            }
        }
    };
}