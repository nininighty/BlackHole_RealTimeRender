#include "stdafx.h"
#include "SampleRdkRendererSdkRender.h"
#include "SampleRdkRendererPlugIn.h"

// 构造函数：初始化渲染设置
CSampleRdkRendererSdkRender::CSampleRdkRendererSdkRender(const CRhinoCommandContext& context, CRhinoRenderPlugIn& plugIn,
	const ON_wString& sCaption, UINT id, bool bPreview)
	:
	CRhRdkSdkRender(context, plugIn, sCaption, id)
{
	// 清除渲染窗口中的所有现有通道
	GetRenderWindow().ClearChannels();

	// TODO: 在渲染开始前添加任何额外通道。

	// 例如：添加一个“相机距离”通道（Z-Buffer），这允许雾效和景深（DOF）等后处理效果工作。
	GetRenderWindow().AddChannel(IRhRdkRenderWindow::chanDistanceFromCamera, sizeof(float));

	// 添加法线通道（X, Y, Z 分量）
	GetRenderWindow().AddChannel(IRhRdkRenderWindow::chanNormalX, sizeof(float));
	GetRenderWindow().AddChannel(IRhRdkRenderWindow::chanNormalY, sizeof(float));
	GetRenderWindow().AddChannel(IRhRdkRenderWindow::chanNormalZ, sizeof(float));

	/** 【Gemini 注解】：
	 在黑洞项目中，除了 RGBA 颜色，你还可以利用这些通道存储物理信息。
	 比如在 Z 通道存储引力红移强度，或者在法线通道存储光线偏折的最终方向向量。
	*/
}

CSampleRdkRendererSdkRender::~CSampleRdkRendererSdkRender()
{
	// 析构时确保渲染线程已销毁
	ASSERT(NULL == m_hRenderThread);
}

// 辅助函数：生成 0.0 到 1.0 之间的随机数
static float Random(void)
{
	return float(rand()) / RAND_MAX;
}

// 核心渲染函数：在这里编写计算逻辑
bool CSampleRdkRendererSdkRender::RenderCore(void)
{
	// TODO: 用你自己的渲染代码替换此部分。

	if ((nullptr == m_pChanRGBA) || (nullptr == m_pChanZ))
		return false;

	ON_4iRect rect(0, 0, m_Region.Width(), 0);

	int percent = 0;

	const int width = m_Region.Width();
	const int height = m_Region.Height();

	// 处理抗锯齿采样率
	int samples = 1, sampleRate = 1;
	if (!m_bPreview)
	{
		switch (m_RenderSettings.m_antialias_style)
		{
		case 1: sampleRate = 2;  break;
		case 2: sampleRate = 4;  break;
		case 3: sampleRate = 16; break;
		}

		samples = sampleRate * sampleRate;
	}

	const float pi = float(ON_PI);

	const float size = float(min(width, height));

	const CPoint origin(m_Region.left, m_Region.bottom);

	// 马赛克尺寸，用于预览时的块状加速显示
	const int mosaic = 8;

	const int delta = m_bPreview ? mosaic : 1;

	// 开始像素遍历循环
	for (int y = m_Region.top; y < m_Region.bottom; y += delta)
	{
		// 检查用户是否点击了“取消”
		if (m_bCancel)
			break;

		// 调试用：按下 SHIFT 键强行模拟失败
		if (0 != (::GetAsyncKeyState(VK_SHIFT) & 0x8000))
			return false;

		const int pixelY = y - m_Region.top;

		for (int x = m_Region.left; x < m_Region.right; x += delta)
		{
			const int pixelX = x - m_Region.left;

			float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;

			// 抗锯齿采样循环
			for (int i = 0; i < samples; i++)
			{
				const float pixoffx = (i % sampleRate) / float(sampleRate);
				const float pixoffy = (i / sampleRate) / float(sampleRate);

				const float dx = (x - origin.x + pixoffx) / size;
				const float dy = (y - origin.y + pixoffy) / size;

				/** 【Gemini 注解】：
				 这一段官方示例代码画了一个程序化的“分形圆圈”。
				 在你的黑洞项目中，这里就是发射光线的地方。
				 你需要在这里根据 (dx, dy) 确定光线方向，然后呼叫
				 RK4 积分函数或 ML 模型推理接口来获取该像素的最终颜色。
				*/

				const float dist2 = (dx * dx) + (dy * dy);
				if (dist2 < 0.95f)
				{
					const float at = fabs(atan2(dy, dx));

					float weight = 0.0f;

					float t0 = at - pi * 0.0f / 6.0f; t0 = t0 * t0;
					float t1 = at - pi * 1.0f / 6.0f; t1 = t1 * t1;
					float t2 = at - pi * 2.0f / 6.0f; t2 = t2 * t2;
					float t3 = at - pi * 3.0f / 6.0f; t3 = t3 * t3;

					float t4 = dist2 - 0.005f; t4 = t4 * t4;
					float t5 = dist2 - 0.950f; t5 = t5 * t5;

					float t6 = fmod(at, pi / 50.0f); t6 = t6 * t6;

					if (t5 < 0.0300f) weight += (0.0300f - t5) / 0.0300f;
					else
						if (t4 < 0.0001f) weight += (0.0001f - t4) / 0.0001f;
						else
						{
							bool bWeight = false;
							if (t0 < 0.0010f) { weight += (0.001f - t0) * 800.0f; bWeight = true; }
							if (t1 < 0.0010f) { weight += (0.001f - t1) * 800.0f; bWeight = true; }
							if (t2 < 0.0010f) { weight += (0.001f - t2) * 800.0f; bWeight = true; }
							if (t3 < 0.0010f) { weight += (0.001f - t3) * 800.0f; bWeight = true; }

							if (!bWeight && (t6 < 0.0030f)) weight += (0.0300f - t6) * 5.0f;
						}

					r += 0.9f * weight + 0.9f;
					g += 0.5f * weight + 0.3f + 0.5f * fmod(Random(), fabs(dx) + 1e-6f);
					b += 0.2f * weight + 0.1f + 0.5f * fmod(Random(), fabs(dy) + 1e-6f);
					a += 1.0f;
				}
				else
				{
					r += 0.2f; g += 0.5f; b += 0.9f;
				}
			}

			// 最终颜色归一化
			float rgba[4];
			rgba[0] = max(0.0f, min(1.0f, r / samples));
			rgba[1] = max(0.0f, min(1.0f, g / samples));
			rgba[2] = max(0.0f, min(1.0f, b / samples));
			rgba[3] = max(0.0f, min(1.0f, a / samples));

			const float z = (rgba[0] + rgba[1] + rgba[2]) / 3.0f;

			const float nx = rgba[0] * 2.0f;
			const float ny = rgba[1] * 2.0f;
			const float nz = rgba[2] * 2.0f;

			// 将计算好的颜色值填充进 RDK 的通道中
			if (m_bPreview)
			{
				// 预览模式：填充一个马赛克块以提升感官速度
				for (int j = 0; j < mosaic; j++)
				{
					for (int i = 0; i < mosaic; i++)
					{
						const int px = pixelX + i, py = pixelY + j;
						m_pChanRGBA->SetValue(px, py, ComponentOrder::RGBA, &rgba);

						m_pChanZ->SetValue(px, py, ComponentOrder::Irrelevant, &z);

						if (nullptr != m_pChanNormalX)
							m_pChanNormalX->SetValue(px, py, ComponentOrder::Irrelevant, &nx);

						if (nullptr != m_pChanNormalY)
							m_pChanNormalY->SetValue(px, py, ComponentOrder::Irrelevant, &ny);

						if (nullptr != m_pChanNormalZ)
							m_pChanNormalZ->SetValue(px, py, ComponentOrder::Irrelevant, &nz);
					}
				}
			}
			else
			{
				// 标准模式：单像素填充
				m_pChanRGBA->SetValue(pixelX, pixelY, ComponentOrder::RGBA, &rgba);

				m_pChanZ->SetValue(pixelX, pixelY, ComponentOrder::Irrelevant, &z);

				if (nullptr != m_pChanNormalX)
					m_pChanNormalX->SetValue(pixelX, pixelY, ComponentOrder::Irrelevant, &nx);

				if (nullptr != m_pChanNormalY)
					m_pChanNormalY->SetValue(pixelX, pixelY, ComponentOrder::Irrelevant, &ny);

				if (nullptr != m_pChanNormalZ)
					m_pChanNormalZ->SetValue(pixelX, pixelY, ComponentOrder::Irrelevant, &nz);
			}
		}

		// 刷新视口中的这一行区域
		rect.top = pixelY;
		rect.bottom = pixelY + 1;
		GetRenderWindow().InvalidateArea(rect);

		// 更新进度条
		percent = (pixelY * 100) / height;
		GetRenderWindow().SetProgress(L"Rendering...", percent);

		// 调试用：如果不按 Ctrl 键，则人为放慢渲染速度
		if (0 == (::GetAsyncKeyState(VK_CONTROL) & 0x8000))
		{
			::Sleep(m_bPreview ? 10 : 100);
		}
	}

	return true;
}

// 后台计算线程的入口
int CSampleRdkRendererSdkRender::ThreadedRender(void)
{
	// 此函数在工作线程（Worker Thread）的上下文中执行。
	// TODO: 用你自己的渲染逻辑替换此处。

	m_bCancel = false;

	bool bSuccess = false;

	srand(int(time(nullptr)));

	m_Region = m_RectRender;

	// 打开 RGBA 通道，这是渲染的基础。
	m_pChanRGBA = GetRenderWindow().OpenChannel(IRhRdkRenderWindow::chanRGBA);

	// 可选：打开 Z-Buffer（相机距离）通道。
	m_pChanZ = GetRenderWindow().OpenChannel(IRhRdkRenderWindow::chanDistanceFromCamera);

	// 可选：打开法线分量通道。
	m_pChanNormalX = GetRenderWindow().OpenChannel(IRhRdkRenderWindow::chanNormalX);
	m_pChanNormalY = GetRenderWindow().OpenChannel(IRhRdkRenderWindow::chanNormalY);
	m_pChanNormalZ = GetRenderWindow().OpenChannel(IRhRdkRenderWindow::chanNormalZ);

	// 执行核心渲染逻辑
	bSuccess = RenderCore();

	// 渲染结束，关闭所有通道（必须与 Open 配对）
	if (nullptr != m_pChanNormalZ)
		m_pChanNormalZ->Close();

	if (nullptr != m_pChanNormalY)
		m_pChanNormalY->Close();

	if (nullptr != m_pChanNormalX)
		m_pChanNormalX->Close();

	if (nullptr != m_pChanZ)
		m_pChanZ->Close();

	if (nullptr != m_pChanRGBA)
		m_pChanRGBA->Close();

	GetRenderWindow().SetProgress(bSuccess ? L"Completed" : L"Failed", 100);

	// 通知主线程：模态框渲染流程已结束
	SetContinueModal(false);

	return 0;
}

void CSampleRdkRendererSdkRender::SetContinueModal(bool b)
{
	m_bContinueModal = b;
}

// 停止渲染：线程同步机制
void CSampleRdkRendererSdkRender::StopRendering()
{
	m_bCancel = true;

	// 主线程必须等待渲染线程结束，这一点至关重要。
	// 否则，该对象可能在渲染线程仍在使用它时就被销毁了。
	::WaitForSingleObject(m_hRenderThread, INFINITE);

	m_hRenderThread = NULL;
}

// 渲染前的准备工作：获取场景网格数据
bool CSampleRdkRendererSdkRender::SetUpRender(CRhinoView* pView, bool bQuiet)
{
	if (nullptr == pView)
		return false;

	if (!::RhRdkIsAvailable())
		return false;

	const ON_Viewport& vp = pView->ActiveViewport().VP();

	// 强制在主线程上创建渲染网格（Mesh）。
	auto* pIterator = NewRenderMeshIterator(vp, true, bQuiet);
	pIterator->EnsureRenderMeshesCreated();

	// 现在可以使用这个迭代器获取场景中的所有网格。
	// 迭代器存在期间，所有网格都被保证可用，无需在渲染过程中复制它们。

	CRhRdkRenderMesh rm;
	pIterator->Reset();
	while (pIterator->Next(rm))
	{
		// TODO: 使用网格数据。 
		// 这里可能是你创建 BVH 等空间加速结构的最佳时机。
		// 如果你的渲染器需要自己的网格表示方式，请在此处进行转换。

		/** 【Gemini 注解】：
		 在黑洞引力透镜演示中，如果你在场景里放了一个地球模型，
		 你就是在这里通过迭代器抓到地球的顶点和索引，
		 然后把它们喂给你的光线求交引擎，这样黑洞扭曲的背景里就会出现被遮挡的效果。
		*/
	}

	delete pIterator;

	return true;
}

// 渲染执行入口
CRhinoSdkRender::RenderReturnCodes CSampleRdkRendererSdkRender::Render(const ON_2iSize& sizeImage)
{
	if (!SetUpRender(RhinoApp().ActiveView(), false))
		return CRhinoSdkRender::render_error_starting_render;

	return __super::Render(sizeImage);
}

// 局部渲染窗口执行入口
CRhinoSdkRender::RenderReturnCodes CSampleRdkRendererSdkRender::RenderWindow(CRhinoView* pView, const LPRECT pRect, bool bInPopupWindow)
{
	if (!SetUpRender(pView, false))
		return CRhinoSdkRender::render_error_starting_render;

	CRhinoSdkRender::RenderReturnCodes rc;

	if (bInPopupWindow)
	{
		// 在独立的弹出窗口中渲染指定区域
		const CRect rect(pRect);
		const auto cs = rect.Size();
		const auto size = ON_2iSize(cs.cx, cs.cy);

		rc = __super::Render(size);
	}
	else
	{
		// 直接在视口（Viewport）中渲染
		rc = __super::RenderWindow(pView, pRect, bInPopupWindow);
	}

	return rc;
}

// 询问插件：是否需要重新处理几何体表
BOOL CSampleRdkRendererSdkRender::NeedToProcessGeometryTable()
{
	return ::SampleRdkRendererPlugIn().SceneChanged();
}

// 询问插件：是否需要重新处理灯光表
BOOL CSampleRdkRendererSdkRender::NeedToProcessLightTable()
{
	return ::SampleRdkRendererPlugIn().LightingChanged();
}

// 渲染窗口创建前的预处理
BOOL CSampleRdkRendererSdkRender::RenderPreCreateWindow()
{
	::SampleRdkRendererPlugIn().SetSceneChanged(FALSE);
	::SampleRdkRendererPlugIn().SetLightingChanged(FALSE);

	return TRUE;
}

// 询问：是否复用现有的渲染窗口
bool CSampleRdkRendererSdkRender::ReuseRenderWindow(void) const
{
	// 这通常可以设置为用户的可选项。
	return false;
}

// C 风格的线程函数代理
static int RenderThread(void* pv)
{
	return reinterpret_cast<CSampleRdkRendererSdkRender*>(pv)->ThreadedRender();
}

// 启动渲染：创建计算线程
void CSampleRdkRendererSdkRender::StartRendering()
{
	const auto* doc = CommandContext().Document();
	if (nullptr == doc)
		return;

	const auto size = RenderSize(*doc, true);
	m_RectRender = CRect(0, 0, size.cx, size.cy);

	// 创建 Win32 线程，开始异步渲染
	m_hRenderThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RenderThread, this, 0, NULL);
}

// 在指定视口区域启动渲染
BOOL CSampleRdkRendererSdkRender::StartRenderingInWindow(CRhinoView*, const LPCRECT rect)
{
	m_RectRender = rect;

	m_hRenderThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RenderThread, this, 0, NULL);

	return true;
}