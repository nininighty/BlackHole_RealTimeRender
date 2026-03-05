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
    void UpdateParams(const CameraParameters& cam, int w, int h);
    void Dispatch(int w, int h);
    void* MapResult(UINT& rowPitch);
    void UnmapResult();
    void Release();

private:
    TheBlackHole m_theBlackHole;

    ComPtr<ID3D11ShaderResourceView> m_pSkyboxSRV;   // HDR 纹理资源视图
    ComPtr<ID3D11SamplerState>       m_pSkyboxSampler; // 纹理采样器

    //  当前视窗宽高，用于判断是否需要重建纹理
    int m_currentWidth = 0;
    int m_currentHeight = 0;

    ComPtr<ID3D11Device>            m_pDevice;      // 虚拟设备接口
    ComPtr<ID3D11DeviceContext>     m_pContext;     // 设备上下文接口
    ComPtr<ID3D11ComputeShader>     m_pShader;    // 计算着色器对象
    ComPtr<ID3D11Buffer>            m_pConstantBuffer;  // 常量缓冲区
    ComPtr<ID3D11Texture2D>         m_pOutputTex;   // 二维纹理资源
    ComPtr<ID3D11UnorderedAccessView> m_pUAV;   // 无序访问视图
    ComPtr<ID3D11Texture2D>         m_pStagingTex;  // 暂存纹理资源
};