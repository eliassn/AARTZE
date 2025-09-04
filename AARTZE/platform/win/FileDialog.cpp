#ifdef _WIN32
#include "FileDialog.hpp"
#include <windows.h>
#include <commdlg.h>
#include <shobjidl.h>

namespace WinFileDialog
{
std::string OpenFile(const char* filter)
{
    char file[260] = {0};
    OPENFILENAMEA ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = file;
    ofn.nMaxFile = sizeof(file);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    if (GetOpenFileNameA(&ofn) == TRUE)
        return std::string(ofn.lpstrFile);
    return std::string();
}

std::string PickFolder()
{
    IFileDialog* pfd = nullptr;
    std::string result;
    if (SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
    {
        if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
        {
            DWORD opts; pfd->GetOptions(&opts); pfd->SetOptions(opts | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
            if (SUCCEEDED(pfd->Show(NULL)))
            {
                IShellItem* item; if (SUCCEEDED(pfd->GetResult(&item)))
                {
                    PWSTR pszFilePath = NULL; if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath)))
                    {
                        char path[260]; WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, path, 260, NULL, NULL);
                        result = path; CoTaskMemFree(pszFilePath);
                    }
                    item->Release();
                }
            }
            pfd->Release();
        }
        CoUninitialize();
    }
    return result;
}

std::string SaveFile(const char* filter, const char* defaultExt)
{
    char file[260] = {0};
    OPENFILENAMEA ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = file;
    ofn.nMaxFile = sizeof(file);
    ofn.lpstrFilter = filter;
    ofn.lpstrDefExt = defaultExt;
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (GetSaveFileNameA(&ofn) == TRUE)
        return std::string(ofn.lpstrFile);
    return std::string();
}
}
#endif
