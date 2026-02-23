// BlackHole_RealTimeRenderMaterial.cpp
//

#include "stdafx.h"
#include "BlackHole_RealTimeRenderMaterial.h"
#include "BlackHole_RealTimeRenderMaterialSection.h"
#include "BlackHole_RealTimeRenderPlugIn.h"

CRhRdkMaterial* CBlackHole_RealTimeRenderMaterialFactory::NewMaterial() const
{
	return new CBlackHole_RealTimeRenderMaterial();
}

UUID CBlackHole_RealTimeRenderMaterial::RenderEngineId() const
{
	return ::BlackHole_RealTimeRenderPlugIn().PlugInID();
}

UUID CBlackHole_RealTimeRenderMaterial::PlugInId() const
{
	return ::BlackHole_RealTimeRenderPlugIn().PlugInID();
}

UUID CBlackHole_RealTimeRenderMaterial::TypeId() const
{
	// {A33FA512-66C6-4EA8-B045-0CF0DDE59D82}
	static const GUID BlackHole_RealTimeRenderMaterial_UUID = 
	{0xf31c1e52,0x8156,0x44db,{0xb9,0x12,0xf,0x86,0xb8,0x76,0x6e,0x78}};
	return BlackHole_RealTimeRenderMaterial_UUID;
}

ON_wString CBlackHole_RealTimeRenderMaterial::TypeName() const
{
	// TODO: Return the user-friendly type name of the material.
	return L"BlackHole_RealTimeRender Material";
}

ON_wString CBlackHole_RealTimeRenderMaterial::TypeDescription() const
{
	// TODO: Return a description of the material.
	return "Demo material for BlackHole_RealTimeRender";
}

ON_wString CBlackHole_RealTimeRenderMaterial::InternalName() const
{
	// TODO: Return the material's internal name (not usually seen by the user).
	return L"your-unique-material-name-goes-here";
}

void CBlackHole_RealTimeRenderMaterial::SimulateMaterial(ON_Material& matOut, CRhRdkTexture::TextureGeneration tg, int iSimulatedTextureSize, const CRhinoObject* pObject) const
{
	// TODO: Set up the ON_Material based on your material state so that it looks as much
	//       as possible like your material will look when it is rendered.
	UNREFERENCED_PARAMETER(matOut);
	UNREFERENCED_PARAMETER(tg);
	UNREFERENCED_PARAMETER(iSimulatedTextureSize);
	UNREFERENCED_PARAMETER(pObject);
}

void CBlackHole_RealTimeRenderMaterial::AddUISections(IRhRdkExpandableContentUI& ui)
{
#if defined (RHINO_SDK_MFC)
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	ui.AddSection(new CPlugIn1MaterialSection);
#endif

	AddAutomaticUISection(ui, L"Parameters", L"Parameters");

	// TODO: Add any further UI sections.

	__super::AddUISections(ui);
}

void* CBlackHole_RealTimeRenderMaterial::GetShader(const UUID& uuidRenderEngine, void* pvData) const
{
	UNREFERENCED_PARAMETER(pvData);

	if (!IsCompatible(uuidRenderEngine))
		return nullptr;

	void* pShader = nullptr;

	// TODO: Get a pointer to the shader used to render this material.

	return pShader;
}

bool CBlackHole_RealTimeRenderMaterial::IsFactoryProductAcceptableAsChild(const CRhRdkContentFactory& factory, const wchar_t* wszChildSlotName) const
{
	// TODO: Determine if pFactory produces content suitable as a child for a particular child slot.
	//       If so, return true, otherwise return false.
	UNREFERENCED_PARAMETER(factory);
	UNREFERENCED_PARAMETER(wszChildSlotName);

	return false;
}

unsigned int CBlackHole_RealTimeRenderMaterial::BitFlags() const
{
	auto flags = __super::BitFlags();

	flags &= ~bfTextureSummary; // No texture summary required.

	// TODO: Modify flags to customize how RDK uses your material.

	return flags;
}
