// BlackHole_RealTimeRenderMaterial.h
//

#pragma once

// CBlackHole_RealTimeRenderMaterialFactory
// See BlackHole_RealTimeRenderMaterial.cpp for the implementation of this class.
//

class CBlackHole_RealTimeRenderMaterialFactory : public CRhRdkMaterialFactory
{
public:
	CBlackHole_RealTimeRenderMaterialFactory() = default;

protected:
	virtual CRhRdkMaterial* NewMaterial(void) const override;
};

// CBlackHole_RealTimeRenderMaterial
// See BlackHole_RealTimeRenderMaterial.cpp for the implementation of this class.
//

class CBlackHole_RealTimeRenderMaterial : public CRhRdkMaterial
{
public:
	CBlackHole_RealTimeRenderMaterial() = default;

	// CRhRdkContent overrides.
	virtual UUID TypeId(void) const override;
	virtual ON_wString TypeName(void) const override;
	virtual ON_wString TypeDescription(void) const override;
	virtual ON_wString InternalName(void) const override;
	virtual void AddUISections(IRhRdkExpandableContentUI& ui) override;
	virtual UUID RenderEngineId(void) const override;
	virtual UUID PlugInId(void) const override;
	virtual void* GetShader(const UUID& uuidRenderEngine, void* pvData) const override;
	virtual bool IsFactoryProductAcceptableAsChild(const CRhRdkContentFactory& factory, const wchar_t* wszChildSlotName) const override;
	virtual unsigned int BitFlags(void) const override;

	// CRhRdkMaterial overrides.
	virtual void SimulateMaterial(ON_Material& matOut, CRhRdkTexture::TextureGeneration tg = CRhRdkTexture::TextureGeneration::Allow,
	                              int iSimulatedTextureSize = -1, const CRhinoObject* pObject = nullptr) const override;
};
