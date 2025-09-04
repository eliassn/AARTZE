#pragma once
#include <array>
#include <optional>
#include <vector>

struct GridNav
{
    int width{64}, height{64};
    float cellSize{1.0f};
    std::vector<uint8_t> blocked; // 0 free, 1 blocked

    GridNav() : blocked(width*height, 0) {}
    bool InBounds(int x,int y) const { return x>=0 && y>=0 && x<width && y<height; }
    bool IsFree(int x,int y) const { return InBounds(x,y) && blocked[y*width+x]==0; }
    std::array<int,2> ToCell(float x,float z) const { return { int(x/cellSize), int(z/cellSize) }; }
    std::array<float,3> CellCenter(int x,int y) const { return { (x+0.5f)*cellSize, 0.0f, (y+0.5f)*cellSize }; }
    std::vector<std::array<int,2>> FindPath(std::array<int,2> start, std::array<int,2> goal) const;
};

