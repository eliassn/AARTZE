#include "ThumbnailCache.hpp"
#include <stb_image.h>
#include <string>
#include <filesystem>

namespace {
std::unordered_map<std::string, GLuint> gCache;
GLuint gPlaceholder = 0;

GLuint MakePlaceholder()
{
    if (gPlaceholder) return gPlaceholder;
    unsigned char pix[4*4*3];
    for (int y=0;y<4;++y){ for(int x=0;x<4;++x){ bool c=((x+y)&1)!=0; pix[(y*4+x)*3+0]=c?220:60; pix[(y*4+x)*3+1]=c?220:60; pix[(y*4+x)*3+2]=c?220:60; } }
    glGenTextures(1, &gPlaceholder);
    glBindTexture(GL_TEXTURE_2D, gPlaceholder);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 4, 4, 0, GL_RGB, GL_UNSIGNED_BYTE, pix);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return gPlaceholder;
}
}

namespace ThumbnailCache
{
GLuint Get(const std::string& path)
{
    if (path.empty() || !std::filesystem::exists(path)) return MakePlaceholder();
    auto it = gCache.find(path);
    if (it != gCache.end()) return it->second;

    int w,h,n;
    stbi_uc* data = stbi_load(path.c_str(), &w,&h,&n, 4);
    if (!data) return MakePlaceholder();

    GLuint tex=0; glGenTextures(1,&tex); glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    stbi_image_free(data);

    gCache[path] = tex;
    return tex;
}

void Clear()
{
    for (auto& kv : gCache) glDeleteTextures(1, &kv.second);
    gCache.clear();
    if (gPlaceholder) { glDeleteTextures(1, &gPlaceholder); gPlaceholder=0; }
}
}
