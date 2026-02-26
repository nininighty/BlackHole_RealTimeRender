// SampleRdkRendererEventWatcher.cpp
//

#include "StdAfx.h"
#include "SampleRdkRendererEventWatcher.h"

/////////////////////////////////////////////////////////////////////////////
// 构造与析构函数
//

CSampleRdkRendererEventWatcher::CSampleRdkRendererEventWatcher()
{
    // 初始化默认状态
    Defaults();
}

void CSampleRdkRendererEventWatcher::Defaults(BOOL b /*= FALSE*/)
{
    // 将所有状态标志位设为指定值（默认为 FALSE）
    SetRenderMeshFlags(b);
    SetMaterialFlags(b);
    SetLightFlags(b);
}

/////////////////////////////////////////////////////////////////////////////
// CSampleRdkRendererEventWatcher 方法
//

// 判断场景几何或材质是否被修改
BOOL CSampleRdkRendererEventWatcher::RenderSceneModified() const
{
    return(MaterialModified() ||
        MaterialAdded() ||
        MaterialDeleted() ||
        RenderMeshModified() ||
        RenderMeshAdded() ||
        RenderMeshDeleted() ||
        RenderMeshVisibilityChanged()
        );
}

// 判断光照系统是否被修改
BOOL CSampleRdkRendererEventWatcher::RenderLightingModified() const
{
    return(LightModified() ||
        LightAdded() ||
        LightDeleted()
        );
}

/** 【Gemini 注解】：
 下面这一系列函数（MaterialModified 到 LightDeleted）是简单的状态查询接口。
 它们被渲染主循环频繁调用，用于决定是否需要清空当前的渲染缓存并重新开始。
 对于你的黑洞项目，如果背景星空图换了，或者吸积盘的参数改了，这里就会触发。
*/

BOOL CSampleRdkRendererEventWatcher::MaterialModified() const
{
    return m_material_modified;
}

BOOL CSampleRdkRendererEventWatcher::MaterialAdded() const
{
    return m_material_added;
}

BOOL CSampleRdkRendererEventWatcher::MaterialDeleted() const
{
    return m_material_deleted;
}

BOOL CSampleRdkRendererEventWatcher::RenderMeshModified() const
{
    return m_render_mesh_modified;
}

BOOL CSampleRdkRendererEventWatcher::RenderMeshAdded() const
{
    return m_render_mesh_added;
}

BOOL CSampleRdkRendererEventWatcher::RenderMeshDeleted() const
{
    return m_render_mesh_deleted;
}

BOOL CSampleRdkRendererEventWatcher::RenderMeshVisibilityChanged() const
{
    return m_render_mesh_visibility_changed;
}

BOOL CSampleRdkRendererEventWatcher::LightModified() const
{
    return m_light_modified;
}

BOOL CSampleRdkRendererEventWatcher::LightAdded() const
{
    return m_light_added;
}

BOOL CSampleRdkRendererEventWatcher::LightDeleted() const
{
    return m_light_deleted;
}

// 批量设置网格相关的标志位
void CSampleRdkRendererEventWatcher::SetRenderMeshFlags(BOOL b /*= FALSE*/)
{
    m_render_mesh_modified = b;
    m_render_mesh_added = b;
    m_render_mesh_deleted = b;
    m_render_mesh_visibility_changed = b;
}

// 批量设置材质相关的标志位
void CSampleRdkRendererEventWatcher::SetMaterialFlags(BOOL b /*= FALSE*/)
{
    m_material_modified = b;
    m_material_added = b;
    m_material_deleted = b;
}

// 批量设置灯光相关的标志位
void CSampleRdkRendererEventWatcher::SetLightFlags(BOOL b /*= FALSE*/)
{
    m_light_modified = b;
    m_light_added = b;
    m_light_deleted = b;
}

/////////////////////////////////////////////////////////////////////////////
// CRhinoEventWatcher 重写方法（这是核心逻辑所在）
//

void CSampleRdkRendererEventWatcher::OnEnableEventWatcher(BOOL b)
{
    UNREFERENCED_PARAMETER(b);
    Defaults(); // 开启监听时重置状态
}

void CSampleRdkRendererEventWatcher::OnInitRhino(CRhinoApp& app)
{
    UNREFERENCED_PARAMETER(app);
}

void CSampleRdkRendererEventWatcher::OnCloseRhino(CRhinoApp& app)
{
    UNREFERENCED_PARAMETER(app);
}

void CSampleRdkRendererEventWatcher::OnCloseDocument(CRhinoDoc& doc)
{
    UNREFERENCED_PARAMETER(doc);
}

void CSampleRdkRendererEventWatcher::OnNewDocument(CRhinoDoc& doc)
{
    UNREFERENCED_PARAMETER(doc);
    Defaults(true); // 新文档视为全部改变
}

void CSampleRdkRendererEventWatcher::OnBeginOpenDocument(CRhinoDoc& doc, const wchar_t* filename, BOOL bMerge, BOOL bReference)
{
    UNREFERENCED_PARAMETER(doc);
    UNREFERENCED_PARAMETER(filename);
    UNREFERENCED_PARAMETER(bMerge);
    UNREFERENCED_PARAMETER(bReference);
    Defaults(true);
}

void CSampleRdkRendererEventWatcher::OnEndOpenDocument(CRhinoDoc& doc, const wchar_t* filename, BOOL bMerge, BOOL bReference)
{
    UNREFERENCED_PARAMETER(doc);
    UNREFERENCED_PARAMETER(filename);
    UNREFERENCED_PARAMETER(bMerge);
    UNREFERENCED_PARAMETER(bReference);
    Defaults(true);
}

void CSampleRdkRendererEventWatcher::OnBeginSaveDocument(CRhinoDoc& doc, const wchar_t* filename, BOOL bExportSelected)
{
    UNREFERENCED_PARAMETER(doc);
    UNREFERENCED_PARAMETER(filename);
    UNREFERENCED_PARAMETER(bExportSelected);
}

void CSampleRdkRendererEventWatcher::OnEndSaveDocument(CRhinoDoc& doc, const wchar_t* filename, BOOL bExportSelected)
{
    UNREFERENCED_PARAMETER(doc);
    UNREFERENCED_PARAMETER(filename);
    UNREFERENCED_PARAMETER(bExportSelected);
}

void CSampleRdkRendererEventWatcher::OnDocumentPropertiesChanged(CRhinoDoc& doc)
{
    UNREFERENCED_PARAMETER(doc);
}

void CSampleRdkRendererEventWatcher::OnBeginCommand(const CRhinoCommand& command, const CRhinoCommandContext& context)
{
    UNREFERENCED_PARAMETER(command);
    UNREFERENCED_PARAMETER(context);
}

void CSampleRdkRendererEventWatcher::OnEndCommand(
    const CRhinoCommand& command,
    const CRhinoCommandContext& context,
    CRhinoCommand::result rc
)
{
    UNREFERENCED_PARAMETER(command);
    UNREFERENCED_PARAMETER(context);
    UNREFERENCED_PARAMETER(rc);
}

// 当场景中添加新物体时调用
void CSampleRdkRendererEventWatcher::OnAddObject(CRhinoDoc& doc, CRhinoObject& object)
{
    UNREFERENCED_PARAMETER(doc);

    // TODO: 此处应检查是否为灯光
    // 如果物体可以生成渲染网格（Mesh），标记为“网格已添加”
    if (object.IsMeshable(ON::render_mesh))
        m_render_mesh_added = true;
}

// 当场景中删除物体时调用
void CSampleRdkRendererEventWatcher::OnDeleteObject(CRhinoDoc& doc, CRhinoObject& object)
{
    UNREFERENCED_PARAMETER(doc);

    // TODO: 此处应检查是否为灯光
    if (object.IsMeshable(ON::render_mesh))
        m_render_mesh_deleted = true;
}

// 当物体被替换时（例如移动、缩放物体本质上是删掉旧的换个新的）
void CSampleRdkRendererEventWatcher::OnReplaceObject(CRhinoDoc& doc, CRhinoObject& old_object, CRhinoObject& new_object)
{
    UNREFERENCED_PARAMETER(doc);

    // TODO: 此处应检查是否为灯光
    ON_SimpleArray<const ON_Mesh*> old_meshes, new_meshes;

    // 检查旧物体和新物体是否包含渲染网格
    BOOL bOldMeshes = (old_object.GetMeshes(ON::render_mesh, old_meshes) < 1);
    BOOL bNewMeshes = (new_object.GetMeshes(ON::render_mesh, new_meshes) < 1);

    if (bOldMeshes)
    {
        if (new_object.IsMeshable(ON::render_mesh))
        {
            if (bNewMeshes)
                m_render_mesh_modified = true;
            else
                m_render_mesh_deleted = true;
        }
        else
        {
            m_render_mesh_deleted = true;
        }
    }
    else if (bNewMeshes)
    {
        m_render_mesh_added = true;
    }
    else if (new_object.IsMeshable(ON::render_mesh))
    {
        m_render_mesh_added = true;
    }
}

// 当撤销删除（Undelete）物体时调用
void CSampleRdkRendererEventWatcher::OnUnDeleteObject(CRhinoDoc& doc, CRhinoObject& object)
{
    UNREFERENCED_PARAMETER(doc);

    // TODO: 此处应检查是否为灯光
    if (object.IsMeshable(ON::render_mesh))
        m_render_mesh_added = true;
}

void CSampleRdkRendererEventWatcher::OnPurgeObject(CRhinoDoc& doc, CRhinoObject& object)
{
    UNREFERENCED_PARAMETER(doc);
    UNREFERENCED_PARAMETER(object);
}

void CSampleRdkRendererEventWatcher::OnSelectObject(CRhinoDoc& doc, const CRhinoObject& object)
{
    UNREFERENCED_PARAMETER(doc);
    UNREFERENCED_PARAMETER(object);
}

void CSampleRdkRendererEventWatcher::OnDeselectObject(CRhinoDoc& doc, const CRhinoObject& object)
{
    UNREFERENCED_PARAMETER(doc);
    UNREFERENCED_PARAMETER(object);
}

void CSampleRdkRendererEventWatcher::OnDeselectAllObjects(CRhinoDoc& doc, int count)
{
    UNREFERENCED_PARAMETER(doc);
    UNREFERENCED_PARAMETER(count);
}

// 当物体属性（如材质来源、图层）改变时调用
void CSampleRdkRendererEventWatcher::OnModifyObjectAttributes(
    CRhinoDoc& doc,
    CRhinoObject& object,
    const CRhinoObjectAttributes& old_attributes
)
{
    UNREFERENCED_PARAMETER(doc);
    UNREFERENCED_PARAMETER(object);

    // 如果该物体根本不产生网格，直接忽略
    if (!object.IsMeshable(ON::render_mesh))
        return;

    const CRhinoObjectAttributes& new_attributes = object.Attributes();

    // 检查材质来源是否发生变化，或者材质索引是否改变
    if (old_attributes.MaterialSource() == new_attributes.MaterialSource())
    {
        if (new_attributes.MaterialSource() == ON::material_from_object
            && old_attributes.m_material_index != new_attributes.m_material_index)
        {
            m_render_mesh_modified = true;
        }
        else if (new_attributes.MaterialSource() == ON::material_from_layer
            && old_attributes.m_layer_index != new_attributes.m_layer_index)
        {
            m_render_mesh_modified = true;
        }
    }
    else
        m_render_mesh_modified = true;
}

// 当物体的网格被手动强制更新时调用
void CSampleRdkRendererEventWatcher::OnUpdateObjectMesh(
    CRhinoDoc& doc,
    CRhinoObject& object,
    ON::mesh_type mesh_type
)
{
    UNREFERENCED_PARAMETER(doc);
    UNREFERENCED_PARAMETER(object);

    if (mesh_type == ON::render_mesh)
        m_render_mesh_modified = true;
}

// 处理图层表（Layer Table）相关的事件
void CSampleRdkRendererEventWatcher::LayerTableEvent(
    CRhinoEventWatcher::layer_event event,
    const CRhinoLayerTable& layer_table,
    int layer_index,
    const ON_Layer* old_settings
)
{
    if (event != CRhinoEventWatcher::layer_modified)
        return;

    const CRhinoLayer& layer = layer_table[layer_index];

    if (old_settings)
    {
        // 检查图层绑定的渲染材质是否改变
        if (layer.RenderMaterialIndex() != old_settings->RenderMaterialIndex())
            m_render_mesh_modified = true;

        // 检查图层可见性（影藏图层意味着物体在渲染中消失）
        if (layer.IsVisible() != old_settings->IsVisible())
            m_light_modified = m_render_mesh_modified = true;

        // 检查图层锁定状态
        if (layer.IsLocked() != old_settings->IsLocked())
            m_light_modified = m_render_mesh_modified = true;
    }
}

// 处理灯光表（Light Table）相关的事件
void CSampleRdkRendererEventWatcher::LightTableEvent(
    CRhinoEventWatcher::light_event event,
    const CRhinoLightTable& light_table,
    int light_index,
    const ON_Light* old_settings
)
{
    UNREFERENCED_PARAMETER(old_settings);
    UNREFERENCED_PARAMETER(light_index);
    UNREFERENCED_PARAMETER(light_table);

    switch (event)
    {
    case light_added:
    case light_undeleted:
        m_light_added = true;
        break;
    case light_deleted:
        m_light_deleted = true;
        break;
    case light_modified:
        m_light_modified = true;
        break;
    }
}

// 处理材质表（Material Table）相关的事件
void CSampleRdkRendererEventWatcher::MaterialTableEvent(
    CRhinoEventWatcher::material_event event,
    const CRhinoMaterialTable& material_table,
    int material_index,
    const ON_Material* old_settings
)
{
    UNREFERENCED_PARAMETER(old_settings);
    UNREFERENCED_PARAMETER(material_index);
    UNREFERENCED_PARAMETER(material_table);

    switch (event)
    {
    case CRhinoEventWatcher::material_added:
    case CRhinoEventWatcher::material_undeleted:
        m_material_added = true;
        break;
    case CRhinoEventWatcher::material_deleted:
        m_material_deleted = true;
        break;
    case CRhinoEventWatcher::material_modified:
        m_material_modified = true;
        break;
    }
}

/** 【Gemini 注解】：
 组、标注样式等事件在这里被忽略，因为它们通常不直接影响物理渲染结果。
*/

void CSampleRdkRendererEventWatcher::GroupTableEvent(
    CRhinoEventWatcher::group_event event,
    const CRhinoGroupTable& group_table,
    int group_index,
    const ON_Group* old_settings
)
{
    UNREFERENCED_PARAMETER(event);
    UNREFERENCED_PARAMETER(group_table);
    UNREFERENCED_PARAMETER(group_index);
    UNREFERENCED_PARAMETER(old_settings);
}

void CSampleRdkRendererEventWatcher::DimStyleTableEvent(
    CRhinoEventWatcher::dimstyle_event event,
    const CRhinoDimStyleTable& dimstyle_table,
    int dimstyle_index,
    const ON_DimStyle* old_settings
)
{
    UNREFERENCED_PARAMETER(event);
    UNREFERENCED_PARAMETER(dimstyle_table);
    UNREFERENCED_PARAMETER(dimstyle_index);
    UNREFERENCED_PARAMETER(old_settings);
}