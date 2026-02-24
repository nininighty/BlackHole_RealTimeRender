#pragma once

// 1. 必须首先包含这个，它定义了 Rhino SDK 的所有基础宏，
// 确保 RDK 能够识别当前是 Windows 平台并加载对应的渲染接口。
#include "rhinoSdk.h"

// 2. 包含你展示的那个 RDK 总头文件。
#include "RhRdkHeaders.h"

/**
 * 黑洞实时渲染器类
 */
class CBlackHoleRealtimeRenderer : public CRhRdkRealtimeRenderer
{
public:
	CBlackHoleRealtimeRenderer(const CRhinoView* pView);
	virtual ~CBlackHoleRealtimeRenderer() = default;

	// RDK 接口实现
	virtual bool IsCompleted() const override;
	virtual bool IsPaused() const override;
	virtual void OnDrawUpdate(HDC hdc, const ON_4iRect& rect) override;
};

/**
 * 渲染器工厂类
 */
class CBlackHoleRealtimeRendererFactory : public CRhRdkRealtimeRendererFactory
{
public:
	virtual UUID PlugInId() const override;
	virtual UUID RenderEngineId() const override;
	virtual ON_wString LocalName() const override;
	virtual CRhRdkRealtimeRenderer* CreateRenderer(const CRhinoView* pView) const override;
};