#pragma once
#include <cstdint>

class TextRenderingSystem
{
public:
    bool Initialize() { return true; }
    void Shutdown() {}
    void Render(int width, int height);
};

