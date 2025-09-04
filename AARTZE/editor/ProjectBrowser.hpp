#pragma once
#include <string>
#include <vector>

class ProjectBrowser
{
public:
    void Init();
    void Show(bool* pOpen);

private:
    int m_tab = 0; // 0 Recent, 1 Templates, 2 Samples
    int m_selectedTemplate = 0;
    int m_language = 0; // 0 Blueprint, 1 C++
    char m_projName[128] = "MyProject";
    char m_location[260] = "projects";

    void DrawRecent();
    void DrawTemplates();
    void DrawSamples();
    // (picker removed)
};
