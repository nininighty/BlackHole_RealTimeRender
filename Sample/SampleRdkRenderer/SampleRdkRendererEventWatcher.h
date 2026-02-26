// SampleRdkRendererEventWatcher.h
//

#pragma once

// CSampleRdkRendererEventWatcher
// 有关此类实现的代码，请参阅 SampleRdkRendererEventWatcher.cpp。
//

class CSampleRdkRendererEventWatcher : public CRhinoEventWatcher
{
public:
    CSampleRdkRendererEventWatcher();

    // 重置所有标志位为默认值（通常为 FALSE）
    void Defaults(BOOL b = FALSE);

    /** 【Gemini 注解】：
        下面这些 BOOL 函数是插件的“状态查询接口”。
        渲染引擎会不断询问这些接口：“材质改了吗？”、“灯光动了吗？”。
    */
    BOOL MaterialModified() const;
    BOOL MaterialAdded() const;
    BOOL MaterialDeleted() const;
    BOOL RenderMeshModified() const;
    BOOL RenderMeshAdded() const;
    BOOL RenderMeshDeleted() const;
    BOOL RenderMeshVisibilityChanged() const;
    BOOL LightModified() const;
    BOOL LightAdded() const;
    BOOL LightDeleted() const;

    // 批量设置/重置各类标志位
    void SetRenderMeshFlags(BOOL b = FALSE);
    void SetMaterialFlags(BOOL b = FALSE);
    void SetLightFlags(BOOL b = FALSE);

    // 综合判定场景或光照是否发生了任何形式的修改
    virtual BOOL RenderSceneModified() const;
    virtual BOOL RenderLightingModified() const;

    /** 【Gemini 注解】：
        以下是重写的 CRhinoEventWatcher 虚函数。
        每当 Rhino 内部发生对应的事件时，这些函数就会被自动触发。
    */
    // CRhinoEventWatcher 重写方法
    void OnEnableEventWatcher(BOOL b) override;            // 开启/关闭监视器
    void OnInitRhino(CRhinoApp& app) override;            // Rhino 初始化时
    void OnCloseRhino(CRhinoApp& app) override;           // Rhino 关闭时
    void OnCloseDocument(CRhinoDoc& doc) override;        // 关闭文档时
    void OnNewDocument(CRhinoDoc& doc) override;          // 新建文档时
    void OnBeginOpenDocument(CRhinoDoc& doc, const wchar_t* filename, BOOL bMerge, BOOL bReference) override;
    void OnEndOpenDocument(CRhinoDoc& doc, const wchar_t* filename, BOOL bMerge, BOOL bReference) override;
    void OnBeginSaveDocument(CRhinoDoc& doc, const wchar_t* filename, BOOL bExportSelected) override;
    void OnEndSaveDocument(CRhinoDoc& doc, const wchar_t* filename, BOOL bExportSelected) override;
    void OnDocumentPropertiesChanged(CRhinoDoc& doc) override; // 文档属性改变

    // 命令执行前后的监听（例如用户执行了移动命令）
    void OnBeginCommand(const CRhinoCommand& command, const CRhinoCommandContext& context) override;
    void OnEndCommand(const CRhinoCommand& command, const CRhinoCommandContext& context, CRhinoCommand::result rc) override;

    // 物体增删改的监听
    void OnAddObject(CRhinoDoc& doc, CRhinoObject& object) override;
    void OnDeleteObject(CRhinoDoc& doc, CRhinoObject& object) override;
    void OnReplaceObject(CRhinoDoc& doc, CRhinoObject& old_object, CRhinoObject& new_object) override;
    void OnUnDeleteObject(CRhinoDoc& doc, CRhinoObject& object) override;
    void OnPurgeObject(CRhinoDoc& doc, CRhinoObject& object) override;

    // 选择状态监听
    void OnSelectObject(CRhinoDoc& doc, const CRhinoObject& object) override;
    void OnDeselectObject(CRhinoDoc& doc, const CRhinoObject& object) override;
    void OnDeselectAllObjects(CRhinoDoc& doc, int count) override;

    // 物体属性（如颜色、层）与表格（层表、灯光表、材质表）的变更监听
    void OnModifyObjectAttributes(CRhinoDoc& doc, CRhinoObject& object, const CRhinoObjectAttributes& old_attributes) override;
    void LayerTableEvent(CRhinoEventWatcher::layer_event event, const CRhinoLayerTable& layer_table, int layer_index, const ON_Layer* old_settings) override;
    void LightTableEvent(CRhinoEventWatcher::light_event event, const CRhinoLightTable& light_table, int light_index, const ON_Light* old_settings) override;
    void MaterialTableEvent(CRhinoEventWatcher::material_event event, const CRhinoMaterialTable& material_table, int material_index, const ON_Material* old_settings) override;
    void GroupTableEvent(CRhinoEventWatcher::group_event event, const CRhinoGroupTable& group_table, int group_index, const ON_Group* old_settings) override;
    void DimStyleTableEvent(CRhinoEventWatcher::dimstyle_event event, const CRhinoDimStyleTable& dimstyle_table, int dimstyle_index, const ON_DimStyle* old_settings) override;

    // 几何网格更新监听
    void OnUpdateObjectMesh(CRhinoDoc& doc, CRhinoObject& object, ON::mesh_type mesh_type) override;

private:
    /** 【Gemini 注解】：
        这些私有成员就是所谓的“脏位（Dirty Flags）”。
        一旦对应的事件触发，就把它们设为 TRUE。
        渲染引擎看到 TRUE，就知道：“哦，场景变了，我之前的 ML 预测结果失效了，得重算。”
    */
    BOOL m_material_modified;
    BOOL m_material_added;
    BOOL m_material_deleted;
    BOOL m_render_mesh_modified;
    BOOL m_render_mesh_added;
    BOOL m_render_mesh_deleted;
    BOOL m_render_mesh_visibility_changed;
    BOOL m_light_modified;
    BOOL m_light_added;
    BOOL m_light_deleted;
};