#pragma once
#include <memory>
#include <string>

class LuaVM
{
public:
    LuaVM();
    ~LuaVM();
    bool Init();
    void Shutdown();
    bool RunFile(const std::string& path);

private:
    void* L{nullptr};
    void RegisterAPI();
};

extern std::unique_ptr<LuaVM> gLua;
