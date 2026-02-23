// BlackHole_RealTimeRenderRdkPlugIn.h
//

#pragma once

// CBlackHole_RealTimeRenderRdkRenderPlugIn
// 该类的实现请参见 BlackHole_RealTimeRenderRdkPlugIn.cpp
//

class CBlackHole_RealTimeRenderRdkPlugIn : public CRhRdkRenderPlugIn
{
public:
	// ==========================  【我的代码】  =============================

	virtual unsigned int RenderCapabilities() const;
	virtual GUID RenderEngineId() const;

	// ==========================  【我的代码】  =============================

	CBlackHole_RealTimeRenderRdkPlugIn() = default;

	virtual bool Initialize() override;
	virtual void Uninitialize() override;

	virtual UUID PlugInId() const override;

	virtual void AbortRender() override;

	CRhinoPlugIn& RhinoPlugIn() const override;


protected:
	virtual void RegisterExtensions() const override;

	// 预览渲染器
	virtual bool CreatePreview(const ON_2iSize& sizeImage, RhRdkPreviewQuality quality, const IRhRdkPreviewSceneServer* pSceneServer, IRhRdkPreviewCallbacks* pNotify, CRhinoDib& dibOut) override;
	virtual bool CreatePreview(const ON_2iSize& sizeImage, const CRhRdkTexture& texture, CRhinoDib& dibOut) override;

	virtual bool SupportsFeature(const UUID& uuidFeature) const override;
};