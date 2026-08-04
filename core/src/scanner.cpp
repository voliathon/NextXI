#include "scanner.hpp"
#include "library.hpp"

#include <windows.h>

#include <algorithm>
#include <bit>
#include <cstddef>
#include <span>

namespace
{

    // Suppress bounds/type warnings because parsing PE headers requires raw memory offsets
    [[gsl::suppress("bounds.1"), gsl::suppress("type.1")]]
    std::span<::IMAGE_SECTION_HEADER const> get_sections(windower::library const& library) noexcept
    {
        auto const* const base = static_cast<std::byte const*>(library);
        if (!base) return {};

        auto const* const dos_header = std::bit_cast<::IMAGE_DOS_HEADER const*>(base);
        if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) return {};

        auto const* const nt_header = std::bit_cast<::IMAGE_NT_HEADERS const*>(base + dos_header->e_lfanew);
        if (nt_header->Signature != IMAGE_NT_SIGNATURE || nt_header->FileHeader.Machine != IMAGE_FILE_MACHINE_I386) return {};

        auto const* const section_headers = std::bit_cast<::IMAGE_SECTION_HEADER const*>(
            base + dos_header->e_lfanew + sizeof(nt_header->Signature) + sizeof(nt_header->FileHeader) + nt_header->FileHeader.SizeOfOptionalHeader);

        return { section_headers, nt_header->FileHeader.NumberOfSections };
    }

    [[gsl::suppress("bounds.1"), gsl::suppress("type.1")]]
    std::span<std::byte const> get_data(windower::library const& library, ::IMAGE_SECTION_HEADER const& section) noexcept
    {
        if ((section.Characteristics & IMAGE_SCN_MEM_EXECUTE) == 0) return {};

        auto const* const base = static_cast<std::byte const*>(library);
        return { base + section.VirtualAddress, section.Misc.VirtualSize };
    }

    // Suppress bounds.4 so we can use raw indexing for maximum scan performance
    [[gsl::suppress("bounds.1"), gsl::suppress("bounds.4")]]
    std::span<std::byte const>::iterator match(std::span<std::byte const> data, windower::signature const& sig)
    {
        auto const sig_data = sig.data();
        auto const sig_mask = sig.mask();

        if (data.size() < sig.size()) return data.end();

        auto it = data.begin();
        auto const end = data.end() - sig.size() + 1;

        while (it != end)
        {
            // Fast-path: find the first byte first
            it = std::find(it, end, sig_data[0]);
            if (it == end) break;

            // Verify the remainder of the signature against the mask
            bool is_match = true;
            for (std::size_t i = 1; i < sig.size(); ++i)
            {
                if ((it[i] & sig_mask[i]) != sig_data[i])
                {
                    is_match = false;
                    break;
                }
            }

            if (is_match) return it;
            ++it;
        }

        return data.end();
    }

} // namespace

[[gsl::suppress("bounds.1")]]
void windower::scan(library const& library, signature const& sig, std::span<address> results) noexcept
{
    if (library && !results.empty())
    {
        for (auto const& section : get_sections(library))
        {
            auto section_data = get_data(library, section);
            auto it = match(section_data, sig);

            while (!results.empty() && it != section_data.end())
            {
                auto result = address{ std::to_address(it) };
                result += sig.offset();

                gsl::at(results, 0) = sig.dereference() ? *result : result;
                results = results.subspan(1);

                section_data = section_data.subspan(std::distance(section_data.begin(), it) + 1);
                it = match(section_data, sig);
            }

            if (results.empty()) return;
        }
    }
    std::fill(results.begin(), results.end(), nullptr);
}
