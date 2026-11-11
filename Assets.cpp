#include "GameTypes.hpp"
#ifdef _WIN32
#include <windows.h>
#endif

namespace sandbox {
std::filesystem::path assetDirectory() {
#ifdef _WIN32
    wchar_t path[MAX_PATH]{};
    const DWORD length = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (length > 0 && length < MAX_PATH)
        return std::filesystem::path(path).parent_path() / "assets";
#endif
    return std::filesystem::current_path() / "assets";
}
}
