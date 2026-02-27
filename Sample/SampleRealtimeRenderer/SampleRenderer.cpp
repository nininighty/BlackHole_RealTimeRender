/////////////////////////////////////////////////////////////////////////////
// SampleRenderer.cpp
//

#include "stdafx.h"
#include "SampleRenderer.h"


CSampleRenderer::CSampleRenderer(RhRdk::Realtime::ISignalUpdate* pSignalUpdateInterface)
	: m_pRenderThread(0),
	m_pSignalUpdateInterface(pSignalUpdateInterface)
{
	// 初始化为空闲状态
	m_bRunning = false;
}

CSampleRenderer::~CSampleRenderer()
{
	// 确保渲染进程已经停止
	StopRenderProcess();
}

bool CSampleRenderer::StartRenderProcess(const ON_2iSize& frameSize)
{
	// 如果渲染窗口尚未创建，则创建一个
	if (nullptr == m_pRenderWnd)
	{
		m_pRenderWnd = IRhRdkRenderWindow::New();
		if (nullptr == m_pRenderWnd)
			return false;
	}

	// 设置渲染窗口尺寸
	m_pRenderWnd->SetSize(frameSize);

	// 确保创建 DIB（设备无关位图）缓冲区
	if (!m_pRenderWnd->EnsureDib())
	{
		delete m_pRenderWnd;
		m_pRenderWnd = nullptr;
		return false;
	}

	// 如果渲染线程不存在，则创建并启动
	if (0 == m_pRenderThread)
	{
		// 设置运行标志为 true
		m_bRunning = true;

		// 以挂起状态创建线程，并将 this 指针传入
		m_pRenderThread = AfxBeginThread(RenderProcess, (void*)this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, 0);

		// 线程对象由我们自己销毁
		m_pRenderThread->m_bAutoDelete = FALSE;

		// 启动线程
		m_pRenderThread->ResumeThread();
	}

	return true;
}

void CSampleRenderer::StopRenderProcess()
{
	// 如果线程存在，则停止渲染
	if (0 != m_pRenderThread)
	{
		// 设置运行标志为 false，通知线程退出
		m_bRunning = false;

		// 等待线程检测到标志并退出
		WaitForSingleObject(m_pRenderThread->m_hThread, INFINITE);

		// 删除线程对象
		delete m_pRenderThread;

		// 置空指针，避免重复停止
		m_pRenderThread = 0;
	}

	// 删除渲染窗口
	delete m_pRenderWnd;
	m_pRenderWnd = nullptr;
}

bool CSampleRenderer::Running() const
{
	// 返回当前是否仍在运行
	return m_bRunning;
}

// 渲染线程执行该静态函数，传入参数为当前渲染器对象指针
unsigned int CSampleRenderer::RenderProcess(void* pData)
{
	// 将 void 指针转换为 CSampleRenderer*，并检查是否为空
	CSampleRenderer* pRenderProcess = static_cast<CSampleRenderer*>(pData);
	if (0 == pRenderProcess)
		return 0;

	const ON_2iSize sz = pRenderProcess->m_pRenderWnd->Size();

	// 开始渲染，并定期更新渲染图像
	int iScanline = -1;

	while (true)
	{
		// 检查是否需要停止
		if (!pRenderProcess->Running())
			break;

		// 锁定渲染图像通道以便更新
		// 检查是否为空，并渲染一条扫描线
		IRhRdkRenderWindow::IChannel* pChannel =
			pRenderProcess->m_pRenderWnd->OpenChannel(IRhRdkRenderWindow::chanRGBA);

		if (nullptr != pChannel)
		{
			// 初始化或重置扫描线索引
			if (iScanline == -1 || iScanline >= sz.cy)
				iScanline = sz.cy - 1;

			const int py = iScanline;

			// 遍历当前扫描线的每个像素
			for (int px = 0; px < sz.cx; px++)
			{
				// 生成随机颜色
				const float red = (float)(rand() % 256) / 255.0f;
				const float green = (float)(rand() % 256) / 255.0f;
				const float blue = (float)(rand() % 256) / 255.0f;

				float fColor[4];
				fColor[0] = red;
				fColor[1] = green;
				fColor[2] = blue;
				fColor[3] = 1.0f; // 不透明

				// 设置当前像素颜色
				pChannel->SetValue(px, py, ComponentOrder::RGBA, fColor);
			}

			// 关闭通道（解锁）
			pChannel->Close();

			// 通知需要重绘
			if (nullptr != pRenderProcess->m_pSignalUpdateInterface)
				pRenderProcess->m_pSignalUpdateInterface->SignalUpdate();
		}

		iScanline--;

		// 休眠约 50 毫秒
		Sleep(50);
	}

	return 0;
}