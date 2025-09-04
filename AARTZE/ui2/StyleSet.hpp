#pragma once
#include <cstdint>

namespace ui2 {

struct Color { float r,g,b,a; };

struct StyleSet {
    // Backgrounds
    Color WindowBg {0.10f,0.10f,0.12f,1.0f}; // keep the chosen dark bg
    Color PanelBg  {0.16f,0.17f,0.20f,1.0f};
    // Accents
    Color Accent   {0.18f,0.50f,0.95f,1.0f};
    Color Text     {0.93f,0.95f,0.98f,1.0f};
    // Metrics
    float Rounding = 4.0f;
    float PaddingX = 8.0f;
    float PaddingY = 6.0f;
};

const StyleSet& GetStyle();
void SetStyle(const StyleSet& s);

}

