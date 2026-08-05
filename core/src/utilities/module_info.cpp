#include "module_info.hpp"

#include <windows.h>

#include <algorithm>
#include <array>
#include <bit>

extern "C" ::IMAGE_DOS_HEADER __ImageBase;

void* windower::module_for(void const* ptr) noexcept
{
    ::HMODULE library = nullptr;
    ::DWORD constexpr flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
    if (!::GetModuleHandleExW(flags, static_cast<::LPCWSTR>(ptr), &library))
    {
        return nullptr;
    }
    return library;
}

void* windower::windower_module() noexcept { return &__ImageBase; }

bool windower::is_windower_module(void const* ptr) noexcept
{
    auto const begin = std::bit_cast<std::byte const*>(&__ImageBase);
    auto const end = std::next(
        begin, std::bit_cast<::IMAGE_NT_HEADERS const*>(
            std::next(begin, __ImageBase.e_lfanew))
        ->OptionalHeader.SizeOfImage);
    return begin <= ptr && ptr < end;
}

bool windower::is_game_module(void const* ptr) noexcept
{
    static constexpr auto const modules = std::array<::WCHAR const*, 10>{
        L"ffximain.dll",   nullptr,        L"polcore.dll",
        L"polcoreeu.dll",  L"polhook.dll", L"app.dll",
        L"appeu.dll",      L"ffxi.dll",    L"ffxiresource.dll",
        L"ffxiversion.dll" };

    auto const module = module_for(ptr);
    return module != nullptr &&
        std::find_if(modules.begin(), modules.end(), [module](auto name) {
        return ::GetModuleHandleW(name) == module;
            }) != modules.end();
}
