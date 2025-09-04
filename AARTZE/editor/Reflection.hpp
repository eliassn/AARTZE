#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

// Very small reflection system used by the Inspector to auto-generate
// property editors for selected components.

namespace Reflection
{
struct DrawCtx
{
    int entity;      // entity id
    const char* filter; // optional search filter (nullptr or empty = show all)
};

using DrawFn = std::function<void(const DrawCtx&)>;

struct ComponentInfo
{
    std::string name;
    DrawFn draw;     // draws all properties for the component if present on entity
};

// Register a component inspector. Name should be stable and human friendly.
void Register(const std::string& name, DrawFn draw);

// Enumerate registered component inspectors
const std::vector<ComponentInfo>& List();
}

