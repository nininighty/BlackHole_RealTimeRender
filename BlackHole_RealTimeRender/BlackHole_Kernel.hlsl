// ==========================================
// 1. 缓冲区与资源绑定

cbuffer CameraBuffer : register(b0) {
    float3 camPos;
    float maxSteps;
    float3 camDir;
    float time;
    float3 camUp;
    float fov;
    float2 res;
    float mass;
    float spin;
    
    float3 spherePos;
    float sphereRadius;
    float orbitSpeed;
    float inclination;
    int hasSphere;
    int hasTexture;
    float3 sphereColor;
    float padding2;
};  

RWTexture2D<float4> OutputBuffer : register(u0); // Rhino视窗在显存中的缓存区
Texture2D<float4> SkyboxTex : register(t0); // 背景素材
Texture2D<float4> SphereTex : register(t1); // 绕行天体贴图
SamplerState SkyboxSampler : register(s0);  // 背景采样规则
static const float PI = 3.14159265;

// ==========================================
// 2. 物理核心

// 计算当前位置的坐标导数与动量导数
void GetKerrSchildDerivatives(
float3 pos, // 当前位置坐标
float3 p, // 共轭动量空间动量
float M, float a, // 黑洞参数 
out float3 dPos, // 返回值：坐标导数 
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
    GetKerrSchildDerivatives(pos + 0.5 * h_step * k1_pos, p_vec + 0.5 * h_step * k1_p,
    M, a, k2_pos, k2_p);

    float3 k3_pos, k3_p;
    GetKerrSchildDerivatives(pos + 0.5 * h_step * k2_pos, p_vec + 0.5 * h_step * k2_p,
    M, a, k3_pos, k3_p);

    float3 k4_pos, k4_p;
    GetKerrSchildDerivatives(pos + h_step * k3_pos, p_vec + h_step * k3_p,
    M, a, k4_pos, k4_p);

    pos += (h_step / 6.0) * (k1_pos + 2.0 * k2_pos + 2.0 * k3_pos + k4_pos);
    p_vec += (h_step / 6.0) * (k1_p + 2.0 * k2_p + 2.0 * k3_p + k4_p);
    // 这里绝对不能 normalize(p_vec)，弯曲时空动量大小不是1
}

// ==========================================
//  吸积盘视觉辅助函数与封装模块

float rand(float2 uv) {
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453123);
}

// 后期调色，不调色完全过曝
float3 ACESFilm(float3 x) {
    float a = 2.51f, b = 0.03f, c = 2.43f, d = 0.59f, e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

float noise(float2 p) {
    float2 i = floor(p), f = frac(p);
    float h1 = frac(sin(dot(i + float2(0, 0), float2(127.1, 311.7))) * 43758.5453);
    float h2 = frac(sin(dot(i + float2(1, 0), float2(127.1, 311.7))) * 43758.5453);
    float h3 = frac(sin(dot(i + float2(0, 1), float2(127.1, 311.7))) * 43758.5453);
    float h4 = frac(sin(dot(i + float2(1, 1), float2(127.1, 311.7))) * 43758.5453);
    // 平滑阶跃：埃尔米特插值多项式
    float2 u = f * f * (3.0 - 2.0 * f);
    return lerp(lerp(h1, h2, u.x), lerp(h3, h4, u.x), u.y);
}

float fbm(float2 p) {
    float v = 0.0, a = 0.5;
    for (int i = 0; i < 2; i++) {
        v += a * noise(p);
        p *= 2.0;
        a *= 0.5;
    }
    return v;
}

// 体积渲染气体吸积盘
void RenderAccretionDisk(
    float3 pos, float3 p_vec, float r, float h, float rs,
    float zBumpiness, float3 colorTint,
    inout float3 diskColor, inout float transmittance)
{
    // ==========================================
    // 【效果调优滑块】
    // ==========================================
    float CloudScale = 2.5 ; // [噪声频率] 
    float FadeSharpness = 1.5 ; // [边缘消散] 
    float DiskThickness = 70.0; // [垂直压缩] 
    
    float CoreGlowSize = 0.65; // [高亮门槛]
    float DopplerIntense = 4.0; // [多普勒强度]
    float EmissionPower = 20.0; // [全局曝光] 
    
    float RotationSpeed = 0.05; // [旋转速度倍率]
    float3 col_edge = float3(0.7, 0.04, 0.06); // [边缘颜色] 
    float3 col_mid = float3(4.5, 0.8, 0.1); // [中间颜色] 
    float3 col_core = float3(8.0, 6.5, 4.5); // [核心颜色] 
    // ==========================================

    // 1. 物理边界
    float innerR = 3.5 * mass; 
    float outerR = 18.0 * mass;
    // 检测光子是否处于吸积盘
    if (r <= innerR || r >= outerR + 5.0 * mass)
        return;

    float r_plane = sqrt(pos.x * pos.x + pos.y * pos.y);

    // 2. 动态开普勒旋转与坐标剪切
    float static_shear = -(10.0 * mass) / max(r_plane, 1e-5);
    float global_rotation = time * RotationSpeed;
    float total_rotation = static_shear - global_rotation;
    float s_shear = sin(total_rotation), c_shear = cos(total_rotation);
    
    float2 sheared_pos = float2(pos.x * c_shear - pos.y * s_shear, pos.x * s_shear + pos.y * c_shear) * CloudScale;

    // 3. 增强噪声层次 (FBM 叠加细节)
    float2 warp = sheared_pos + 1.2 * float2(fbm(sheared_pos), fbm(sheared_pos + float2(5.2, 1.3)));
    // 加入三层细节：Base + Micro + Nano
    float turb = fbm(warp) * 0.6 + fbm(warp * 3.0) * 0.3 + fbm(warp * 8.0) * 0.1;
    float turbulence = turb ;

    // 4. 密度与边缘
    float macroNoise = fbm(sheared_pos * 0.08);
    float asymmetricR = r + (macroNoise - 0.5) * 8.0 * mass;
    float falloff = saturate((outerR - asymmetricR) / (outerR - innerR));
    float radialDensity = pow(falloff, FadeSharpness);

    float z_perturb = pos.z + (turbulence - 0.5) * zBumpiness * mass * 0.4;
    float densityZ = exp(-z_perturb * z_perturb * DiskThickness);

    float density = densityZ * radialDensity * turbulence;
    // 内缘平滑开启，展示光线弯曲
   density *= smoothstep(innerR, innerR + 0.2 * mass, r);

    if (density <= 0.0001)
        return;

    // 5. 相对论效应
    float3 gas_vel = normalize(float3(-pos.y, pos.x, 0.0));
    float v_mag = clamp(sqrt(mass / max(r, 1e-5)), 0.0, 0.98);
    float g_SR = sqrt(1.0 - v_mag * v_mag) / (1.0 + dot(normalize(p_vec), gas_vel) * v_mag);    // 狭义相对论的多普勒效应
    float g_GR =  sqrt(max(0.01, 1.0 - rs / max(r, 1e-5))); // 广义相对论的引力红移效应
    float g_total = g_SR * g_GR;
    
    // 6. 电影色彩调色
    float colorWeight = radialDensity * g_total;

    float3 baseCol;
    if (colorWeight < CoreGlowSize) {
        // 在边缘到中间，引入一个非线性插值，让暗部细节更丰富，优化pow(x,1.5)
        float ratio = colorWeight / CoreGlowSize;
        baseCol = lerp(col_edge, col_mid, ratio * sqrt(ratio));
    } else {
        // 从浅金迅速过渡到冷白
        baseCol = lerp(col_mid, col_core, (colorWeight - CoreGlowSize) / (1.0 - CoreGlowSize));
    }
    
    // 7. 最终积分：增加散射分量
    float beaming = pow(g_total, DopplerIntense);
    // 让气体不仅自发光，还稍微带一点环境光
    float3 finalCol = (baseCol * colorTint);
    diskColor += transmittance * finalCol * (density * EmissionPower * beaming) * h;
    transmittance *= exp(-density * 2.0 * h);
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

    // 初始克尔半径，之后在循环里面由RK4积分修改
    float R2 = dot(pos, pos);
    float spin_sq = spin * spin;
    float temp_diff = R2 - spin_sq;
    float r2 = 0.5 * (temp_diff + sqrt(max(0.0, temp_diff * temp_diff + 4.0 * spin_sq * pos.z * pos.z)));
    float r_cur = sqrt(max(r2, 1e-8));
    
    // 预判光子的第一步步长 h_init
    float base_h = 0.04 * r_cur;
    float dynamic_max_h = max(0.25, abs(pos.z) * 0.15);
    float h_init = clamp(base_h, 0.003, dynamic_max_h);
    // 用第一步步长来进行 100% 覆盖的随机抖动
    pos += p_vec * (rand(uv) * h_init);
    
        // 初始化吸积盘渲染状态
    float3 diskColor = 0;
    float transmittance = 1.0;
    
    // 3. 追踪循环
    for (int i = 0; i < maxSteps; ++i)  {
        // 基于缓存的半径计算步长
        float base_h = 0.02 * r_cur;
        float dynamic_max_h = max(0.25, abs(pos.z) * 0.15);
        float h = clamp(base_h, 0.003, dynamic_max_h);

        // RK4积分
        StepRK4(pos , p_vec, h , mass , spin); 

        // 更新克尔半径
        R2 = dot(pos, pos);
        float spin_sq = spin * spin;
        float temp_diff = R2 - spin_sq;
        float r2 = 0.5 * (temp_diff + sqrt(max(0.0, temp_diff * temp_diff + 4.0 * spin_sq * pos.z * pos.z)));
        r_cur = sqrt(max(r2, 1e-8));
        
        // 只有当光线真正进入了吸积盘的附近时，才去执行昂贵的体积渲染
        float r_plane2 = pos.x * pos.x + pos.y * pos.y;
        float abs_z = abs(pos.z);
        
        // 预计算边界的平方值，连开方(sqrt)都省了
        float inner_bound = (3.5 * mass) * (3.5 * mass);
        float outer_bound = (23.0 * mass) * (23.0 * mass); 
        float max_disk_height = 2.0 * mass; 

        // 仅当光线在圆盘半径内，且高度贴近赤道面时，才渲染吸积盘
        if (r_plane2 > inner_bound && r_plane2 < outer_bound && abs_z < max_disk_height)  {
            RenderAccretionDisk(pos, p_vec, r_cur, h, rs, 0.05, float3(1.0, 1.0, 1.0), diskColor, transmittance);
        }
        
        // 碰撞视界或坠入奇点
        if (r2 < rs * rs || r2 < 0.05) {
            hit = true;
            break;
        }
        
        // 逃逸判定及吸积盘完全遮挡判定
        if (R2 > esc * esc || transmittance < 0.01) break;
        
        //  绕行天体解析求交与引力透镜贴图映射
        if (hasSphere == 1)
        {
            float distToSphere = length(pos - spherePos);
            if (distToSphere < sphereRadius)
            {
                hit = true;
        
                // 1. 计算原始法线 N
                float3 N = normalize(pos - spherePos);
                float3 albedo = sphereColor;
        
                // 2. 贴图采样：增加“潮汐锁定”坐标变换
                if (hasTexture == 1)
                {
                    // --- 核心变换逻辑 ---
                    // A. 计算球心指向黑洞中心的方向作为“前向”基准 (Forward)
                    float3 forward = normalize(-spherePos);
                    
                    // B. 构造一个局部坐标系。为了防止 Forward 与 Z 轴重合导致 cross 失败，增加微小偏移
                    float3 worldUp = abs(forward.z) < 0.99 ? float3(0, 0, 1) : float3(0, 1, 0);
                    float3 right = normalize(cross(worldUp, forward));
                    float3 up = cross(forward, right);
                    
                    // C. 将法线 N 变换到这个“始终面朝黑洞”的局部空间
                    // 这样 localN.y 对应 Forward (指向黑洞的方向)
                    float3 localN;
                    localN.x = dot(N, right);
                    localN.y = dot(N, forward);
                    localN.z = dot(N, up);

                    // D. 使用 localN 代替原始 N 进行 UV 映射
                    // 这样你的校徽中心 (U=0.5, V=0.5) 就会锁定在 localN.y 正方向上
                    float u_coord = 0.5 + atan2(localN.x, localN.y) / (2.0 * PI);
                    float v_coord = 0.5 - asin(clamp(localN.z, -0.99, 0.99)) / PI;

                    float3 rawTex = SphereTex.SampleLevel(SkyboxSampler, float2(u_coord, v_coord), 0).rgb;
                    albedo = pow(rawTex, 2.2);
                }
        
                // 3. 漫反射光照 (Lambertian Shading)
                float3 L = normalize(-pos);
                float diffuse = max(0.0, dot(N, L));
        
                // 混合光照：环境光增加到 0.1，让背光面更清晰
                float3 shadedColor = albedo * (diffuse * 1.5 + 0.1);

                // 4. 颜色累加与阻断
                diskColor += transmittance * shadedColor;
                transmittance = 0.0;
                break;
            }
        }
    }

    // 4. 最终着色
    if (hit) {
        // 应用色调映射输出吸积盘颜色
        OutputBuffer[id.xy] = float4(ACESFilm(diskColor), 1.0);
    } else  {
        // 逃逸到平直时空后，重新将动量归一化转为方向向量进行采样
        float3 v = normalize(p_vec);
        float u_coord = 0.5 + atan2(v.y, v.x) / (2.0 * PI);
        float v_coord = 0.5 - asin(clamp(v.z, -0.99, 0.99)) / PI;
        
        // 结合吸积盘颜色与背景星空
        float3 skyColor = SkyboxTex.SampleLevel(SkyboxSampler, float2(u_coord, v_coord), 0).rgb * 1.2;
        OutputBuffer[id.xy] = float4(ACESFilm(diskColor + transmittance * skyColor), 1.0);
    }
}
