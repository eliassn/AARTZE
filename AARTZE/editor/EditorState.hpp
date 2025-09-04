#pragma once
#include <string>
#include <unordered_map>

namespace EditorState
{
void SetFolder(unsigned entity, const std::string& folder);
const std::string& GetFolder(unsigned entity);
void SetVisible(unsigned entity, bool v);
bool IsVisible(unsigned entity);
void SetLocked(unsigned entity, bool v);
bool IsLocked(unsigned entity);

// Iterates entities grouped by folder (returns a map copy for convenience)
std::unordered_map<std::string, std::vector<unsigned>> GroupByFolder();

// Selection
void SetSelected(int entity);
int  GetSelected();

// Gizmo + snapping state
void SetGizmoMode(int mode);      // 0=T,1=R,2=S
int  GetGizmoMode();
void SetSnap(bool enabled, float value);
bool GetSnapEnabled();
float GetSnapValue();

// Transform space
enum class TransformSpace { World, Local };
void SetTransformSpace(TransformSpace s);
TransformSpace GetTransformSpace();
}
