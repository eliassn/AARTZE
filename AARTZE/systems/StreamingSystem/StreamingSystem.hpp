#pragma once
#include "core/System.hpp"
#include <future>
#include <string>
#include <unordered_map>

struct StreamingSystem : public System
{
    struct PendingMesh { std::future<uint32_t> fut; uint32_t meshId; std::string path; };
    std::unordered_map<uint32_t, PendingMesh> pending; // by meshId
    void Update(float) override;
    const char* GetName() const override { return "StreamingSystem"; }
    void RequestMesh(const std::string& path, uint32_t meshId);
    size_t PendingCount() const { return pending.size(); }
};
