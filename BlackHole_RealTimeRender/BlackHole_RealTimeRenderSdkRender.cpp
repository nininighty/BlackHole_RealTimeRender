// BlackHole_RealTimeRenderSdkRender.cpp
//
// 这个文件是 Rhino 调用你渲染器的核心入口。
// 主要负责：
// 1 初始化渲染
// 2 获取场景 mesh
// 3 启动渲染线程
// 4 逐像素计算并写入 Rhino 渲染窗口
//

#include "stdafx.h"
#include "BlackHole_RealTimeRenderSdkRender.h"
#include "BlackHole_RealTimeRenderPlugIn.h"

CBlackHole_RealTimeRenderSdkRender::CBlackHole_RealTimeRenderSdkRender(
	const CRhinoCommandContext& context,
	CRhinoRenderPlugIn& plugin,
	const ON_wString& sCaption,
	UINT id,
	bool bPreview
)
	: CRhRdkSdkRender(context, plugin, sCaption, id)
{
	// 是否快速渲染（Rhino 小窗口预览模式）
	m_bRenderQuick = bPreview;

	// 是否取消渲染
	m_bCancel = false;

	// Rhino 模态渲染控制
	m_bContinueModal = true;

	m_hRenderThread = NULL;

	// 清空已有通道
	GetRenderWindow().ClearChannels();

	// 在渲染开始之前添加额外通道
	// 例如深度通道，雾效 / 景深等后期需要
	GetRenderWindow().AddChannel(IRhRdkRenderWindow::chanDistanceFromCamera, sizeof(float));
}

CBlackHole_RealTimeRenderSdkRender::~CBlackHole_RealTimeRenderSdkRender()
{
	// 析构时必须等待渲染线程结束
	if (m_hRenderThread != NULL)
	{
		::WaitForSingleObject(m_hRenderThread, INFINITE);
		::CloseHandle(m_hRenderThread);
		m_hRenderThread = NULL;
	}
}

CRhinoSdkRender::RenderReturnCodes CBlackHole_RealTimeRenderSdkRender::Render(const ON_2iSize& sizeRender)
{
	if (!::RhRdkIsAvailable())
		return CRhinoSdkRender::render_error_starting_render;

	// 获取当前视口
	const ON_Viewport& vp = RhinoApp().ActiveView()->ActiveViewport().VP();

	// 强制在主线程创建 Render Mesh
	// Rhino 内部对象（NURBS / SubD）最终都会转换成 Mesh 给渲染器
	IRhRdkSdkRenderMeshIterator* pIterator = NewRenderMeshIterator(vp);
	pIterator->EnsureRenderMeshesCreated();

	// 遍历场景中的所有 mesh
	// 这里通常用于：
	// 构建 BVH
	// 转换 mesh
	// 上传 GPU
	CRhRdkRenderMesh rm;
	pIterator->Reset();
	while (pIterator->Next(rm))
	{
		// TODO: 在这里处理 mesh
	}

	CRhinoSdkRender::RenderReturnCodes rc = CRhRdkSdkRender::Render(sizeRender);

	delete pIterator;

	return rc;
}

CRhinoSdkRender::RenderReturnCodes CBlackHole_RealTimeRenderSdkRender::RenderWindow(
	CRhinoView* pView,
	const LPRECT pRect,
	bool bInPopupWindow)
{
	if (!::RhRdkIsAvailable())
		return CRhinoSdkRender::render_error_starting_render;

	CRhinoDoc* pDocument = CommandContext().Document();
	if (nullptr == pDocument)
		return CRhinoSdkRender::render_error_starting_render;

	const auto sizeRender = RenderSize(*pDocument, true);

	const ON_Viewport& vp = pView->ActiveViewport().VP();

	// 创建 render mesh
	IRhRdkSdkRenderMeshIterator* pIterator = NewRenderMeshIterator(vp);
	pIterator->EnsureRenderMeshesCreated();

	CRhRdkRenderMesh rm;
	pIterator->Reset();
	while (pIterator->Next(rm))
	{
		// TODO
		// 这里通常构建加速结构 BVH
		// 注意：这个迭代器不是线程安全的
	}

	CRhinoSdkRender::RenderReturnCodes rc;

	if (bInPopupWindow)
	{
		// 在弹出窗口渲染
		const ON_4iRect rect(pRect->left, pRect->top, pRect->right, pRect->bottom);
		rc = __super::Render(rect.Size());
	}
	else
	{
		// 在 Rhino 视口实时渲染
		rc = __super::RenderWindow(pView, pRect, bInPopupWindow);
	}

	delete pIterator;

	return rc;
}

BOOL CBlackHole_RealTimeRenderSdkRender::NeedToProcessGeometryTable()
{
	// 如果场景改变，需要重新构建
	return ::BlackHole_RealTimeRenderPlugIn().SceneChanged();
}

BOOL CBlackHole_RealTimeRenderSdkRender::NeedToProcessLightTable()
{
	// 如果光照改变
	return ::BlackHole_RealTimeRenderPlugIn().LightingChanged();
}

BOOL CBlackHole_RealTimeRenderSdkRender::RenderPreCreateWindow()
{
	::BlackHole_RealTimeRenderPlugIn().SetSceneChanged(FALSE);
	::BlackHole_RealTimeRenderPlugIn().SetLightingChanged(FALSE);

	return TRUE;
}

BOOL CBlackHole_RealTimeRenderSdkRender::RenderContinueModal()
{
	return m_bContinueModal;
}

void CBlackHole_RealTimeRenderSdkRender::SetContinueModal(bool b)
{
	m_bContinueModal = b;
}

void CBlackHole_RealTimeRenderSdkRender::RenderThread(void* pv)
{
	// 线程入口
	reinterpret_cast<CBlackHole_RealTimeRenderSdkRender*>(pv)->ThreadedRender();
}

void CBlackHole_RealTimeRenderSdkRender::StartRendering()
{
	// 创建渲染线程
	m_hRenderThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RenderThread, this, 0, NULL);
}

BOOL CBlackHole_RealTimeRenderSdkRender::StartRenderingInWindow(CRhinoView*, const LPCRECT)
{
	m_hRenderThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RenderThread, this, 0, NULL);
	return TRUE;
}

void CBlackHole_RealTimeRenderSdkRender::StopRendering()
{
	// 停止渲染
	if (NULL != m_hRenderThread)
	{
		m_bCancel = true;

		::WaitForSingleObject(m_hRenderThread, INFINITE);
		::CloseHandle(m_hRenderThread);
		m_hRenderThread = NULL;
	}
}

static void CalculatePixel(
	int x,
	int y,
	const ON_2iSize& sizeRender,
	CRhRdkColor& colOut,
	float& zOut,
	bool bFast);

int CBlackHole_RealTimeRenderSdkRender::ThreadedRender(void)
{
	// 真正的渲染入口（核心）
	// 未来你的黑洞光线追踪就在这里

	m_bCancel = false;

	CRhinoDoc* pDocument = CommandContext().Document();
	if (nullptr == pDocument)
		return -1;

	const auto sizeRender = RenderSize(*pDocument, true);

	ON_4iRect rect(0, 0, sizeRender.cx, sizeRender.cy);

	IRhRdkRenderWindow& renderWnd = GetRenderWindow();

	// 设置渲染尺寸
	renderWnd.SetSize(sizeRender);

	// 打开颜色通道
	IRhRdkRenderWindow::IChannel* pChanRGBA =
		renderWnd.OpenChannel(IRhRdkRenderWindow::chanRGBA);

	if (nullptr != pChanRGBA)
	{
		// 打开深度通道
		IRhRdkRenderWindow::IChannel* pChanZ =
			renderWnd.OpenChannel(IRhRdkRenderWindow::chanDistanceFromCamera);

		if (nullptr != pChanZ)
		{
			float z = 0.0f;
			CRhRdkColor col;

			// 遍历每个像素
			for (int y = 0; y < sizeRender.cy; y++)
			{
				for (int x = 0; x < sizeRender.cx; x++)
				{
					CalculatePixel(x, y, sizeRender, col, z, m_bRenderQuick);

					if (m_bCancel)
						break;

					pChanRGBA->SetValue(x, y, ComponentOrder::RGBA, col.FloatArray());
					pChanZ->SetValue(x, y, ComponentOrder::Irrelevant, &z);
				}

				// 刷新这一行
				rect.top = y;
				rect.bottom = y + 1;
				renderWnd.InvalidateArea(rect);
			}

			pChanZ->Close();
		}

		pChanRGBA->Close();
	}

	// 通知 Rhino 渲染结束
	SetContinueModal(false);

	return 0;
}


// ===============================
// 以下是示例着色函数（假的渲染）
// ===============================

void CalcCore(
	int x,
	int y,
	float w,
	float h,
	double x1,
	double y1,
	float& r,
	float& g,
	float& b,
	float& a,
	float& z)
{
	const float k = min(w, h) * 0.5f;
	const float radius = k * 0.8f;

	float rr = 0.05f;
	float gg = 0.25f;
	float bb = 0.6f;
	float aa = 0.0f;
	float zz = 0.0f;

	const float dist = (float)sqrt((double)(x1 * x1) + (double)(y1 * y1));

	if (dist < radius)
	{
		bb = x / w;
		bb *= bb;

		rr = y / h;
		rr *= rr;

		zz = 1.0f - (dist / k);
		aa = 1.0f;
		gg = zz * 0.68f;
	}

	r += rr;
	g += gg;
	b += bb;
	a += aa;
	z += zz;
}

static void CalculatePixel(
	int x,
	int y,
	const ON_2iSize& sizeRender,
	CRhRdkColor& colOut,
	float& zOut,
	bool bFast)
{
	const float w = (float)sizeRender.cx;
	const float h = (float)sizeRender.cy;

	const double midX = w * 0.5f;
	const double midY = h * 0.5f;

	int samples = 0;

	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
	float a = 0.0f;
	float z = 0.0f;

	if (bFast)
	{
		const double x1 = midX - x;
		const double y1 = midY - y;
		CalcCore(x, y, w, h, x1, y1, r, g, b, a, z);
		samples++;
	}
	else
	{
		// 超采样
		for (float s = -0.5f; s < 0.5001f; s += 0.0625f)
		{
			for (float t = -0.5f; t < 0.5001f; t += 0.0625f)
			{
				const double x1 = midX - x + s;
				const double y1 = midY - y + t;
				CalcCore(x, y, w, h, x1, y1, r, g, b, a, z);
				samples++;
			}
		}
	}

	float* f = colOut.FloatArray();
	f[0] = r / samples;
	f[1] = g / samples;
	f[2] = b / samples;
	f[3] = a / samples;

	zOut = z;
}