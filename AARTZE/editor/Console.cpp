#include "Console.hpp"
#include <sstream>
#include <string>
#include <vector>

#include "EditorActions.hpp"
#include "save/SaveSystem.hpp"

namespace {
std::vector<std::string> gLog;
}

namespace EditorConsole
{
void Clear(){ gLog.clear(); }
void Log(const std::string& line){ gLog.push_back(line); }
const std::vector<std::string>& Lines(){ return gLog; }

void Exec(const std::string& line)
{
    gLog.push_back("> " + line);
    std::istringstream iss(line);
    std::string cmd; iss >> cmd;
    if (cmd == "help")
    {
        gLog.push_back("Commands: help, save [file], load [file], cube, physics");
        return;
    }
    else if (cmd == "save")
    {
        std::string f = "world.json"; iss >> f; if (SaveSystem::SaveWorld(f)) gLog.push_back("Saved "+f); else gLog.push_back("Save failed");
        return;
    }
    else if (cmd == "load")
    {
        std::string f = "world.json"; iss >> f; if (SaveSystem::LoadWorld(f)) gLog.push_back("Loaded "+f); else gLog.push_back("Load failed");
        return;
    }
    else if (cmd == "cube")
    {
        EditorActions::CreateDemoCubeEntity(); gLog.push_back("Spawned cube"); return;
    }
    else if (cmd == "physics")
    {
        EditorActions::CreatePhysicsDemo(); gLog.push_back("Created physics demo"); return;
    }
    gLog.push_back("Unknown command. Try 'help'.");
}
}

