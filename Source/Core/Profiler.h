#pragma once

#include <chrono>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>

struct ProfileResult {
    const char* Name;
    long long Start;
    long long End;
    uint32_t ThreadID;
};

struct ProfileSession {
    std::string Name;
};

class Profiler {
public:
    Profiler(const Profiler&) = delete;
    Profiler& operator=(const Profiler&) = delete;

    static Profiler& Get() {
        static Profiler instance;
        return instance;
    }

    void BeginSession(const std::string& name, const std::string& filepath = "results.json") {
        std::lock_guard<std::mutex> lock(m_Mutex);
        if (m_CurrentSession) {
            WriteFooter();
            m_OutputStream.close();
        }

        m_OutputStream.open(filepath);
        if (m_OutputStream.is_open()) {
            m_CurrentSession = new ProfileSession{ name };
            WriteHeader();
        }
    }

    void EndSession() {
        std::lock_guard<std::mutex> lock(m_Mutex);
        if (m_CurrentSession) {
            WriteFooter();
            m_OutputStream.close();
            delete m_CurrentSession;
            m_CurrentSession = nullptr;
            m_ProfileCount = 0;
        }
    }

    void WriteProfile(const ProfileResult& result) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        if (!m_CurrentSession) return;

        if (m_ProfileCount++ > 0)
            m_OutputStream << ',';

        // Escape the name string for JSON safety
        std::string name = result.Name;
        for (char& c : name) {
            if (c == '"') c = '\'';
        }

        m_OutputStream << '{'
            << "\"cat\":\"function\","
            << "\"dur\":" << (result.End - result.Start) << ','
            << "\"name\":\"" << name << "\","
            << "\"ph\":\"X\","
            << "\"pid\":0,"
            << "\"tid\":" << result.ThreadID << ','
            << "\"ts\":" << result.Start
            << '}';

        m_OutputStream.flush();
    }

private:
    Profiler() = default;
    ~Profiler() { EndSession(); }

    void WriteHeader() {
        m_OutputStream << "{\"otherData\":{},\"traceEvents\":[";
        m_OutputStream.flush();
    }
    void WriteFooter() {
        m_OutputStream << "]}";
        m_OutputStream.flush();
    }

    ProfileSession* m_CurrentSession{ nullptr };
    std::ofstream   m_OutputStream;
    std::mutex      m_Mutex;
    uint32_t        m_ProfileCount{ 0 };
};

class InstrumentationTimer {
public:
    explicit InstrumentationTimer(const char* name)
        : m_Name(name), m_Stopped(false)
    {
        m_StartTimepoint = std::chrono::steady_clock::now();
    }

    ~InstrumentationTimer() {
        if (!m_Stopped) {
            Stop();
        }
    }

    void Stop() {
        auto endTimepoint = std::chrono::steady_clock::now();

        long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint)
            .time_since_epoch().count();
        long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint)
            .time_since_epoch().count();

        uint32_t threadID = static_cast<uint32_t>(
            std::hash<std::thread::id>{}(std::this_thread::get_id()));

        Profiler::Get().WriteProfile({ m_Name, start, end, threadID });
        m_Stopped = true;
    }

private:
    const char* m_Name;
    std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
    bool m_Stopped;
};

#ifdef OGE_PROFILE
#if defined(_MSC_VER)
#define OGE_PROFILE_FUNC_SIG __FUNCSIG__
#else
#define OGE_PROFILE_FUNC_SIG __PRETTY_FUNCTION__
#endif

#define OGE_PROFILE_BEGIN_SESSION(name, file) ::Profiler::Get().BeginSession(name, file)
#define OGE_PROFILE_END_SESSION()             ::Profiler::Get().EndSession()
#define OGE_PROFILE_SCOPE(name)               ::InstrumentationTimer OGE_TIMER_##__LINE__(name)
#define OGE_PROFILE_FUNCTION()                OGE_PROFILE_SCOPE(OGE_PROFILE_FUNC_SIG)
#else
#define OGE_PROFILE_BEGIN_SESSION(name, file) ((void)0)
#define OGE_PROFILE_END_SESSION()             ((void)0)
#define OGE_PROFILE_SCOPE(name)               ((void)0)
#define OGE_PROFILE_FUNCTION()                ((void)0)
#endif
