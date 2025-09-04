#pragma once
#include <future>
#include <string>

#include "core/Coordinator.hpp"
#include "World/MeshRegistry.hpp"
#include "World/TextureRegistry.hpp"
#include "utils/SkeletonUtils.hpp"

// Forward declaration so async loaders can call the global function
uint32_t RegisterMesh(const std::string& path);

namespace AssetLoader
{
inline std::future<uint32_t> LoadMeshAsync(const std::string& path)
{
    return gThreadPool.enqueue([path]() { return ::RegisterMesh(path); });
}

inline std::future<uint32_t> LoadTextureAsync(const std::string& path)
{
    return gThreadPool.enqueue([path]() { return ::RegisterTexture(path); });
}

inline std::future<SkeletonComponent> LoadSkeletonAsync(const std::string& path)
{
    return gThreadPool.enqueue([path]() { return LoadFbxSkeleton(path); });
}

inline std::future<std::vector<AnimationClip>> LoadAnimationsAsync(
    const std::string& path, const SkeletonComponent& skeleton)
{
    return gThreadPool.enqueue(
        [path, skeleton]() { return LoadFbxAnimations(path, skeleton); });
}
}  // namespace AssetLoader
