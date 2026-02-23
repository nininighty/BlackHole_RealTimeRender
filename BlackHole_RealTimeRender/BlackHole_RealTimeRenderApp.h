// BlackHole_RealTimeRender.h : main header file for the BlackHole_RealTimeRender DLL.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h" // main symbols

// CBlackHole_RealTimeRenderApp
// See BlackHole_RealTimeRenderApp.cpp for the implementation of this class
//

class CBlackHole_RealTimeRenderApp : public CWinApp
{
public:
  // CRITICAL: DO NOT CALL RHINO SDK FUNCTIONS HERE!
  // Only standard MFC DLL instance construction belongs here. 
  // All other significant initialization should take place in
  // CBlackHole_RealTimeRenderPlugIn::OnLoadPlugIn().
	CBlackHole_RealTimeRenderApp() = default;

  // CRITICAL: DO NOT CALL RHINO SDK FUNCTIONS HERE!
  // Only standard MFC DLL instance initialization belongs here. 
  // All other significant initialization should take place in
  // CBlackHole_RealTimeRenderPlugIn::OnLoadPlugIn().
	BOOL InitInstance() override;
  
  // CRITICAL: DO NOT CALL RHINO SDK FUNCTIONS HERE!
  // Only standard MFC DLL instance clean up belongs here. 
  // All other significant cleanup should take place in either
  // CBlackHole_RealTimeRenderPlugIn::OnSaveAllSettings() or
  // CBlackHole_RealTimeRenderPlugIn::OnUnloadPlugIn().  
	int ExitInstance() override;
  
	DECLARE_MESSAGE_MAP()
};
