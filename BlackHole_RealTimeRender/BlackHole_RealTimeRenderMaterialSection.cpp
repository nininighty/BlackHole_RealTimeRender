// BlackHole_RealTimeRenderMaterialSection.cpp
//

#include "stdafx.h"
#include "BlackHole_RealTimeRenderMaterialSection.h"
#include "BlackHole_RealTimeRenderMaterial.h"

#if defined (RHINO_SDK_MFC)
CBlackHole_RealTimeRenderMaterialSection::CBlackHole_RealTimeRenderMaterialSection()
	:
	CRhRdkMaterialUISection_MFC(IDD)
{
}

UUID CBlackHole_RealTimeRenderMaterialSection::Uuid() const
{
	// {F425261A-F002-456D-B3E7-F0B2C46DFB35}
	static const GUID uuid = 
	{0xf425261a,0xf002,0x456d,{0xb3,0xe7,0xf0,0xb2,0xc4,0x6d,0xfb,0x35}};
	return uuid;
}

void CBlackHole_RealTimeRenderMaterialSection::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	// TODO: Add your controls here.

//	DDX_Control(pDX, IDC_XXXX, m_xxxx);
}

BEGIN_MESSAGE_MAP(CBlackHole_RealTimeRenderMaterialSection, CRhRdkMaterialUISection_MFC)
END_MESSAGE_MAP()

BOOL CBlackHole_RealTimeRenderMaterialSection::OnInitDialog()
{
	__super::OnInitDialog();

	DisplayData();
	EnableDisableControls();

	return FALSE;
}

const wchar_t* CBlackHole_RealTimeRenderMaterialSection::Caption(bool bAlwaysEnglish) const
{
	UNREFERENCED_PARAMETER(bAlwaysEnglish);

	return L"BlackHole_RealTimeRender Material";
}

void CBlackHole_RealTimeRenderMaterialSection::DisplayData()
{
	CRhRdkMaterialArray a;
	GetMaterialList(a);

	if (0 == a.Count())
	{
		return;
	}

	// TODO: transfer editable material state to UI controls.
}

void CBlackHole_RealTimeRenderMaterialSection::EnableDisableControls()
{
	// TODO: enable and disable controls based on material state.
}
#endif
