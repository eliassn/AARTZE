#include "ProjectManager.hpp"
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include "save/SaveSystem.hpp"
#include "editor/EditorActions.hpp"
#include <chrono>

using json = nlohmann::json;

ProjectManager gProjectManager;

static std::string NowISO()
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    char buf[64];
#ifdef _WIN32
    tm tmv; localtime_s(&tmv, &t);
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &tmv);
#else
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", std::localtime(&t));
#endif
    return buf;
}

bool ProjectManager::LoadRecents(const std::string& file)
{
    m_recents.clear();
    if (!std::filesystem::exists(file)) return true;
    std::ifstream f(file, std::ios::binary); if (!f.is_open()) return false;
    json j; f >> j;
    for (auto& it : j["recents"]) m_recents.push_back({it["name"], it["path"], it.value("lastOpened", "")});
    return true;
}

bool ProjectManager::SaveRecents(const std::string& file) const
{
    json j; j["recents"] = json::array();
    for (auto& r : m_recents) j["recents"].push_back({{"name", r.name}, {"path", r.path}, {"lastOpened", r.lastOpened}});
    std::ofstream f(file, std::ios::binary); if (!f.is_open()) return false; f << j.dump(2); return true;
}

bool ProjectManager::CreateProject(const std::string& name,
                                   const std::string& locationDir,
                                   int templateId,
                                   int language)
{
    (void)templateId; (void)language; // placeholders for future scaffolding
    std::filesystem::path dir = std::filesystem::path(locationDir) / name;
    std::error_code ec; std::filesystem::create_directories(dir, ec);
    if (!std::filesystem::exists(dir)) return false;
    // Write a minimal project file
    json pj; pj["name"] = name; pj["created"] = NowISO(); pj["template"] = templateId; pj["language"] = language;
    std::ofstream pf((dir / "project.json").string(), std::ios::binary); if (!pf.is_open()) return false; pf << pj.dump(2);
    if (!OpenProject(dir.string())) return false;
    // Bootstrap template scene and save as world.json
    // Simple mapping for now
    if (templateId == 0)
    {
        // Blank: ground only
        EditorActions::CreatePhysicsDemo(); // ground + cube (gives something)
    }
    else if (templateId == 1)
    {
        EditorActions::CreatePhysicsDemo();
    }
    else
    {
        EditorActions::CreateDemoCubeEntity();
    }
    SaveSystem::SaveWorld((dir / "world.json").string());
    return true;
}

bool ProjectManager::OpenProject(const std::string& projectDir)
{
    std::filesystem::path dir = projectDir;
    if (!std::filesystem::exists(dir)) return false;
    m_currentPath = dir.string();
    // Try read name; else use folder name
    std::string name = dir.filename().string();
    auto pj = dir / "project.json";
    if (std::filesystem::exists(pj))
    {
        std::ifstream f(pj.string(), std::ios::binary); if (f.is_open()) { json j; f >> j; name = j.value("name", name); }
    }
    m_currentName = name;

    // Update recents (dedupe by path)
    auto it = std::find_if(m_recents.begin(), m_recents.end(), [&](const ProjectInfo& p){ return p.path == m_currentPath; });
    if (it != m_recents.end()) it->lastOpened = NowISO();
    else m_recents.insert(m_recents.begin(), ProjectInfo{name, m_currentPath, NowISO()});
    // Cap list
    if (m_recents.size() > 12) m_recents.resize(12);
    SaveRecents();
    // Auto-load default world if present
    auto w = dir / "world.json";
    if (std::filesystem::exists(w)) SaveSystem::LoadWorld(w.string());
    return true;
}
