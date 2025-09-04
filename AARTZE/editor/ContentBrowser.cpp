#include "ContentBrowser.hpp"
#include <filesystem>
#include <algorithm>

ContentBrowser::ContentBrowser(const std::string& root) : m_root(root), m_current(root) {}

void ContentBrowser::SetDir(const std::string& dir)
{
    std::filesystem::path p = dir;
    if (p.is_relative()) p = std::filesystem::path(m_current) / p;
    if (std::filesystem::exists(p) && std::filesystem::is_directory(p)) m_current = p.string();
}

bool ContentBrowser::Up()
{
    std::filesystem::path p = m_current;
    if (p == std::filesystem::path(m_root)) return false;
    if (p.has_parent_path()) { p = p.parent_path(); m_current = p.string(); return true; }
    return false;
}

std::vector<FileEntry> ContentBrowser::List() const
{
    std::vector<FileEntry> out;
    if (!std::filesystem::exists(m_current)) return out;
    for (auto& de : std::filesystem::directory_iterator(m_current))
    {
        FileEntry fe; fe.name = de.path().filename().string(); fe.isDir = de.is_directory(); out.push_back(std::move(fe));
    }
    std::sort(out.begin(), out.end(), [](const FileEntry& a, const FileEntry& b){ if (a.isDir!=b.isDir) return a.isDir>b.isDir; return a.name<b.name; });
    return out;
}
