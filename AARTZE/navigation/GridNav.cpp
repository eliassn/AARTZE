#include "GridNav.hpp"
#include <queue>
#include <unordered_map>

struct Node{int x,y;};
static int key(int x,int y,int w){ return y*w + x; }

std::vector<std::array<int,2>> GridNav::FindPath(std::array<int,2> start, std::array<int,2> goal) const
{
    auto h = [&](int x,int y){ return abs(goal[0]-x)+abs(goal[1]-y); };
    struct PQ{int k; int x,y; int f;};
    auto cmp=[](const PQ&a,const PQ&b){return a.f>b.f;};
    std::priority_queue<PQ,std::vector<PQ>,decltype(cmp)> open(cmp);
    std::unordered_map<int,int> came;
    std::unordered_map<int,int> g;
    int s = key(start[0],start[1],width);
    open.push({s,start[0],start[1],h(start[0],start[1])});
    g[s]=0;
    const int dirs[4][2]={{1,0},{-1,0},{0,1},{0,-1}};
    while(!open.empty()){
        auto cur=open.top(); open.pop();
        if (cur.x==goal[0] && cur.y==goal[1]) break;
        for(auto& d:dirs){int nx=cur.x+d[0], ny=cur.y+d[1]; if(!IsFree(nx,ny)) continue; int nk=key(nx,ny,width); int ng=g[key(cur.x,cur.y,width)]+1; if(!g.count(nk) || ng<g[nk]){ g[nk]=ng; came[nk]=key(cur.x,cur.y,width); open.push({nk,nx,ny,ng+h(nx,ny)});} }
    }
    std::vector<std::array<int,2>> path;
    int goalKey = key(goal[0],goal[1],width);
    if(!g.count(goalKey)) return path;
    int cur=goalKey; while(cur!=s){ int px=came[cur]%width; int py=came[cur]/width; path.push_back({cur%width,cur/width}); cur=came[cur]; if(cur==came[cur]) break; }
    path.push_back(start);
    std::reverse(path.begin(), path.end());
    return path;
}

