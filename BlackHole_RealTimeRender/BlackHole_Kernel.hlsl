// ==========================================
// 1. 缓冲区与资源绑定

cbuffer CameraBuffer : register(b0) {
    float3 camPos;
    float maxSteps;
    float3 camDir;
    float p2;
    float3 camUp;
    float fov;
    float2 res;
    float mass;
    float spin;
};

RWTexture2D<float4> OutputBuffer : register(u0);    // Rhino视窗在显存中的缓存区
Texture2D<float4> SkyboxTex : register(t0); // 背景素材
SamplerState SkyboxSampler : register(s0);  // 背景采样规则
static const float PI = 3.14159265;

// ==========================================
// 2. 物理核心：Kerr-Schild 精确测地线方程

// 计算当前位置的坐标导数与动量导数
void GetKerrSchildDerivatives(
float3 pos,     // 当前位置坐标
float3 p,       // 共轭动量空间动量
float M , float a,  // 黑洞参数 
out float3 dPos,    // 返回值：坐标导数 
out float3 dP)      // 返回值：动量导数
{
    float x = pos.x, y = pos.y, z = pos.z;
    float R2 = x * x + y * y + z * z;
    float a2 = a * a;
    
    // 求解 Kerr 坐标半径 r
    float r2 = 0.5 * (R2 - a2 + sqrt((R2 - a2) * (R2 - a2) + 4.0 * a2 * z * z));
    float r = sqrt(max(r2, 1e-8));
    float r3 = r2 * r;
    float r4 = r2 * r2;

    // 计算 Sigma_KS 辅助量
    float SigmaKS = (x * x + y * y) / ((r2 + a2) * (r2 + a2)) + (z * z) / max(r4, 1e-8);
    SigmaKS = max(SigmaKS, 1e-8);

    // 半径关于直角坐标的偏导 dr_i
    float dr_x = x / (r * (r2 + a2) * SigmaKS);
    float dr_y = y / (r * (r2 + a2) * SigmaKS);
    float dr_z = z / (r3 * SigmaKS);
    float3 dr_vec = float3(dr_x, dr_y, dr_z);

    // 计算标量函数 f 及其偏导 df_i
    float D = r4 + a2 * z * z;
    float D2 = D * D;
    float f = (2.0 * M * r3) / D;
    float df_dr = 2.0 * M * r2 * (3.0 * a2 * z * z - r4) / D2;
    float df_dz = -4.0 * M * a2 * r3 * z / D2;
    float3 df_vec = df_dr * dr_vec + float3(0, 0, df_dz);

    // 计算零矢 l_i
    float lx = (r * x + a * y) / (r2 + a2);
    float ly = (r * y - a * x) / (r2 + a2);
    float lz = z / r;
    float3 l_vec = float3(lx, ly, lz);

    // 计算零矢的偏导辅助项 B_i
    float Bx = (a2 * x - r2 * x - 2.0 * a * r * y) / ((r2 + a2) * (r2 + a2));
    float By = (a2 * y - r2 * y + 2.0 * a * r * x) / ((r2 + a2) * (r2 + a2));
    float Bz = -z / r2;

    // 缩并 p_alpha * \partial_\mu l^\alpha
    float term_x = (r * p.x - a * p.y) / (r2 + a2);
    float term_y = (a * p.x + r * p.y) / (r2 + a2);
    float term_z = p.z / r;
    float p_dot_B = p.x * Bx + p.y * By + p.z * Bz;
    float3 p_dl = float3(term_x, term_y, term_z) + p_dot_B * dr_vec;

    // 动量缩并 k = l^\alpha p_\alpha (由于能量 p_t = -1 守恒)
    float k = 1.0 + dot(l_vec, p);

    // 哈密顿运动方程：位置导数 dx/d\lambda
    dPos = p - f * k * l_vec;

    // 哈密顿运动方程：动量导数 dp/d\lambda
    dP = 0.5 * k * k * df_vec + f * k * p_dl;
}

// RK4 积分器
void StepRK4(inout float3 pos, inout float3 p_vec, float h_step, float M, float a)
{
    float3 k1_pos, k1_p;
    GetKerrSchildDerivatives(pos, p_vec, 
    M, a, k1_pos, k1_p);
    
    float3 k2_pos, k2_p;
    GetKerrSchildDerivatives(pos  +  0.5 * h_step * k1_pos   ,     p_vec + 0.5 * h_step * k1_p,
    M, a, k2_pos, k2_p);

    float3 k3_pos, k3_p;
    GetKerrSchildDerivatives(pos + 0.5 * h_step * k2_pos    ,   p_vec + 0.5 * h_step * k2_p, 
    M, a, k3_pos, k3_p);

    float3 k4_pos, k4_p;
    GetKerrSchildDerivatives(pos + h_step * k3_pos  ,  p_vec + h_step * k3_p,
    M, a, k4_pos, k4_p);

    pos += (h_step / 6.0) * (k1_pos + 2.0 * k2_pos + 2.0 * k3_pos + k4_pos);
    p_vec += (h_step / 6.0) * (k1_p + 2.0 * k2_p + 2.0 * k3_p + k4_p);
    // 这里绝对不能 normalize(p_vec)，弯曲时空动量大小不是1
}

// ==========================================
// 3. 主程序

[numthreads(16, 16, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    // 屏幕越界检测
    if (id.x >= (uint) res.x || id.y >= (uint) res.y)
        return;

    // 1. 相机射线生成
    float2 uv = (float2(id.xy) / res.xy) * 2.0 - 1.0;
    uv.y = -uv.y;

    float aspect = res.x / res.y;
    float3 f_dir = normalize(camDir);
    float3 r_axis = normalize(cross(f_dir, normalize(camUp)));
    float3 u_axis = cross(r_axis, f_dir);

    // 计算当前单个光子初始朝向
    float3 rayDir = normalize(f_dir + r_axis * uv.x * aspect * tan(fov * 0.5) + u_axis * uv.y * tan(fov * 0.5));

    // 2. 初始化物理状态
    float3 pos = camPos;
    float3 p_vec = rayDir; // 初始处处于平直时空，光子动量即为方向

    // 事件视界半径 rs (外视界)
    float rs = mass + sqrt(max(0.0, mass * mass - spin * spin));
    float esc = max(length(camPos) + 10.0, 50.0);
    bool hit = false;

    // 3. 追踪循环
    for (int i = 0; i < maxSteps; ++i)  {
        // RK4积分
        StepRK4(pos, p_vec, 0.15, mass, spin);

        // 每步计算最新的克尔半径 r 用于碰撞检测
        float R2 = pos.x * pos.x + pos.y * pos.y + pos.z * pos.z;
        float r2 = 0.5 * (R2 - spin * spin + sqrt((R2 - spin * spin) * (R2 - spin * spin) + 4.0 * spin * spin * pos.z * pos.z));

        // 碰撞视界或坠入奇点
        if (r2 < rs * rs || r2 < 0.05) {
            hit = true;
            break;
        }
        
        // 逃逸判定
        if (R2 > esc * esc)
            break;
    }

    // 4. 最终着色
    if (hit) {
        OutputBuffer[id.xy] = float4(0, 0, 0, 1);
    } else  {
        // 逃逸到平直时空后，重新将动量归一化转为方向向量进行采样
        float3 v = normalize(p_vec);
        float u_coord = 0.5 + atan2(v.y, v.x) / (2.0 * PI);
        float v_coord = 0.5 - asin(clamp(v.z, -0.99, 0.99)) / PI;
        OutputBuffer[id.xy] = SkyboxTex.SampleLevel(SkyboxSampler, float2(u_coord, v_coord), 0) * 1.2;
    }
}