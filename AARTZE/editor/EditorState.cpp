#include "EditorState.hpp"
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {
std::unordered_map<unsigned, std::string> gFolder;
std::unordered_set<unsigned> gHidden;
std::unordered_set<unsigned> gLocked;
static std::string emptyStr;
int gSelected = -1;
int gGizmoMode = 0;
bool gSnapEnabled = false;
float gSnapValue = 1.0f;
EditorState::TransformSpace gSpace = EditorState::TransformSpace::World;
}

namespace EditorState
{
void SetFolder(unsigned e, const std::string& f){ gFolder[e] = f; }
const std::string& GetFolder(unsigned e){ auto it=gFolder.find(e); return it==gFolder.end()?emptyStr:it->second; }
void SetVisible(unsigned e, bool v){ if (v) gHidden.erase(e); else gHidden.insert(e); }
bool IsVisible(unsigned e){ return gHidden.find(e)==gHidden.end(); }
void SetLocked(unsigned e, bool v){ if (v) gLocked.insert(e); else gLocked.erase(e); }
bool IsLocked(unsigned e){ return gLocked.find(e)!=gLocked.end(); }

std::unordered_map<std::string, std::vector<unsigned>> GroupByFolder()
{
    std::unordered_map<std::string, std::vector<unsigned>> m;
    // Caller will iterate alive entities; this just provides mapping convenience
    for (auto& kv : gFolder) m[kv.second].push_back(kv.first);
    return m;
}

void SetSelected(int e){ gSelected = e; }
int  GetSelected(){ return gSelected; }
void SetGizmoMode(int mode){ gGizmoMode = mode; }
int  GetGizmoMode(){ return gGizmoMode; }
void SetSnap(bool enabled, float value){ gSnapEnabled = enabled; gSnapValue = value; }
bool GetSnapEnabled(){ return gSnapEnabled; }
float GetSnapValue(){ return gSnapValue; }
void SetTransformSpace(TransformSpace s){ gSpace = s; }
TransformSpace GetTransformSpace(){ return gSpace; }
}
