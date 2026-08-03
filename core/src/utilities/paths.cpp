#include "paths.hpp"

#include "core.hpp"
#include "utilities/module_info.hpp"

#include <windows.h>
#include <shlobj.h>
#include <gsl/gsl>

#include <array>

std::filesystem::path windower::windower_path()
{
    namespace fs = std::filesystem;

    auto buffer = std::array<::WCHAR, MAX_PATH + 1>{};
    auto library = static_cast<::HMODULE>(windower_module());
    ::GetModuleFileNameW(library, buffer.data(), buffer.size());
    buffer.back() = L'\0';
    return fs::canonical(fs::path{ buffer.data() }.remove_filename());
}

std::filesystem::path windower::settings_path()
{
    namespace fs = std::filesystem;

    auto result = windower::core::instance().settings.settings_path;

    if (result.empty())
    {
        ::WCHAR* wstr = nullptr;
        auto const deleter = gsl::finally([&]() { ::CoTaskMemFree(wstr); });
        if (SUCCEEDED(::SHGetKnownFolderPath(
            FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &wstr)))
        {
            result = fs::path{ wstr } / u8"Windower";
        }
    }

    return result;
}

std::filesystem::path windower::user_path()
{
    namespace fs = std::filesystem;

    auto result = windower::core::instance().settings.user_path;

    if (result.empty())
    {
        ::WCHAR* wstr = nullptr;
        auto const deleter = gsl::finally([&]() { ::CoTaskMemFree(wstr); });
        if (SUCCEEDED(::SHGetKnownFolderPath(
            FOLDERID_SavedGames, KF_FLAG_CREATE, nullptr, &wstr)))
        {
            result = fs::path{ wstr } / u8"Windower";
        }
    }

    return result;
}

std::filesystem::path windower::temp_path()
{
    namespace fs = std::filesystem;

    auto result = windower::core::instance().settings.temp_path;

    if (result.empty())
    {
        result = fs::temp_directory_path() / u8"Windower";
    }

    return result;
}

std::filesystem::path windower::client_path()
{
    namespace fs = std::filesystem;

    fs::path result;

    auto handle = ::GetModuleHandleW(L"ffximain.dll");
    if (handle)
    {
        auto buffer = std::array<::WCHAR, MAX_PATH + 1>{};
        ::GetModuleFileNameW(handle, buffer.data(), buffer.size());
        buffer.back() = L'\0';
        result = fs::canonical(fs::path{ buffer.data() }.remove_filename());
    }

    return result;
}
