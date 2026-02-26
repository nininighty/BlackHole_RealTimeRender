// BlackHole_RealTimeRenderRdkPlugIn.cpp
//

#include "stdafx.h"
#include "BlackHole_RealTimeRenderRdkPlugIn.h"
#include "BlackHole_RealTimeRenderPlugIn.h"
#include "BlackHole_RealTimeRenderMaterial.h"

UUID CBlackHole_RealTimeRenderRdkPlugIn::PlugInId() const
{
	return ::BlackHole_RealTimeRenderPlugIn().PlugInID();
}

CRhinoPlugIn& CBlackHole_RealTimeRenderRdkPlugIn::RhinoPlugIn() const
{
	return ::BlackHole_RealTimeRenderPlugIn();
}

bool CBlackHole_RealTimeRenderRdkPlugIn::Initialize()
{
	// TODO：初始化你的插件。如果初始化失败，请返回 false。

	return __super::Initialize();
}

void CBlackHole_RealTimeRenderRdkPlugIn::Uninitialize()
{
	// TODO：在这里执行插件需要的清理工作。

	__super::Uninitialize();
}

void CBlackHole_RealTimeRenderRdkPlugIn::RegisterExtensions() const
{
	AddExtension(new CBlackHole_RealTimeRenderMaterialFactory);

	// TODO：在这里继续添加材质 / 环境 / 纹理 工厂。

	__super::RegisterExtensions();
}

void CBlackHole_RealTimeRenderRdkPlugIn::AbortRender()
{
	// TODO：在这里实现中止渲染的逻辑。
}

bool CBlackHole_RealTimeRenderRdkPlugIn::CreatePreview(const ON_2iSize& sizeImage, RhRdkPreviewQuality quality, const IRhRdkPreviewSceneServer* pSceneServer, IRhRdkPreviewCallbacks* pNotify, CRhinoDib& dibOut)
{
	// TODO：以指定尺寸和质量，为当前场景生成一个渲染预览图。
	UNREFERENCED_PARAMETER(sizeImage);
	UNREFERENCED_PARAMETER(quality);
	UNREFERENCED_PARAMETER(pSceneServer);
	UNREFERENCED_PARAMETER(pNotify);
	UNREFERENCED_PARAMETER(dibOut);

	return false;
}

bool CBlackHole_RealTimeRenderRdkPlugIn::CreatePreview(const ON_2iSize& sizeImage, const CRhRdkTexture& texture, CRhinoDib& dibOut)
{
	// TODO：可选地为纹理生成预览图。
	// 如果返回 false，则由 RDK 自动生成所有纹理预览。
	UNREFERENCED_PARAMETER(sizeImage);
	UNREFERENCED_PARAMETER(texture);
	UNREFERENCED_PARAMETER(dibOut);

	return false;
}

bool CBlackHole_RealTimeRenderRdkPlugIn::SupportsFeature(const UUID& uuidFeature) const
{
	// TODO：决定在当前渲染器下，RDK 的哪些功能是可用的。

	if (uuidFeature == uuidFeatureCustomRenderMeshes)
		return true; // 该渲染器支持自定义渲染网格（因为使用了迭代器）。

	if (uuidFeature == uuidFeatureDecals)
		return false; // 该渲染器不支持贴花（Decal）。

	if (uuidFeature == uuidFeatureGroundPlane)
		return false; // 该渲染器不支持地面平面。

	if (uuidFeature == uuidFeatureSun)
		return false; // 该渲染器不支持太阳光源。

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// 实时渲染支持（手动添加）

//RenderCapabilities : 向 Rhino 软件“递交名片”，声明你的插件具备哪些高级渲染能力。
unsigned int CBlackHole_RealTimeRenderRdkPlugIn::RenderCapabilities() const {
	// 只要 stdafx.h 里有 RhRdkHeaders.h，下面这些常量就是全局可见的
	return (unsigned int)(
		IRhRdkRenderPlugIn::CanDoRealtimeRender |
		IRhRdkRenderPlugIn::CanDoCustomDisplayModes
		);
}

GUID CBlackHole_RealTimeRenderRdkPlugIn::RenderEngineId() const{
	// 确保调用主插件的 ID
	return ::BlackHole_RealTimeRenderPlugIn().PlugInID();
}