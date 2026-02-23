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
	// TODO: Initialize your plug-in. Return false on failure.

	return __super::Initialize();
}

void CBlackHole_RealTimeRenderRdkPlugIn::Uninitialize()
{
	// TODO: Do any necessary plug-in clean-up here.

	__super::Uninitialize();
}

void CBlackHole_RealTimeRenderRdkPlugIn::RegisterExtensions() const
{
	AddExtension(new CBlackHole_RealTimeRenderMaterialFactory);

	// TODO: Add further material/environment/texture factories.

	__super::RegisterExtensions();
}

void CBlackHole_RealTimeRenderRdkPlugIn::AbortRender()
{
	// TODO:
}

bool CBlackHole_RealTimeRenderRdkPlugIn::CreatePreview(const ON_2iSize& sizeImage, RhRdkPreviewQuality quality, const IRhRdkPreviewSceneServer* pSceneServer, IRhRdkPreviewCallbacks* pNotify, CRhinoDib& dibOut)
{
	// TODO: Create a rendered preview of the specified scene at the specified size and quality.
	UNREFERENCED_PARAMETER(sizeImage);
	UNREFERENCED_PARAMETER(quality);
	UNREFERENCED_PARAMETER(pSceneServer);
	UNREFERENCED_PARAMETER(pNotify);
	UNREFERENCED_PARAMETER(dibOut);

	return false;
}

bool CBlackHole_RealTimeRenderRdkPlugIn::CreatePreview(const ON_2iSize& sizeImage, const CRhRdkTexture& texture, CRhinoDib& dibOut)
{
	// TODO: Optionally create a preview of the texture.
	// Return false to allow RDK to produce all texture previews.
	UNREFERENCED_PARAMETER(sizeImage);
	UNREFERENCED_PARAMETER(texture);
	UNREFERENCED_PARAMETER(dibOut);

	return false;
}

bool CBlackHole_RealTimeRenderRdkPlugIn::SupportsFeature(const UUID& uuidFeature) const
{
	// TODO: Determine which features of the RDK are exposed while this is the current renderer.

	if (uuidFeature == uuidFeatureCustomRenderMeshes)
		return true; // This renderer supports custom render meshes (because it uses the iterator).

	if (uuidFeature == uuidFeatureDecals)
		return false; // This renderer does not support decals.

	if (uuidFeature == uuidFeatureGroundPlane)
		return false; // This renderer does not support a ground plane.

	if (uuidFeature == uuidFeatureSun)
		return false; // This renderer does not support the Sun.

	return true;
}
