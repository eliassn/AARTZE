#pragma once
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>

// Minimal texture registry for AARTZE.
// Maps texture file paths to stable numeric IDs.
inline uint32_t RegisterTexture(const std::string& path)
{
    static std::unordered_map<std::string, uint32_t> s_map;
    static std::mutex s_mutex;
    static uint32_t s_nextId = 1;

    std::lock_guard<std::mutex> lock(s_mutex);
    auto it = s_map.find(path);
    if (it != s_map.end()) return it->second;
    uint32_t id = s_nextId++;
    s_map.emplace(path, id);
    return id;
}

