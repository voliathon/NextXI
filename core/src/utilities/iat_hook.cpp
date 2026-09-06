#include "iat_hook.hpp"
#include <gsl/gsl>
#include <cstring>

#pragma warning(push)
#pragma warning(disable: 26481 26485 26462 26461)

namespace windower::memory
{
    bool hook_iat(
        ::HMODULE target_module,
        std::string_view dll_name,
        std::string_view func_name,
        void* proxy_func,
        void** original_func
    ) noexcept
    {
        if (!target_module || !proxy_func) return false;

        auto const* const base = reinterpret_cast<const std::uint8_t*>(target_module);
        auto const* const dos_header = reinterpret_cast<const ::IMAGE_DOS_HEADER*>(base);
        if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) return false;

        auto const* const nt_headers = reinterpret_cast<const ::IMAGE_NT_HEADERS*>(base + dos_header->e_lfanew);
        if (nt_headers->Signature != IMAGE_NT_SIGNATURE) return false;

        auto const& import_dir = nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
        if (import_dir.Size == 0 || import_dir.VirtualAddress == 0) return false;

        auto* import_desc = reinterpret_cast<::PIMAGE_IMPORT_DESCRIPTOR>(const_cast<std::uint8_t*>(base + import_dir.VirtualAddress));

        for (; import_desc->Name != 0; ++import_desc)
        {
            auto const* const current_dll_name = reinterpret_cast<const char*>(base + import_desc->Name);
            if (_stricmp(current_dll_name, dll_name.data()) == 0)
            {
                auto* thunk = reinterpret_cast<::PIMAGE_THUNK_DATA>(const_cast<std::uint8_t*>(base + import_desc->FirstThunk));
                auto const* orig_thunk = import_desc->OriginalFirstThunk
                    ? reinterpret_cast<const ::IMAGE_THUNK_DATA*>(base + import_desc->OriginalFirstThunk)
                    : reinterpret_cast<const ::IMAGE_THUNK_DATA*>(thunk);

                for (; thunk->u1.Function != 0; ++thunk, ++orig_thunk)
                {
                    if (IMAGE_SNAP_BY_ORDINAL(orig_thunk->u1.Ordinal)) continue;

                    auto const* const import_by_name = reinterpret_cast<const ::IMAGE_IMPORT_BY_NAME*>(base + orig_thunk->u1.AddressOfData);
                    if (std::strcmp(reinterpret_cast<const char*>(import_by_name->Name), func_name.data()) == 0)
                    {
                        ::DWORD old_protect = 0;
                        if (!::VirtualProtect(&thunk->u1.Function, sizeof(std::uintptr_t), PAGE_READWRITE, &old_protect))
                        {
                            return false;
                        }

                        if (original_func && *original_func == nullptr)
                        {
                            *original_func = reinterpret_cast<void*>(thunk->u1.Function);
                        }

                        thunk->u1.Function = reinterpret_cast<std::uintptr_t>(proxy_func);

                        ::VirtualProtect(&thunk->u1.Function, sizeof(std::uintptr_t), old_protect, &old_protect);
                        return true;
                    }
                }
            }
        }

        return false;
    }
}

#pragma warning(pop)
