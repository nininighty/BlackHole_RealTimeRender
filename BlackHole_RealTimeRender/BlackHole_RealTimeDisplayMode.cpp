#include "stdafx.h"
#include "BlackHole_RealTimeDisplayMode.h"

// ==============================================================================
// 第二部分：CBlackHole_RealTimeDisplayMode (显示模式管理器 / "大堂经理")
// 运行在 Rhino 主 UI 线程，负责监听视窗事件，并向后台渲染器下达命令。
// ==============================================================================

// [构造函数] 初始化显示模式
// 接收当前视窗的管线数据 (pipeline)，并将自己 (this) 注册给后台渲染器作为通讯接口
CBlackHole_RealTimeDisplayMode::CBlackHole_RealTimeDisplayMode(const CRhinoDisplayPipeline& pipeline)
    : m_Renderer(this), RhRdk::Realtime::DisplayMode(pipeline) {
}

// [析构函数] 销毁显示模式
// 当视窗被关闭时触发，确保立刻通知手下的后台渲染器停工
CBlackHole_RealTimeDisplayMode::~CBlackHole_RealTimeDisplayMode() {
    ShutdownRenderer();
}

// [启动渲染事件] 当 Rhino 视窗第一次切换到黑洞模式时被触发
bool CBlackHole_RealTimeDisplayMode::StartRenderer(const ON_2iSize& frameSize, const CRhinoDoc& doc, const ON_3dmView& view, const ON_Viewport& vp, const DisplayMode* pParent) {
    m_Renderer.UpdateCamera(vp);    // 直接把视口传给渲染器，解决 m_viewport 找不到的问题

    if (m_Renderer.Running()) return true;
    return m_Renderer.StartRenderProcess(frameSize);
}

// [视窗尺寸改变事件] 当用户拖拽改变 Rhino 窗口大小时被触发
bool CBlackHole_RealTimeDisplayMode::OnRenderSizeChanged(const ON_2iSize& newFrameSize) {
    // 粗暴但稳妥的做法：先停掉旧线程，按照新的长宽尺寸再重新开启一个新线程
    if (m_Renderer.Running()) m_Renderer.StopRenderProcess();
    return m_Renderer.StartRenderProcess(newFrameSize);
}

// [关闭渲染事件] 当用户切换到其他显示模式（如线框模式），或关闭软件时被触发
void CBlackHole_RealTimeDisplayMode::ShutdownRenderer() {
    m_Renderer.StopRenderProcess(); // 命令后台停工
}

// [状态查询] Rhino 询问：你的后台渲染器跑起来了吗？
bool CBlackHole_RealTimeDisplayMode::IsRendererStarted(void) const {
    return m_Renderer.Running();
}

// [状态查询] Rhino 询问：你的帧缓冲区准备好被读取了吗？
bool CBlackHole_RealTimeDisplayMode::IsFrameBufferAvailable(const ON_3dmView& vp) const {
    return true; // 始终准备好
}

// [锁定与绘制同步] Rhino 将我们后台算好的像素“偷”到前台屏幕上显示的核心操作
// 加锁是为了防止：我们在后台疯狂画的同时，前台刚好来拿数据，导致画面撕裂
bool CBlackHole_RealTimeDisplayMode::DrawOrLockRendererFrameBuffer(const FRAME_BUFFER_INFO_INPUTS& input, FRAME_BUFFER_INFO_OUTPUTS& outputs) {
    // 【核心修正】：根据 RhinoSdkDisplayPipeline.h，方法名是 VP()
    if (input.pipeline) {
        // pipeline->VP() 返回的是 ON_Viewport 的引用
        m_Renderer.UpdateCamera(input.pipeline->VP());
    }

    auto* pRW = m_Renderer.RenderWindow();
    if (nullptr == pRW) return false;

    // 调用基类的默认实现
    return DrawOrLockRendererFrameBufferImpl(*this, *pRW, input, outputs);
}

// [解锁帧缓冲区] 前台拿完数据了，解锁内存，让后台接着画下一帧
void CBlackHole_RealTimeDisplayMode::UnlockRendererFrameBuffer() {
    auto* pRW = m_Renderer.RenderWindow();
    if (nullptr != pRW) UnlockRendererFrameBufferImpl(*this, *pRW);
}


// ==============================================================================
// 第三部分：必须补全的虚函数 (占位符)
// 因为基类 RhRdk::Realtime::DisplayMode 定义了这些纯虚函数，不实现编译器会报错 (C2259)。
// 我们目前的黑洞纯数学渲染暂时用不到这些复杂特性，所以全给默认/空返回值。
// ==============================================================================

// [创建世界] 用于解析 Rhino 场景里的网格、灯光等实体（黑洞不需要，我们是自己算的）
void CBlackHole_RealTimeDisplayMode::CreateWorld(const CRhinoDoc& doc, const ON_3dmView& view, const CDisplayPipelineAttributes& attributes) {}

// [查询当前渲染趟数] 如果是离线渲染，会返回当前是第几 Pass。实时模式无视。
int CBlackHole_RealTimeDisplayMode::LastRenderedPass(void) const { return 0; }

// [是否显示进度条] 视窗渲染不需要进度条
bool CBlackHole_RealTimeDisplayMode::ShowCaptureProgress(void) const { return false; }

// [当前渲染进度]
double CBlackHole_RealTimeDisplayMode::Progress(void) const { return 0.0; }

// [是否已渲染完成] 实时渲染永远在循环，永远不“完成”
bool CBlackHole_RealTimeDisplayMode::IsCompleted(void) const { return false; }

// [是否使用快速绘制] 开启可能会降低画质，这里我们强制关掉
bool CBlackHole_RealTimeDisplayMode::UseFastDraw() { return false; }