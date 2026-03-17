// cmdSetBlackHoleOrbitSphere.cpp
#include "stdafx.h"
#include "cmdSetBlackHoleOrbitSphere.h"
#include "BlackHole_RealTimeRenderPlugIn.h"
#include "CBlackHole_RealTimeRenderer.h"

// 向 Rhino 引擎注册
static class CCommandSetBlackHoleOrbitSphere theSetBlackHoleOrbitSphereCommand;

namespace BH_Render {

    void UpdateOrbitSphereCommand::execute() {
        if (m_renderer) {
            // 调用渲染器接口更新天体数据
            m_renderer->UpdateOrbitSphere(m_params);
            RhinoApp().Print(L"已将绕行天体参数更新至渲染管线。\n");
        }
    }

}
// namespace BH_Render


CRhinoCommand::result CCommandSetBlackHoleOrbitSphere::RunCommand(const CRhinoCommandContext& context)
{
    // 获取插件中的渲染器实例
    CBlackHole_RealTimeRenderer* pRenderer = BlackHole_RealTimeRenderPlugIn().GetRenderer();

    if (!pRenderer) {
        RhinoApp().Print(L"错误：渲染器未启动或无法获取实例。\n");
        return CRhinoCommand::failure;
    }

    // ==========================================
    // 1. 询问并拾取球体

    CRhinoGetObject go;
    go.SetCommandPrompt(L"请选择一个球体作为绕行天体 (按 Enter 取消)");
    go.SetGeometryFilter(CRhinoGetObject::surface_object | CRhinoGetObject::polysrf_object);
    go.GetObjects(1, 1);

    if (go.CommandResult() != CRhinoCommand::success) {
        return go.CommandResult(); // 用户按了 Esc 或回车
    }

    const CRhinoObjRef& ref = go.Object(0);
    const ON_Brep* brep = ref.Brep();
    ON_Sphere parsedSphere;
    bool bIsSphere = false;

    // 【修改部分】：获取 Brep 的底层曲面来判断是否为球体
    if (brep && brep->m_S.Count() > 0) {
        const ON_Surface* srf = brep->m_S[0];
        if (srf && srf->IsSphere(&parsedSphere)) {
            bIsSphere = true;
        }
    }

    if (bIsSphere) {

        // ==========================================
        // 2. 串行询问倾角 

        CRhinoGetNumber gnInc;
        gnInc.SetCommandPrompt(L"请输入轨道倾角 (角度，例如 15.0)");
        gnInc.SetDefaultNumber(0.0);
        gnInc.AcceptNothing(true);

        CRhinoGet::result resInc = gnInc.GetNumber();
        if (resInc == CRhinoGet::cancel) {
            return CRhinoCommand::cancel;
        }

        double inclinationRad = 0.0;
        if (resInc == CRhinoGet::number) {
            inclinationRad = gnInc.Number() * ON_PI / 180.0; // 角度转弧度
        }

        // ==========================================
        // 3. 提取 Rhino 物体材质与贴图

        OrbitSphereParameters orbitData;
        orbitData.active = true;
        orbitData.initialCenter = parsedSphere.Center();
        orbitData.radius = parsedSphere.Radius();
        orbitData.orbitSpeed = 0.5;
        orbitData.inclination = inclinationRad;

        // 获取所选物件的真实指针
        const CRhinoObject* pObj = ref.Object();
        if (pObj) {
            orbitData.objectId = pObj->Id();
            // 获取物件当前显示的材质
            const ON_Material& mat = pObj->ObjectMaterial();
            orbitData.baseColor = mat.Diffuse(); // 获取漫反射基础色

            // 尝试在材质中寻找图像贴图 (bitmap_texture)
            int texIndex = mat.FindTexture(nullptr, ON_Texture::TYPE::bitmap_texture);
            if (texIndex >= 0) {
                // 如果有贴图，提取出它在硬盘上的绝对路径
                orbitData.texturePath = mat.m_textures[texIndex].m_image_file_reference.FullPath();
            }
        }

        // ==========================================
        // 4. 打包数据并通过 CommandManager 执行
        BH_Render::CommandManager invoker;
        // 严格使用 std::make_unique
        invoker.invoke(std::make_unique<BH_Render::UpdateOrbitSphereCommand>(pRenderer, orbitData));

        return CRhinoCommand::success;
    }
    else {
        RhinoApp().Print(L"所选物件无法解析为标准球体，请重试。\n");
        return CRhinoCommand::cancel;
    }
}