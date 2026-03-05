// ==========================================
// 1. 缓冲区与资源绑定

cbuffer CameraBuffer : register(b0)
{
    float3 camPos;
    float pad1;
    float3 camDir;
    float pad2;
    float3 camUp;
    float fov;
    float2 resolution;
    float mass; 
    float spin; 
};

RWTexture2D<float4> OutputBuffer : register(u0);

// HDR 资源
Texture2D<float4> SkyboxTex : register(t0);
SamplerState SkyboxSampler : register(s0);

static const float PI = 3.14159265359;

// ==========================================
// 2. 物理引擎：广义相对论测地线

// 计算施瓦西度规下的加速度
float3 GetSchwarzschildAcceleration(float3 pos, float3 vel, float mass)
{
    float r2 = dot(pos, pos);
    float r = sqrt(r2);
    float r5 = r2 * r2 * r;
    
    if (r5 < 0.0001)
        return float3(0, 0, 0);

    float3 h = cross(pos, vel);
    float h2 = dot(h, h);

    return -(3.0 * mass * h2 / r5) * pos;
}

// RK4 积分器
void StepRK4(inout float3 pos, inout float3 vel, float h_step, float mass)
{
    float3 kr1 = vel;
    float3 kv1 = GetSchwarzschildAcceleration(pos, vel, mass);

    float3 r2 = pos + 0.5 * h_step * kr1;
    float3 v2 = vel + 0.5 * h_step * kv1;
    float3 kr2 = v2;
    float3 kv2 = GetSchwarzschildAcceleration(r2, v2, mass);

    float3 r3 = pos + 0.5 * h_step * kr2;
    float3 v3 = vel + 0.5 * h_step * kv2;
    float3 kr3 = v3;
    float3 kv3 = GetSchwarzschildAcceleration(r3, v3, mass);

    float3 r4 = pos + h_step * kr3;
    float3 v4 = vel + h_step * kv3;
    float3 kr4 = v4;
    float3 kv4 = GetSchwarzschildAcceleration(r4, v4, mass);

    pos += (h_step / 6.0) * (kr1 + 2.0 * kr2 + 2.0 * kr3 + kr4);
    vel += (h_step / 6.0) * (kv1 + 2.0 * kv2 + 2.0 * kv3 + kv4);
}

// ==========================================
// 3. 主渲染管线：光线追踪

[numthreads(16, 16, 1)]
void CSMain(uint3 id : SV_DispatchThreadID) {
    if (id.x >= (uint) resolution.x || id.y >= (uint) resolution.y)
        return;

    // --- 1. 相机射线初始化 ---
    float2 uv = float2(id.xy) / resolution.xy;
    uv = uv * 2.0 - 1.0;
    uv.y = -uv.y;

    float aspect = resolution.x / resolution.y;
    float3 forward = normalize(camDir);
    float3 up = normalize(camUp);
    float3 right = normalize(cross(forward, up));
    up = cross(right, forward);

    float halfFovTan = tan(fov * 0.5);
    float3 rayDir = normalize(forward + right * uv.x * aspect * halfFovTan + up * uv.y * halfFovTan);

    // --- 2. 物理状态初始化 ---
    float3 pos = camPos;
    float3 vel = rayDir;
    
    float rs = 2.0 * mass;
    
    // 调整项：增大步数和步长，让光线能跑大约 100 个单位！
    int maxSteps = 2000;
    float h_step = 0.1;

    float escapeRadius = max(length(camPos) + 10.0, 30.0);
    
    // 标记是否掉进黑洞
    bool isCaptured = false;

    // --- 3. Raymarching 主循环 ---
    for (int i = 0; i < maxSteps; ++i) {
        StepRK4(pos, vel, h_step, mass);
        float r = length(pos);

        // 条件 A：撞击视界，掉入黑洞
        if (r < rs) {
            isCaptured = true;
            break; // 结束这根光线的计算
        }

        // 条件 B：逃逸到宇宙
        if (r > escapeRadius) {
            break; // 逃逸了，去采样星空
        }
    }

    // --- 4. 结算最终颜色 ---
    if (isCaptured) {
        // 只有被黑洞彻底吞噬，才是纯黑色
        OutputBuffer[id.xy] = float4(0.0, 0.0, 0.0, 1.0);
    }
    else  {
        // 只要没被吞噬都根据光线现在的方向去采样星空
        float3 outDir = normalize(vel);
        
        float u = 0.5 + atan2(outDir.y, outDir.x) / (2.0 * PI);
        float v = 0.5 - asin(outDir.z) / PI;
        
        float4 skyColor = SkyboxTex.SampleLevel(SkyboxSampler, float2(u, v), 0);
        OutputBuffer[id.xy] = skyColor*1.2;
    }
}