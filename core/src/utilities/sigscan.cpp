#include "sigscan.hpp"

#include <windows.h>
#include <psapi.h>

#include <string>
#include <vector>

namespace windower::util
{

std::uint8_t* SigScan(const char* pattern, std::ptrdiff_t offset, const char* module_name)
{
    ::HMODULE module = ::GetModuleHandleA(module_name);
    if (!module)
    {
        return nullptr;
    }

    ::MODULEINFO module_info = {};
    if (!::GetModuleInformation(::GetCurrentProcess(), module, &module_info, sizeof(module_info)))
    {
        return nullptr;
    }

    auto base = reinterpret_cast<std::uint8_t*>(module_info.lpBaseOfDll);
    auto size = module_info.SizeOfImage;

    std::vector<int> pattern_bytes;
    size_t pat_len = std::strlen(pattern);
    for (size_t i = 0; i < pat_len; )
    {
        if (pattern[i] == ' ' || pattern[i] == '#')
        {
            ++i;
            continue;
        }
        if (pattern[i] == '?')
        {
            pattern_bytes.push_back(-1);
            if (i + 1 < pat_len && pattern[i + 1] == '?')
            {
                i += 2;
            }
            else
            {
                i += 1;
            }
        }
        else
        {
            char byte_str[3] = {pattern[i], pattern[i + 1], '\0'};
            pattern_bytes.push_back(std::strtol(byte_str, nullptr, 16));
            i += 2;
        }
    }

    auto pattern_size = pattern_bytes.size();
    if (pattern_size == 0)
    {
        return nullptr;
    }

    auto pattern_data = pattern_bytes.data();

    for (DWORD i = 0; i < size - pattern_size; )
    {
        ::MEMORY_BASIC_INFORMATION mbi;
        if (!::VirtualQuery(base + i, &mbi, sizeof(mbi)))
        {
            break;
        }

        if (mbi.State != MEM_COMMIT || (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)))
        {
            i += static_cast<DWORD>(reinterpret_cast<std::uint8_t*>(mbi.BaseAddress) + mbi.RegionSize - (base + i));
            continue;
        }

        DWORD region_end = static_cast<DWORD>(reinterpret_cast<std::uint8_t*>(mbi.BaseAddress) + mbi.RegionSize - base);
        if (region_end > size) region_end = size;
        if (region_end < pattern_size) break;

        for (; i <= region_end - pattern_size; ++i)
        {
            bool found = true;
            for (size_t j = 0; j < pattern_size; ++j)
            {
                if (pattern_data[j] >= 0 && base[i + j] != static_cast<std::uint8_t>(pattern_data[j]))
                {
                    found = false;
                    break;
                }
            }

            if (found)
            {
                return base + i + offset;
            }
        }
        
        i = region_end;
    }

    return nullptr;
}

}
