#pragma once

// 该类创建一个渲染线程，在后台处理渲染任务

class CSampleRenderer
{
public:
	// 构造函数，可传入用于通知界面更新的接口指针
	CSampleRenderer(RhRdk::Realtime::ISignalUpdate* pSignalUpdateInterface = nullptr);
	virtual ~CSampleRenderer();

	// 控制渲染流程
	bool StartRenderProcess(const ON_2iSize& frameSize);
	void StopRenderProcess();

	// 判断渲染器是否正在运行或应当继续运行
	bool Running() const;

	// 获取渲染窗口对象
	IRhRdkRenderWindow* RenderWindow() const { return m_pRenderWnd; }

private:
	// 作为线程执行的静态函数
	// pData 为指向当前类实例的指针
	static unsigned int RenderProcess(void* pData);

	// 运行状态标志，由 Running() 使用
	bool m_bRunning;

	// 指向线程对象的指针
	CWinThread* m_pRenderThread;

	// 用于存储渲染图像的 DIB（设备无关位图），线程安全
	IRhRdkRenderWindow* m_pRenderWnd = nullptr;

	// 用于通知显示管线进行重绘
	RhRdk::Realtime::ISignalUpdate* m_pSignalUpdateInterface = nullptr;
};