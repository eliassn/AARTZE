#pragma once
#include <string>
#include <vector>

struct ProjectInfo
{
    std::string name;
    std::string path; // absolute or relative
    std::string lastOpened; // ISO timestamp
};

class ProjectManager
{
public:
    bool LoadRecents(const std::string& file = "aartze_recent.json");
    bool SaveRecents(const std::string& file = "aartze_recent.json") const;

    bool CreateProject(const std::string& name,
                       const std::string& locationDir,
                       int templateId,
                       int language);
    bool OpenProject(const std::string& projectDir);

    const std::vector<ProjectInfo>& Recents() const { return m_recents; }
    bool HasOpenProject() const { return !m_currentPath.empty(); }
    const std::string& CurrentPath() const { return m_currentPath; }
    const std::string& CurrentName() const { return m_currentName; }

private:
    std::vector<ProjectInfo> m_recents;
    std::string m_currentPath;
    std::string m_currentName;
};

extern ProjectManager gProjectManager;

