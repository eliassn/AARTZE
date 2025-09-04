#include "StreamingSystem.hpp"
#include <chrono>
#include "utils/AssetLoader.hpp"
#include "systems/RenderingSystem/RenderResources.hpp"
#include "utils/MeshUtils.hpp"

void StreamingSystem::RequestMesh(const std::string& path, uint32_t meshId)
{
    if (pending.count(meshId)) return;
    auto f = AssetLoader::LoadMeshAsync(path);
    pending.emplace(meshId, PendingMesh{std::move(f), meshId, path});
}

void StreamingSystem::Update(float)
{
    for (auto it = pending.begin(); it != pending.end(); )
    {
        auto& p = it->second;
        if (p.fut.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            uint32_t mid = p.fut.get(); (void)mid; // mesh registry ID
            MeshData md;
            auto ext = std::filesystem::path(p.path).extension().string();
            for(char& c:ext) c=(char)tolower((unsigned char)c);
            if (ext == ".gltf" || ext == ".glb") md = LoadGltfModel(p.path, true, 1.0f);
            else md = LoadMeshAny(p.path, true, 1.0f);
            if (!md.vertices.empty()) RenderResources::UploadMesh(p.meshId, md);
            it = pending.erase(it);
        }
        else ++it;
    }
}
