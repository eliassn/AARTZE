#pragma once
#ifdef _WIN32
#include <string>

namespace WinFileDialog
{
// Opens a native file open dialog. Returns empty string if user cancels.
// Filter example: "Model Files\0*.gltf;*.glb;*.fbx;*.obj;*.dae\0All Files\0*.*\0"
std::string OpenFile(const char* filter);

// Opens a native folder picker. Returns empty string if cancelled.
std::string PickFolder();

// Opens a native save dialog. Returns chosen path or empty.
// Filter example: "PNG Files\0*.png\0All Files\0*.*\0"
std::string SaveFile(const char* filter, const char* defaultExt);
}
#endif
