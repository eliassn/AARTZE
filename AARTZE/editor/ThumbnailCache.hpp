#pragma once
#include <string>
#include <unordered_map>
#include <glad/glad.h>

namespace ThumbnailCache
{
// Returns a GL texture id for the given image path, loading and caching on demand.
// Returns a persistent placeholder texture if the file cannot be loaded.
GLuint Get(const std::string& path);
void Clear();
}

