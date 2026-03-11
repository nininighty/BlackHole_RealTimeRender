#pragma once
#include "stdafx.h"
#include <string>
#include <mutex>
#include <thread>
#include <chrono>
#include <fstream>
#include <iostream>
#include <algorithm>

// 专门用于显卡传输的结构体，16 字节对齐
struct GPU_Buffer_Data {
    float camPos[3];    float maxSteps;      // 相机坐标 + 步数
    float camDir[3];    float time;      // 面朝方向+时间戳
    float camUp[3];     float fov;       // 上方向量 + fov，16字节
    float width;        float height;    float mass;  float spin; 
};

// 后台逻辑使用的相机参数
struct CameraParameters {
    ON_3dPoint  pos;
    ON_3dVector dir;
    ON_3dVector up;
    double      viewAngle;
};

// ===========================================
// 性能追踪部分 

// 用于存储单词测量的结果
struct ProfileResult {
    std::string Name;
    long long Start, End;
    uint32_t ThreadID;
};

// 性能数据记录器 (单例模式)
class Instrumentor {
private:
    std::ofstream m_OutputStream;
    int m_ProfileCount;
    std::mutex m_Lock; // 保证多线程写入时的安全

    Instrumentor() : m_ProfileCount(0) {}

public:
    static Instrumentor& Get() {
        static Instrumentor instance;
        return instance;
    }

    void BeginSession(const std::string& name, const std::string& filepath = "BlackHoleProfile.json") {
        m_OutputStream.open(filepath);
        WriteHeader();
    }

    void EndSession() {
        WriteFooter();
        m_OutputStream.close();
        m_ProfileCount = 0;
    }

    void WriteProfile(const ProfileResult& result) {
        std::lock_guard<std::mutex> lock(m_Lock);

        if (m_ProfileCount++ > 0)
            m_OutputStream << ",";

        std::string name = result.Name;
        std::replace(name.begin(), name.end(), '"', '\'');

        // 严格遵循 Chrome Tracing JSON 格式
        m_OutputStream << "{";
        m_OutputStream << "\"cat\":\"function\",";
        m_OutputStream << "\"dur\":" << (result.End - result.Start) << ",";
        m_OutputStream << "\"name\":\"" << name << "\",";
        m_OutputStream << "\"ph\":\"X\",";
        m_OutputStream << "\"pid\":0,";
        m_OutputStream << "\"tid\":" << result.ThreadID << ",";
        m_OutputStream << "\"ts\":" << result.Start;
        m_OutputStream << "}";
        m_OutputStream.flush();
    }

private:
    void WriteHeader() {
        m_OutputStream << "{\"otherData\": {},\"traceEvents\":[";
        m_OutputStream.flush();
    }

    void WriteFooter() {
        m_OutputStream << "]}";
        m_OutputStream.flush();
    }
};

// 作用域计时器 (利用 RAII 机制，构造时开始，析构时记录)
class ScopedTimer {
public:
    ScopedTimer(const std::string& name)
        : m_Name(name), m_Stopped(false) {
        m_StartTimepoint = std::chrono::high_resolution_clock::now();
    }

    ~ScopedTimer() {
        if (!m_Stopped)
            Stop();
    }

    void Stop() {
        auto endTimepoint = std::chrono::high_resolution_clock::now();

        // 转换为微秒 (Chrome Tracing 的标准时间单位)
        long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
        long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

        // 获取当前线程 ID，方便在 Chrome 里区分主线程和渲染线程
        uint32_t threadID = static_cast<uint32_t>(std::hash<std::thread::id>{}(std::this_thread::get_id()));

        Instrumentor::Get().WriteProfile({ m_Name, start, end, threadID });

        m_Stopped = true;
    }

private:
    std::string m_Name;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
    bool m_Stopped;
};

// 定义一个极简的宏，方便在代码里到处插桩
#define PROFILE_SCOPE(name) ScopedTimer timer##__LINE__(name)
#define PROFILE_FUNCTION() PROFILE_SCOPE(__FUNCTION__)