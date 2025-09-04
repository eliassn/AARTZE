#pragma once
#include <string>
#include "core/Coordinator.hpp"

namespace EditorActions
{
// Loads mesh from path (gltf/obj/fbx), uploads to GPU, and creates an entity.
// Returns entity id or -1 on failure.
int CreateEntityFromMeshPath(const std::string& path);
// Import helper that chooses correct loader based on extension and uploads
// Returns meshId (0 on failure)
uint32_t ImportMeshAndUpload(const std::string& path);

// Creates a unit cube entity as a fallback/demo.
int CreateDemoCubeEntity();
// Spawns a static ground and a dynamic cube with physics.
void CreatePhysicsDemo();
}
