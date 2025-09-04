#pragma once
#include <chrono>
#include <string>
#include <vector>

struct ProfileEvent { std::string name; double ms; };

class Profiler
{
public:
    void NewFrame() { lastFrame.clear(); }
    void Add(const std::string& name, double ms) { lastFrame.push_back({name, ms}); }
    const std::vector<ProfileEvent>& GetLastFrame() const { return lastFrame; }
private:
    std::vector<ProfileEvent> lastFrame;
};

struct ProfileScope
{
    ProfileScope(Profiler& p, const char* n) : prof(p), name(n), start(std::chrono::high_resolution_clock::now()) {}
    ~ProfileScope()
    {
        auto end = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - start).count();
        prof.Add(name, ms);
    }
    Profiler& prof; const char* name; std::chrono::high_resolution_clock::time_point start;
};

inline Profiler gProfiler;

