// BlackHole_RealTimeRenderMaterialSection.h
//

#pragma once

#include "BlackHole_RealTimeRenderMaterial.h"
#include "Resource.h"

// CBlackHole_RealTimeRenderMaterialSection
// See BlackHole_RealTimeRenderMaterialSection.cpp for the implementation of this class.
//

#if defined (RHINO_SDK_MFC)
class CBlackHole_RealTimeRenderMaterialSection : public CRhRdkMaterialUISection_MFC
{
public:
	CBlackHole_RealTimeRenderMaterialSection();

private:
	enum { IDD = IDD_MATERIAL_SECTION };

protected:
	virtual BOOL OnInitDialog() override;
	virtual void DoDataExchange(CDataExchange* pDX) override;

protected: // CRhRdkMaterialUISection_MFC overrides.
	virtual void DisplayData(void) override;
	virtual void EnableDisableControls(void) override;
	virtual const wchar_t* Caption(bool bAlwaysEnglish = false) const override;
	virtual UUID Uuid(void) const override;
	virtual AFX_MODULE_STATE* GetModuleState(void) const override { return AfxGetStaticModuleState(); }
	virtual unsigned int GetIDD(void) const override { return IDD; }

protected:
	DECLARE_MESSAGE_MAP()
};
#endif
