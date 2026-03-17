// CBlackHole_GPUManager.cpp
#include "stdafx.h"
#define STB_IMAGE_IMPLEMENTATION
#include "Resource.h"
#include "stb_image.h"
#include "BlackHole_Kernel.h"
#include "CBlackHole_GPUManager.h"

#pragma comment(lib, "d3d11.lib")

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

        // 内存直读 HDR 星空贴图
        int width = 0, height = 0, channels = 0;
        float* data = nullptr;

        // 通过静态局部变量的内存地址，精准反查本插件 DLL 的真实句柄
        HMODULE hInst = NULL;
        static int s_dummy = 0; // 声明一个哑变量，它永远驻留在当前 DLL 的数据段中
        ::GetModuleHandleEx(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCWSTR>(&s_dummy),
            &hInst
        );

        // 查找资源，RT_RCDATA 表示这块资源是纯数据。
        HRSRC hResInfo = ::FindResource(hInst, MAKEINTRESOURCE(IDR_HDR_IMAGE1), L"HDR_IMAGE");

        if (hResInfo == nullptr) {
            RhinoApp().Print(L"【HDR】FindResource 失败，资源找不到\n");
        }
        else {
            HGLOBAL hResData = ::LoadResource(hInst, hResInfo);
            DWORD dataSize = ::SizeofResource(hInst, hResInfo);
            const unsigned char* pData = static_cast<const unsigned char*>(::LockResource(hResData));

            wchar_t msg[256];
            swprintf_s(msg, L"【HDR】资源找到，大小: %d bytes\n", dataSize);
            RhinoApp().Print(msg);

            if (pData != nullptr && dataSize > 0) {
                data = stbi_loadf_from_memory(pData, static_cast<int>(dataSize), &width, &height, &channels, 4);
                if (data == nullptr) {
                    RhinoApp().Print(L"【HDR】stbi 解码失败\n");
                }
                else {
                    swprintf_s(msg, L"【HDR】解码成功，尺寸: %d x %d\n", width, height);
                    RhinoApp().Print(msg);
                }
            }
        }

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

    // GPU时间戳查询器
    if (!m_pQueryDisjoint) {
        D3D11_QUERY_DESC qDesc = {};
        qDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
        m_pDevice->CreateQuery(&qDesc, &m_pQueryDisjoint);

        qDesc.Query = D3D11_QUERY_TIMESTAMP;
        m_pDevice->CreateQuery(&qDesc, &m_pQueryStart);
        m_pDevice->CreateQuery(&qDesc, &m_pQueryEnd);
    }

    // 4. 根据新的宽(w)和高(h)创建纹理资源
    D3D11_TEXTURE2D_DESC texDesc = { (UINT)w, (UINT)h, 1, 1, DXGI_FORMAT_R32G32B32A32_FLOAT,
        {1,0}, D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS, 0, 0 };
    m_pDevice->CreateTexture2D(&texDesc, nullptr, &m_pOutputTex);
    m_pDevice->CreateUnorderedAccessView(m_pOutputTex.Get(), nullptr, &m_pUAV);

    texDesc.Usage = D3D11_USAGE_STAGING;
    texDesc.BindFlags = 0;
    texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    // 清理旧缓冲，重新分配3个暂存纹理
    m_pStagingTextures.clear();
    for (int i = 0; i < 3; ++i) {
        Microsoft::WRL::ComPtr<ID3D11Texture2D> stagingTex;
        m_pDevice->CreateTexture2D(&texDesc, nullptr, &stagingTex);
        m_pStagingTextures.push_back(stagingTex);
    }
    m_writeIndex = 0;
    m_frameCount = 0;

    return true;
}

// 将 CPU 端实时捕获的相机动态数据，同步给 GPU 的物理计算单元。
void CBlackHole_GPUManager::UpdateParams(
    const CameraParameters& cam, 
    float mass, float spin, float time, int w, int h, 
    const OrbitSphereParameters& sphereParams, 
    const ON_3dPoint& currentSpherePos)
{
    PROFILE_SCOPE("GPU_UpdateParams");
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
        p->maxSteps = m_MaxSteps;
        p->width = (float)w;
        p->height = (float)h;
        p->time = time;

        // 写入质量和自旋
        m_theBlackHole.set(mass, mass * spin);
        p->mass = m_theBlackHole.getMass();
        p->spin = - m_theBlackHole.getSpin();

        // 写入天体数据
        const wchar_t* rawPath = sphereParams.texturePath.Array();
        std::wstring newTexPath = rawPath ? rawPath : L""; // 拦截空指针，如果是 nullptr 则赋予空字符串 L""

        if (sphereParams.active && newTexPath != m_currentSphereTexPath) {
            LoadSphereTexture(newTexPath);
            m_currentSphereTexPath = newTexPath;
        }
        p->spherePos[0] = static_cast<float>(currentSpherePos.x);
        p->spherePos[1] = static_cast<float>(currentSpherePos.y);
        p->spherePos[2] = static_cast<float>(currentSpherePos.z);
        p->sphereRadius = static_cast<float>(sphereParams.radius);
        p->orbitSpeed = static_cast<float>(sphereParams.orbitSpeed);
        p->inclination = static_cast<float>(sphereParams.inclination);

        p->hasSphere = sphereParams.active ? 1 : 0;
        p->hasTexture = (m_pSphereSRV != nullptr) ? 1 : 0; // 如果贴图生成成功，告诉显卡用贴图

        // 将 ON_Color (0-255) 转换为 (0.0-1.0) 传给着色器
        p->sphereColor[0] = static_cast<float>(sphereParams.baseColor.FractionRed());
        p->sphereColor[1] = static_cast<float>(sphereParams.baseColor.FractionGreen());
        p->sphereColor[2] = static_cast<float>(sphereParams.baseColor.FractionBlue());
        p->padding2 = 0.0f;

        // 6. 解除映射：告知 GPU 数据更新完毕，重新交还缓冲区的访问权给显卡驱动 
        m_pContext->Unmap(m_pConstantBuffer.Get(), 0);
    }
}


double CBlackHole_GPUManager::Dispatch(int w, int h) {
    PROFILE_SCOPE("GPU_Dispatch_CPU");
    // 在开启新一轮的 Query 覆盖记录前，先读取上一帧的耗时
    if (m_frameCount > 0 && m_pQueryDisjoint) {
        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT tsDisjoint;
        if (m_pContext->GetData(m_pQueryDisjoint.Get(), &tsDisjoint, sizeof(tsDisjoint), 0) == S_OK) {
            if (!tsDisjoint.Disjoint) {
                UINT64 tsStart = 0, tsEnd = 0;
                if (m_pContext->GetData(m_pQueryStart.Get(), &tsStart, sizeof(tsStart), 0) == S_OK &&
                    m_pContext->GetData(m_pQueryEnd.Get(), &tsEnd, sizeof(tsEnd), 0) == S_OK) {
                    m_lastGpuTimeMs = double(tsEnd - tsStart) / double(tsDisjoint.Frequency) * 1000.0;
                }
            }
        }
    }

    // 1. 绑定资源
    m_pContext->CSSetShader(m_pShader.Get(), nullptr, 0);
    m_pContext->CSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());
    m_pContext->CSSetUnorderedAccessViews(0, 1, m_pUAV.GetAddressOf(), nullptr);
    if (m_pSkyboxSRV) m_pContext->CSSetShaderResources(0, 1, m_pSkyboxSRV.GetAddressOf());
    if (m_pSphereSRV) m_pContext->CSSetShaderResources(1, 1, m_pSphereSRV.GetAddressOf());
    if (m_pSkyboxSampler) m_pContext->CSSetSamplers(0, 1, m_pSkyboxSampler.GetAddressOf());

    // 2. 性能打点
    if (m_pQueryDisjoint) {
        m_pContext->Begin(m_pQueryDisjoint.Get());
        m_pContext->End(m_pQueryStart.Get());
    }

    // 3. 并行计算
    m_pContext->Dispatch((w + 15) / 16, (h + 15) / 16, 1);

    // 4. 结束打点并拷贝结果
    if (m_pQueryDisjoint) {
        m_pContext->End(m_pQueryEnd.Get());
        m_pContext->End(m_pQueryDisjoint.Get());
    }

    // 拷贝到当前写的暂存缓冲，并移动指针
    {
        PROFILE_SCOPE("GPU_CopyResource");

        m_pContext->CopyResource(
            m_pStagingTextures[m_writeIndex].Get(),
            m_pOutputTex.Get());
    }
    m_writeIndex = (m_writeIndex + 1) % 3;
    m_frameCount++;

    // 直接返回缓存的上一帧时间即可
    return m_lastGpuTimeMs;
}

void* CBlackHole_GPUManager::MapResult(UINT& pitch) {
    PROFILE_SCOPE("GPU_MapResult");
    // 至少跑了3帧再开始读，防止启动时读到空资源
    if (m_frameCount < 3) return nullptr;

    // 读取刚刚 Dispatch 完并 CopyResource 进去的那一帧
    int readIndex = (m_writeIndex + 2) % 3;
    m_lastReadIndex = readIndex;

    D3D11_MAPPED_SUBRESOURCE ms;
    // GPU 不算完，CPU 就在这睡着
    HRESULT hr = m_pContext->Map(m_pStagingTextures[readIndex].Get(), 0, D3D11_MAP_READ, 0, &ms);

    if (hr == S_OK) {
        pitch = ms.RowPitch;
        return ms.pData;
    }
    return nullptr;
}

void CBlackHole_GPUManager::UnmapResult() {
    // 解绑刚才读的那一帧
    int readIndex = m_lastReadIndex;
    m_pContext->Unmap(m_pStagingTextures[readIndex].Get(), 0);
}

// --- 新增：动态读取 Rhino 传来的贴图路径 ---
bool CBlackHole_GPUManager::LoadSphereTexture(const std::wstring& path) {
    if (!m_pDevice) return false;

    // 清空旧的贴图资源
    m_pSphereSRV.Reset();
    if (path.empty()) return false;

    // 将宽字符的 Rhino 路径转换为 UTF-8，stb_image 才能正确读取中文路径
    ON_String utf8Path(path.c_str());

    int s_width, s_height, s_channels;
    // 使用 stbi_load (非 f)，因为 Rhino 里的大部分贴图是 jpg/png，不需要 hdr
    unsigned char* s_data = stbi_load(utf8Path.Array(), &s_width, &s_height, &s_channels, 4);

    if (s_data) {
        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = s_width;
        texDesc.Height = s_height;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        // 普通 8-bit 图片用 UNORM 格式
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texDesc.SampleDesc.Count = 1;
        texDesc.Usage = D3D11_USAGE_IMMUTABLE;
        texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = s_data;
        initData.SysMemPitch = s_width * 4; // 每个像素 4 字节 (RGBA)

        ComPtr<ID3D11Texture2D> pTex;
        if (SUCCEEDED(m_pDevice->CreateTexture2D(&texDesc, &initData, &pTex))) {
            m_pDevice->CreateShaderResourceView(pTex.Get(), nullptr, &m_pSphereSRV);
        }
        stbi_image_free(s_data);
        return true;
    }
    return false;
}