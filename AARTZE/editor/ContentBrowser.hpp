#pragma once
#include <string>
#include <vector>

struct FileEntry { std::string name; bool isDir; };

class ContentBrowser
{
public:
    explicit ContentBrowser(const std::string& root);
    const std::string& CurrentDir() const { return m_current; }
    void SetDir(const std::string& dir);
    bool Up();
    std::vector<FileEntry> List() const;

private:
    std::string m_root;
    std::string m_current;
};

