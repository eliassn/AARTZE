#pragma once
#include <string>
#include <vector>

struct Node
{
    int id;
    std::string name;
};

struct Link { int a; int b; };

struct NodeGraph
{
    std::vector<Node> nodes;
    std::vector<Link> links;
    int nextId{1};
    int AddNode(const std::string& name){ nodes.push_back(Node{nextId++, name}); return nodes.back().id; }
    void AddLink(int a,int b){ links.push_back({a,b}); }
};

extern NodeGraph gNodeGraph;

