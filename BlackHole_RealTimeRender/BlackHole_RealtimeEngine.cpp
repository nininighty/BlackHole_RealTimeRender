#include "stdafx.h"
#include "BlackHole_RealtimeEngine.h"
#include "BlackHole_RealTimeRenderPlugIn.h"

// --- Renderer 实现 ---
CBlackHoleRealtimeRenderer::CBlackHoleRealtimeRenderer(const CRhinoView* pView)
    : CRhRdkRealtimeRenderer(pView)
{
}

bool CBlackHoleRealtimeRenderer::IsCompleted() const { return false; }
bool CBlackHoleRealtimeRenderer::IsPaused() const { return false; }
void CBlackHoleRealtimeRenderer::OnDrawUpdate(HDC hdc, const ON_4iRect& rect) {
    // 暂时留空，这里是未来绘制黑洞像素的地方
}

// --- Factory 实现 ---
UUID CBlackHoleRealtimeRendererFactory::PlugInId() const {
    return ::BlackHole_RealTimeRenderPlugIn().PlugInID();
}

UUID CBlackHoleRealtimeRendererFactory::RenderEngineId() const {
    return ::BlackHole_RealTimeRenderPlugIn().PlugInID();
}

ON_wString CBlackHoleRealtimeRendererFactory::LocalName() const {
    return L"SJU_BlackHole_Engine";
}

CRhRdkRealtimeRenderer* CBlackHoleRealtimeRendererFactory::CreateRenderer(const CRhinoView* pView) const {
    return new CBlackHoleRealtimeRenderer(pView);
}