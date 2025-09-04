#pragma once
#include <string>
#include <vector>

namespace Toasts
{
struct Item { std::string text; double ttl; };
void Push(const std::string& text, double seconds = 3.0);
void UpdateAndDraw();
}

