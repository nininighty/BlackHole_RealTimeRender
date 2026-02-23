// BlackHole_RealTimeRenderPlugIn.h : BlackHole_RealTimeRender 插件的主要头文件。
// 

#pragma once

#include "BlackHole_RealTimeRenderEventWatcher.h"

class CBlackHole_RealTimeRenderRdkPlugIn;

// CBlackHole_RealTimeRenderPlugIn
// 请参阅 BlackHole_RealTimeRenderPlugIn.cpp 了解此类的实现
//

class CBlackHole_RealTimeRenderPlugIn : public CRhinoRenderPlugIn
{
public:
    // CBlackHole_RealTimeRenderPlugIn 构造函数。插件加载时调用该构造函数，
    // 并构造“thePlugIn”。插件加载后会调用 CBlackHole_RealTimeRenderPlugIn::OnLoadPlugIn()。
    // 构造函数应该简单、稳定。任何可能失败的操作应该放在 CBlackHole_RealTimeRenderPlugIn::OnLoadPlugIn() 中。
    CBlackHole_RealTimeRenderPlugIn();

    // CBlackHole_RealTimeRenderPlugIn 析构函数。插件卸载时调用该析构函数销毁
    // "thePlugIn"。在 DLL 卸载之前会调用 CBlackHole_RealTimeRenderPlugIn::OnUnloadPlugin()。
    // 不要做太多工作。在这里清理你通过 onmalloc()、onrealloc()、oncalloc() 或 onstrdup() 分配的内存。
    ~CBlackHole_RealTimeRenderPlugIn() = default;

    // 必须重写的函数

    // 插件名称显示字符串。该名称在 Rhino 加载插件时、
    // 插件帮助菜单中，以及 Rhino 插件管理界面中显示。 
    const wchar_t* PlugInName() const override;

    // 插件版本显示字符串。该字符串在 Rhino 加载插件时
    // 以及 Rhino 插件管理界面中显示。
    const wchar_t* PlugInVersion() const override;

    // 插件唯一标识符。该标识符由 Rhino 用于管理插件。
    GUID PlugInID() const override;

    // 其他重写的函数

    // 插件加载后，构造函数运行时调用此函数。
    // 这是执行任何重要初始化、许可证检查等操作的好地方。
    // 此函数必须返回 TRUE 插件才会继续加载。  
    BOOL OnLoadPlugIn() override;

    // 插件即将卸载时调用一次。在此时，
    // Rhino 的主框架窗口已经销毁，并且一些 SDK 管理器已经被删除。
    // 此时也没有活动文档或活动视图，因此这里只能操作你自己的对象或工具。  
    void OnUnloadPlugIn() override;

    // 渲染相关重写
    CRhinoCommand::result Render(const CRhinoCommandContext& context, bool bPreview) override;
    CRhinoCommand::result RenderWindow(
        const CRhinoCommandContext& context,
        bool render_preview,
        CRhinoView* view,
        const LPRECT rect,
        bool bInWindow,
        bool bBlowUp
    ) override;

    BOOL SaveRenderedImage(ON_wString filename) override;
    BOOL CloseRenderWindow() override;

    // 渲染方法
    CRhinoCommand::result RenderQuiet(const CRhinoCommandContext& context, bool bPreview);
    BOOL SceneChanged() const;
    void SetSceneChanged(BOOL bChanged);
    BOOL LightingChanged() const;
    void SetLightingChanged(BOOL bChanged);
    UINT MainFrameResourceID() const;

private:
    ON_wString m_plugin_version;
    CBlackHole_RealTimeRenderEventWatcher m_event_watcher;
    CBlackHole_RealTimeRenderRdkPlugIn* m_pRdkPlugIn;

    // TODO：在这里添加额外的类信息
};

// 返回唯一的 CBlackHole_RealTimeRenderPlugIn 对象的引用
CBlackHole_RealTimeRenderPlugIn& BlackHole_RealTimeRenderPlugIn();
