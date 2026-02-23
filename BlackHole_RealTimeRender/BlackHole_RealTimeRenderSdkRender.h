// BlackHole_RealTimeRenderSdkRender.h
//

#pragma once

// CBlackHole_RealTimeRenderSdkRender
// See BlackHole_RealTimeRenderSdkRender.cpp for the implementation of this class.
//

class CBlackHole_RealTimeRenderSdkRender : public CRhRdkSdkRender
{
public:
	CBlackHole_RealTimeRenderSdkRender(
	const CRhinoCommandContext& context,
	CRhinoRenderPlugIn& pPlugin,
	const ON_wString& sCaption,
	UINT idIcon,
	bool bPreview
	);

	virtual ~CBlackHole_RealTimeRenderSdkRender();

	int ThreadedRender(void);
	void SetContinueModal(bool b);

	CRhinoSdkRender::RenderReturnCodes Render(const ON_2iSize& sizeRender) override;
	CRhinoSdkRender::RenderReturnCodes RenderWindow(CRhinoView* pView, const LPRECT pRect, bool bInPopupWindow) override;

	// CRhRdkSdkRender overrides.
	virtual BOOL RenderSceneWithNoMeshes() override { return TRUE; }
	virtual BOOL IgnoreRhinoObject(const CRhinoObject*) override { return FALSE; }
	virtual BOOL RenderPreCreateWindow() override;
	virtual BOOL RenderEnterModalLoop() override { return TRUE; }
	virtual BOOL RenderContinueModal() override;
	virtual BOOL RenderExitModalLoop() override { return TRUE; }
	virtual bool ReuseRenderWindow(void) const override { return true; }
	virtual BOOL NeedToProcessGeometryTable() override;
	virtual BOOL NeedToProcessLightTable() override;
	virtual BOOL StartRenderingInWindow(CRhinoView* pView, const LPCRECT pRect) override;
	virtual void StopRendering() override;
	virtual void StartRendering() override;

protected:
	static void RenderThread(void* pv);

private:
	HANDLE m_hRenderThread;
	bool m_bContinueModal;
	bool m_bRenderQuick;
	bool m_bCancel;
};
