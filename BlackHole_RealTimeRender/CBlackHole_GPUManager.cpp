// CBlackHole_GPUManager.cpp
#include "stdafx.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "BlackHole_Kernel.h"
#include "CBlackHole_GPUManager.h"

bool CBlackHole_GPUManager::Initialize(int w, int h) {
    // 1. 不仅要检查资源是否存在，还要检查尺寸是否发生变化
    if (m_pDevice && m_pShader && m_pConstantBuffer && m_currentWidth == w && m_currentHeight == h) {
        return true;
    }

    // 更新当前记录的宽高
    m_currentWidth = w;
    m_currentHeight = h;

    // 2. 如果设备还没创建，才需要创建设备、着色器和常量缓冲
    if (!m_pDevice) {
        D3D_FEATURE_LEVEL fl;
        if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &m_pDevice, &fl, &m_pContext))) {
            return false;
        }
        // 将编译好的 HLSL 字节码部署到显存
        HRESULT hr = m_pDevice->CreateComputeShader(g_BlackHoleShader, sizeof(g_BlackHoleShader), nullptr, &m_pShader);
        if (FAILED(hr)) return false;

        // 创建常量缓冲
        D3D11_BUFFER_DESC cbDesc = { sizeof(GPU_Buffer_Data), D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0 };
        if (FAILED(m_pDevice->CreateBuffer(&cbDesc, nullptr, &m_pConstantBuffer))) return false;

        // 【临时代码】加载本地HDR星空贴图 
        int width, height, channels;
        float* data = stbi_loadf("D:\\Code\\CPP\\SJU RhinoBlackHole\\BlackHoleRealTimeRender\\BlackHole_RealTimeRender\\BlackHole_RealTimeRender\\res\\nebula-1.hdr", 
            & width, &height, &channels, 4);    // 强制转换成 RGBA 四通道
        if (data) {
            D3D11_TEXTURE2D_DESC texDescHDR = {};
            texDescHDR.Width = width;
            texDescHDR.Height = height;
            texDescHDR.MipLevels = 1;   // 原始超清图，不生成缩略图
            texDescHDR.ArraySize = 1;
            texDescHDR.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            texDescHDR.SampleDesc.Count = 1;
            texDescHDR.Usage = D3D11_USAGE_IMMUTABLE;   // 创建后不可修改
            texDescHDR.BindFlags = D3D11_BIND_SHADER_RESOURCE;

            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = data;
            initData.SysMemPitch = width * 4 * sizeof(float);

            ComPtr<ID3D11Texture2D> pHDRTex;
            m_pDevice->CreateTexture2D(&texDescHDR, &initData, &pHDRTex);
            m_pDevice->CreateShaderResourceView(pHDRTex.Get(), nullptr, &m_pSkyboxSRV);

            stbi_image_free(data); // 传进显存后，释放 CPU 内存
        }

        // 创建线性采样器 (允许首尾相接 WRAP)
        D3D11_SAMPLER_DESC sampDesc = {};
        sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;  // U方向允许环绕（经度转一圈无缝）
        sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP; // V方向夹紧（极点不重复）
        sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        m_pDevice->CreateSamplerState(&sampDesc, &m_pSkyboxSampler);
        // =========================================================
    }

    // 3. 清空旧的纹理资源
    m_pOutputTex.Reset();
    m_pUAV.Reset();
    m_pStagingTex.Reset();

    // 4. 根据新的宽(w)和高(h)创建纹理资源
    D3D11_TEXTURE2D_DESC texDesc = { (UINT)w, (UINT)h, 1, 1, DXGI_FORMAT_R32G32B32A32_FLOAT,
        {1,0}, D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS, 0, 0 };
    m_pDevice->CreateTexture2D(&texDesc, nullptr, &m_pOutputTex);
    m_pDevice->CreateUnorderedAccessView(m_pOutputTex.Get(), nullptr, &m_pUAV);

    texDesc.Usage = D3D11_USAGE_STAGING;
    texDesc.BindFlags = 0;
    texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    m_pDevice->CreateTexture2D(&texDesc, nullptr, &m_pStagingTex);

    return true;
}

// 将 CPU 端实时捕获的相机动态数据，同步给 GPU 的物理计算单元。
void CBlackHole_GPUManager::UpdateParams(const CameraParameters& cam, int w, int h) {
    // 1. 安全检查
    if (!m_pConstantBuffer || !m_pContext) return;

    // 2. 定义映射资源结构体：用于接收显存地址指针及数据排布信息
    D3D11_MAPPED_SUBRESOURCE ms;

    // 3. 显存映射
    // D3D11_MAP_WRITE_DISCARD 表示丢弃该缓冲区旧数据并申请新内存块。
    if (SUCCEEDED(m_pContext->Map(m_pConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms))) {

        // 4. 类型转换：将映射到的原始显存指针转换为我们定义的结构体指针 
        GPU_Buffer_Data* p = (GPU_Buffer_Data*)ms.pData;

        // 5. 数据填充：将 CPU 端的双精度相机参数同步为 GPU 端的单精度浮点数 
        p->camPos[0] = (float)cam.pos.x; p->camPos[1] = (float)cam.pos.y; p->camPos[2] = (float)cam.pos.z;
        p->camDir[0] = (float)cam.dir.x; p->camDir[1] = (float)cam.dir.y; p->camDir[2] = (float)cam.dir.z;
        p->camUp[0] = (float)cam.up.x;   p->camUp[1] = (float)cam.up.y;   p->camUp[2] = (float)cam.up.z;
        p->fov = (float)cam.viewAngle;
        p->width = (float)w;
        p->height = (float)h;
        // 写入质量和自旋
        p->mass = m_theBlackHole.getMass();
        p->spin = m_theBlackHole.getSpin();

        // 6. 解除映射：告知 GPU 数据更新完毕，重新交还缓冲区的访问权给显卡驱动 
        m_pContext->Unmap(m_pConstantBuffer.Get(), 0);
    }
}


void CBlackHole_GPUManager::Dispatch(int w, int h) {
    // 1. 状态绑定
    m_pContext->CSSetShader(m_pShader.Get(), nullptr, 0);
    m_pContext->CSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());

    if (m_pSkyboxSRV && m_pSkyboxSampler) {
        m_pContext->CSSetShaderResources(0, 1, m_pSkyboxSRV.GetAddressOf());
        m_pContext->CSSetSamplers(0, 1, m_pSkyboxSampler.GetAddressOf());
    }

    // 2. 输出通道
    m_pContext->CSSetUnorderedAccessViews(0, 1, m_pUAV.GetAddressOf(), nullptr);

    // 3. 激发并行
    m_pContext->Dispatch((w + 15) / 16, (h + 15) / 16, 1);

    // 4. 成果同步
    m_pContext->CopyResource(m_pStagingTex.Get(), m_pOutputTex.Get());
}

void* CBlackHole_GPUManager::MapResult(UINT& pitch) {
    // 1. 定义映射资源结构体
    D3D11_MAPPED_SUBRESOURCE ms;

    // 2. 锁定并映射资源
    if (SUCCEEDED(m_pContext->Map(m_pStagingTex.Get(), 0, D3D11_MAP_READ, 0, &ms))) {

        // 3. 传出行跨度并返回首地址
        pitch = ms.RowPitch;
        return ms.pData;
    }
    // 映射失败
    return nullptr;
}

void CBlackHole_GPUManager::UnmapResult() { m_pContext->Unmap(m_pStagingTex.Get(), 0); }