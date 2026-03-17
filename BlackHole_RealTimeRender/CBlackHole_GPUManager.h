#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <d3dcompiler.h>
#include "CBlackHole_Common.h"
#include "CBlackHole_TheBlackHole.h"

using Microsoft::WRL::ComPtr;

class CBlackHole_GPUManager {
public:
    bool Initialize(int w, int h);

    void UpdateParams(const CameraParameters& cam, 
        float mass, float spin, float time, int w, int h, 
        const OrbitSphereParameters& sphereParams,
        const ON_3dPoint& currentSpherePos);   
    double Dispatch(int w, int h);

    void* MapResult(UINT& rowPitch);
    void UnmapResult();
    void Release();

private:
    TheBlackHole m_theBlackHole;
    int m_MaxSteps = 800; // 步进函数最大步长

    ComPtr<ID3D11ShaderResourceView> m_pSkyboxSRV;   // HDR 纹理资源视图
    ComPtr<ID3D11SamplerState>       m_pSkyboxSampler; // 纹理采样器
    ComPtr<ID3D11ShaderResourceView> m_pSphereSRV;  // 天体贴图
    std::wstring m_currentSphereTexPath = L""; // 记录当前加载的贴图路径

    bool LoadSphereTexture(const std::wstring& path); // 动态加载贴图的内部函数

    //  当前视窗宽高，用于判断是否需要重建纹理
    int m_currentWidth = 0;
    int m_currentHeight = 0;

    // 缓冲组
    std::vector<Microsoft::WRL::ComPtr<ID3D11Texture2D>> m_pStagingTextures;
    int m_writeIndex = 0;
    int m_frameCount = 0;
    int    m_lastReadIndex = -1;     // MapResult 记录本次读取的索引，供 UnmapResult 使用
    double m_lastGpuTimeMs = 0.0;   // 异步延迟一帧的 GPU 计时缓存
    bool   m_queryInFlight = false;  // 标记当前是否有一组 Query 正在等待 GPU 填写

    ComPtr<ID3D11Device>            m_pDevice;      // 虚拟设备接口
    ComPtr<ID3D11DeviceContext>     m_pContext;     // 设备上下文接口
    ComPtr<ID3D11ComputeShader>     m_pShader;    // 计算着色器对象
    ComPtr<ID3D11Buffer>            m_pConstantBuffer;  // 常量缓冲区
    ComPtr<ID3D11Texture2D>         m_pOutputTex;   // 二维纹理资源
    ComPtr<ID3D11UnorderedAccessView> m_pUAV;   // 无序访问视图
    ComPtr<ID3D11Texture2D>         m_pStagingTex;  // 暂存纹理资源

    // 性能检测
    ComPtr<ID3D11Query> m_pQueryDisjoint; // 频率与连续性查询
    ComPtr<ID3D11Query> m_pQueryStart;    // 起点时间戳
    ComPtr<ID3D11Query> m_pQueryEnd;      // 终点时间戳
};