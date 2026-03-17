# 黑洞实时渲染插件

基于 Rhinoceros 3D 的实时黑洞渲染器。通过 DirectX 11 Compute Shader 在 GPU 上完成 Kerr 时空测地线积分与吸积盘体积渲染。

在Rhino中切换显示模式以实现实时渲染，目前暂时只能与一个rhino中的球体交互，可以选择一个球体作为绕行天体，读取该球体的材质进入渲染器并自动绕行。

------
Rhino8的插件成果位于 `\OutputPlugin` 文件夹内

**自定义指令**：

- `SetBlackHoleParams`：命令提示用户输入并动态应用黑洞质量与自旋参数。
- `SetBlackHoleOrbitSphere`：拾取场景球体，提取材质颜色与绝对贴图路径，设定轨道倾角注入渲染管线。

------

## 项目架构

整体分为四层：**Rhino 接入层 → 渲染协调层 → GPU 执行层 → Shader 物理层**。

### 1. Rhino 接入层（主线程）

负责与 Rhino 引擎深度对接，管理生命周期与 UI 交互。

- **`BlackHole_RealTimeRenderPlugIn`**：插件主入口。负责向 Rhino 注册 DisplayMode 工厂、绑定显示属性，在插件加载时执行初始化。
- **`CBlackHole_RealTimeDisplayMode`**：实现 RDK `Realtime::DisplayMode` 接口，作为 Rhino 视口与后台渲染器之间的桥梁。
  - `StartRenderer()`：视口激活时启动后台渲染线程。
  - `DrawOrLockRendererFrameBuffer()`：每帧由 Rhino 调用，将渲染结果从 RenderWindow 送入视口显示。
  - `UpdateCamera()`：响应视口交互，将最新相机参数同步给后台线程。

### 2. 渲染协调层（后台 CWinThread）

负责协调 CPU 逻辑与 GPU 调度，确保主线程 UI 不卡顿。

- **`CBlackHole_RealTimeRenderer`**：后台渲染线程的宿主。
  - 维护 30 FPS 渲染循环，通过 `m_bIsDirty` 脏标记实现按需渲染，场景静止时零 GPU 消耗。
  - 在 CPU 端每帧实时解算轨道天体的动力学位置（开普勒圆轨道 + 倾角旋转）。
  - 调用 GPUManager 完成一帧计算后，将数据推入 `IRhRdkRenderWindow`，触发 `SignalUpdate()` 通知 Rhino 刷新视口。

### 3. GPU 执行层

封装 DirectX 11 底层 API，对渲染线程暴露高层接口。

- **`CBlackHole_GPUManager`**：
  - `Initialize()`：按需创建 D3D11 设备、编译 Compute Shader 及 UAV 输出纹理，支持窗口尺寸缩放自适应。
  - `UpdateParams()`：安全映射并上传相机、物理量、天体数据至常量缓冲区 (Constant Buffer)。
  - `Dispatch()`：调度 Compute Shader，以 16×16 线程组覆盖全屏，利用 D3D11 Timestamp Query 采集纯 GPU 耗时。
  - `MapResult()` / `UnmapResult()`：实现三重缓冲 (Triple Buffering)机制管理 Staging Texture，轮转读写以隐藏 GPU→CPU 的总线回传延迟。
  - `LoadSphereTexture()`：动态监听贴图路径变化，按需加载并生成 `ID3D11ShaderResourceView`。

### 4. Shader 物理层（GPU 并行）

每一像素分配独立线程，执行纯粹的物理与数学计算。

- **`BlackHole_Kernel.hlsl`**：核心光线追踪逻辑 `[numthreads(16,16,1)]`。
  - `GetKerrSchildDerivatives()`：计算 Kerr–Schild 坐标系下测地线方程的右端项（位置与动量导数）。
  - `StepRK4()`：四阶 Runge-Kutta 单步积分，基于距黑洞引力中心的距离自适应调整步长。
  - `RenderAccretionDisk()`：吸积盘体积渲染。采用 FBM 湍流生成云气密度场，结合 Beer-Lambert 定律累积透射率，并叠加多普勒频移与引力红移的相对论色彩偏移。
  - `CSMain()`：主入口函数。发射光线驱动 RK4 积分，处理事件视界吸收、天体遮挡碰撞与引力透镜下的天空盒采样。

------

## 线程与同步

| 线程               | 职责                                                |
| ------------------ | --------------------------------------------------- |
| 主线程（Rhino UI） | 响应视口事件，调用 `UpdateCamera()`，读取帧缓冲送显 |
| 后台渲染线程       | 轮询脏标记，驱动 GPU，将渲染结果写回 RenderWindow   |

三把互斥锁：`m_camMutex`（相机参数）、`m_sphereMutex`（天体参数）、`m_bufferMutex`（帧缓冲读写互斥，防止送显时后台同时写入）。
