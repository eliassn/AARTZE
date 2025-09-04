#include "LuaVM.hpp"
#include "core/Coordinator.hpp"
#include "components/TransformComponent.hpp"
#include "editor/EditorActions.hpp"

std::unique_ptr<LuaVM> gLua;

#ifdef HAVE_LUA
#include <sol/sol.hpp>
struct LuaStateWrap { sol::state s; };

LuaVM::LuaVM() {}
LuaVM::~LuaVM() {}

bool LuaVM::Init()
{
    auto* wrap = new LuaStateWrap();
    L = (void*)&wrap->s;
    wrap->s.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::string);
    RegisterAPI();
    return true;
}

void LuaVM::Shutdown()
{
    if (L)
    {
        auto* wrap = (LuaStateWrap*)L; delete wrap; L=nullptr;
    }
}

bool LuaVM::RunFile(const std::string& path)
{
    if (!L) return false;
    auto* wrap = (LuaStateWrap*)L;
    sol::load_result lr = wrap->s.load_file(path);
    if (!lr.valid()) return false;
    sol::protected_function_result res = lr();
    return res.valid();
}

void LuaVM::RegisterAPI()
{
    auto* wrap = (LuaStateWrap*)L;
    auto& s = wrap->s;
    s.set_function("spawn_cube", [](){ return EditorActions::CreateDemoCubeEntity(); });
    s.set_function("set_pos", [](int e, float x, float y, float z){ auto& tr = gCoordinator.GetComponent<TransformComponent>((unsigned)e); tr.position = {x,y,z}; });
}
#else
LuaVM::LuaVM() {}
LuaVM::~LuaVM() {}
bool LuaVM::Init() { return false; }
void LuaVM::Shutdown() {}
bool LuaVM::RunFile(const std::string&) { return false; }
void LuaVM::RegisterAPI() {}
#endif
