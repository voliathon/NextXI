/*
 * Copyright © Windower Dev Team
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"),to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "crash_handler.hpp"

#include "cloak.hpp"
#include "guid.hpp"
#include "library.hpp"
#include "unicode.hpp"
#include "utility.hpp"

#include <windows.h>

#include <crtdbg.h>
#include <dbghelp.h>
#include <objbase.h>
#include <stdlib.h>

#include <gsl/gsl>

#include <array>
#include <bit>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <string>
#include <string_view>
#include <vector>

namespace
{

constexpr auto default_dump_type =
#if defined(_DEBUG)
    windower::dump_type::full;
#else
    windower::dump_type::basic;
#endif

std::wstring quote_argument(std::wstring_view argument)
{
    std::wstring result;
    if (!argument.empty() &&
        argument.find_first_of(L" \t\n\v\"") == std::wstring::npos)
    {
        result.append(argument);
    }
    else
    {
        result.reserve(argument.size() + 2);
        result.push_back(L'"');
        auto mark            = argument.begin();
        auto backslash_count = 0;
        for (auto it = argument.begin(); it != argument.end(); ++it)
        {
            if (*it == L'\\')
            {
                ++backslash_count;
            }
            else
            {
                if (*it == L'"')
                {
                    result.append(mark, it);
                    result.append(backslash_count + 1, L'\\');
                    result.push_back(L'"');
                    mark = ++it;
                }
                backslash_count = 0;
            }
        }
        result.append(mark, argument.end());
        result.append(backslash_count, L'\\');
        result.push_back(L'"');
    }
    return result;
}

std::wstring get_module_name(::HMODULE library)
{
    std::vector<wchar_t> buffer(MAX_PATH);
    auto size = ::GetModuleFileNameW(library, buffer.data(), buffer.size());
    while (size == buffer.size())
    {
        buffer.resize(buffer.size() * 2);
        size = ::GetModuleFileNameW(library, buffer.data(), buffer.size());
    }
    if (size == 0)
    {
        throw std::system_error{
            gsl::narrow_cast<int>(::GetLastError()), std::system_category()};
    }
    std::filesystem::path path{buffer.begin(), buffer.begin() + size};
    return path.filename().wstring();
}

std::wstring get_signature(::EXCEPTION_POINTERS& exception)
{
    auto const address = exception.ExceptionRecord->ExceptionAddress;
    auto const library = static_cast<::HMODULE>(windower::module_for(address));

    auto const code        = exception.ExceptionRecord->ExceptionCode;
    auto const module_name = get_module_name(library);
    auto const offset      = std::bit_cast<std::uintptr_t>(address) -
                        std::bit_cast<std::uintptr_t>(library);

    std::wstring result;
    auto hex_code = windower::to_u8string(code, 16);
    result.append(hex_code.size() >= 8 ? 0 : 8 - hex_code.size(), L'0');
    result.append(hex_code.begin(), hex_code.end());
    result.append(1, L'@');
    result.append(module_name);
    result.append(1, L'+');
    auto hex_offset = windower::to_u8string(offset, 16);
    result.append(hex_offset.size() >= 8 ? 0 : 8 - hex_offset.size(), L'0');
    result.append(hex_offset.begin(), hex_offset.end());
    return result;
}

std::filesystem::path generate_unique_name()
{
    std::filesystem::path result;
    result += windower::guid::generate().string();
    result += u8".dmp";
    return result;
}

void write_dump(
    std::filesystem::path const& path, windower::dump_type type,
    void* exception)
{
    std::filesystem::create_directories(path.parent_path());
    auto file = ::CreateFileW(
        path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file != INVALID_HANDLE_VALUE)
    {
        MINIDUMP_EXCEPTION_INFORMATION info = {};
        info.ExceptionPointers = static_cast<::EXCEPTION_POINTERS*>(exception);
        info.ThreadId          = ::GetCurrentThreadId();
        auto guard             = windower::uncloak();
        ::MiniDumpWriteDump(
            ::GetCurrentProcess(), ::GetCurrentProcessId(), file,
            gsl::narrow_cast<::MINIDUMP_TYPE>(type), &info, nullptr, nullptr);
        ::CloseHandle(file);
    }
}

}

extern "C"
{
    static ::LONG CALLBACK
    unhandled_exception_handler(::PEXCEPTION_POINTERS ExceptionInfo)
    {
        if (!ExceptionInfo || !ExceptionInfo->ExceptionRecord ||
            ExceptionInfo->ExceptionRecord->ExceptionCode !=
                DBG_PRINTEXCEPTION_C)
        {
            windower::crash_handler::instance().crash(ExceptionInfo);
        }
        return EXCEPTION_CONTINUE_SEARCH;
    }

    static void abort_handler(int)
    {
        WINDOWER_DEBUG_BREAK;
        std::_Exit(-1);
    }

#if defined(_MSC_VER)
    static void pure_call_handler()
    {
        WINDOWER_DEBUG_BREAK;
        std::_Exit(-1);
    }

    static void invalid_parameter_handler(
        [[maybe_unused]] wchar_t const* expression,
        [[maybe_unused]] wchar_t const* function,
        [[maybe_unused]] wchar_t const* file,
        [[maybe_unused]] unsigned int line,
        [[maybe_unused]] std::uintptr_t reserved)
    {
        WINDOWER_DEBUG_BREAK;
        std::_Exit(-1);
    }
#endif

#if defined(_MSC_VER) && defined(_DEBUG)
    static int crt_error_handler(
        [[maybe_unused]] int reportType, [[maybe_unused]] wchar_t* message,
        [[maybe_unused]] int* returnValue)
    {
        WINDOWER_DEBUG_BREAK;
        std::_Exit(-1);
    }
#endif
}

void windower::crash_handler::initialize() { instance(); }

windower::crash_handler& windower::crash_handler::instance()
{
    static crash_handler instance{
        std::filesystem::temp_directory_path() / u8"Windower"};
    return instance;
}

std::filesystem::path windower::crash_handler::write_dump(
    std::filesystem::path const& path, windower::dump_type type)
{
    auto temp = path / generate_unique_name();
    ::EXCEPTION_RECORD record{};
    ::CONTEXT context{};
    ::RtlCaptureContext(&context);
    ::EXCEPTION_POINTERS exception{&record, &context};
    ::write_dump(temp, type, &exception);
    return temp;
}

windower::crash_handler::crash_handler(std::filesystem::path const& path) :
    m_path{path}, m_full_path{path / ::generate_unique_name()},
    m_full_path_quoted{::quote_argument(m_full_path.wstring())},
    m_dump_type{::default_dump_type}
{
    ::SetUnhandledExceptionFilter(::unhandled_exception_handler);
    std::signal(SIGABRT, ::abort_handler);

#if defined(_MSC_VER)
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
    _set_purecall_handler(::pure_call_handler);
    _set_invalid_parameter_handler(::invalid_parameter_handler);
    _CrtSetReportHookW2(_CRT_RPTHOOK_INSTALL, ::crt_error_handler);
    _CrtSetReportMode(_CRT_WARN, 0);
    _CrtSetReportMode(_CRT_ERROR, 0);
    _CrtSetReportMode(_CRT_ASSERT, 0);
#endif

    if (library kernel32{u8"kernel32.dll"})
    {
        auto get_policy = kernel32.get_function<::BOOL WINAPI(::LPDWORD)>(
            u8"GetProcessUserModeExceptionPolicy");
        auto set_policy = kernel32.get_function<::BOOL WINAPI(::DWORD)>(
            u8"SetProcessUserModeExceptionPolicy");
        if (get_policy && set_policy)
        {
            ::DWORD flags;
            if (get_policy(&flags))
            {
                set_policy(flags & ~0x1);
            }
        }
    }
}

std::filesystem::path const& windower::crash_handler::dump_path() const noexcept
{
    return m_path;
}

void windower::crash_handler::dump_path(std::filesystem::path const& path)
{
    auto temp_path             = path;
    auto temp_full_path        = path / generate_unique_name();
    auto temp_full_path_quoted = quote_argument(temp_full_path.wstring());

    m_path             = std::move(temp_path);
    m_full_path        = std::move(temp_full_path);
    m_full_path_quoted = std::move(temp_full_path_quoted);
}

windower::dump_type windower::crash_handler::default_dump_type() const noexcept
{
    return m_dump_type;
}

void windower::crash_handler::default_dump_type(
    windower::dump_type dump_type) noexcept
{
    m_dump_type = dump_type;
}

std::filesystem::path windower::crash_handler::write_dump() const
{
    return write_dump(m_path, m_dump_type);
}

std::filesystem::path
windower::crash_handler::write_dump(windower::dump_type dump_type) const
{
    return write_dump(m_path, dump_type);
}

std::filesystem::path
windower::crash_handler::write_dump(std::filesystem::path const& path) const
{
    return write_dump(path, m_dump_type);
}

void windower::crash_handler::crash(void* exception) const
{
    // Redirect the memory dump so it doesn't create files/temp/
    auto dmp_path = windower_path() / u8"crash.dmp";
    ::write_dump(dmp_path, m_dump_type, exception);

    auto reporter = windower_path() / u8"windower.exe";

    // Pass the new dmp_path to the C# Launcher so it stops throwing
    // exceptions!
    auto args = quote_argument(reporter.wstring()) + L" report-crash " +
                quote_argument(dmp_path.wstring());

if (auto ptr = static_cast<::EXCEPTION_POINTERS*>(exception))
    {
        args.append(L" --signature ");
        args.append(quote_argument(get_signature(*ptr)));

        // --- GENERATE CRASH.LOG ---
        // windower_path() points directly to your bin/release/ folder!
        auto report_path = windower_path() / u8"crash.log";
        std::ofstream report(report_path);
        if (report)
        {
            report << "# Windower 5 Engine Crash Report\n\n";

            auto sig = get_signature(*ptr);
            std::string narrow_sig;
            narrow_sig.reserve(sig.size());
            for (auto const wc : sig)
            {
                narrow_sig.push_back(gsl::narrow_cast<char>(wc));
            }

            report << "**Signature:** `" << narrow_sig << "`\n\n";

            report << "### Exception Details\n";
            report << "- **Code:** `0x" << std::hex << std::uppercase
                   << ptr->ExceptionRecord->ExceptionCode << "`\n";
            report << "- **Address (Instruction Pointer):** `0x"
                   << ptr->ExceptionRecord->ExceptionAddress << "`\n";

            // 0xC0000005 is EXCEPTION_ACCESS_VIOLATION
            if (ptr->ExceptionRecord->ExceptionCode == 0xC0000005)
            {
                report << "- **Violation Type:** "
                       << (ptr->ExceptionRecord->ExceptionInformation[0] == 0
                               ? "Read"
                               : (ptr->ExceptionRecord
                                              ->ExceptionInformation[0] == 1
                                      ? "Write"
                                      : "Data Execution Prevention (DEP)"))
                       << "\n";
                report << "- **Target Memory Address:** `0x"
                       << ptr->ExceptionRecord->ExceptionInformation[1]
                       << "`\n";
            }

            report << "\n### CPU Registers\n";
            report << "```text\n";
#if defined(_M_IX86) // FFXI is a 32-bit process
            report << "EAX: 0x" << std::hex << ptr->ContextRecord->Eax << "\n";
            report << "EBX: 0x" << std::hex << ptr->ContextRecord->Ebx << "\n";
            report << "ECX: 0x" << std::hex << ptr->ContextRecord->Ecx << "\n";
            report << "EDX: 0x" << std::hex << ptr->ContextRecord->Edx << "\n";
            report << "ESI: 0x" << std::hex << ptr->ContextRecord->Esi << "\n";
            report << "EDI: 0x" << std::hex << ptr->ContextRecord->Edi << "\n";
            report << "EBP: 0x" << std::hex << ptr->ContextRecord->Ebp << "\n";
            report << "ESP: 0x" << std::hex << ptr->ContextRecord->Esp << "\n";
            report << "EIP: 0x" << std::hex << ptr->ContextRecord->Eip << "\n";
#endif
            report << "```\n";

            // --- C++ CALL STACK GENERATOR ---
            report << "\n### C++ Call Stack\n";
            report << "```text\n";

            ::HANDLE process = ::GetCurrentProcess();
            ::HANDLE thread  = ::GetCurrentThread();

            // Tell DbgHelp to "demangle" the ugly C++ symbols into
            // human-readable text
            ::SymSetOptions(
                SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);

            // Explicitly point the Stack Walker at the Windower
            // directory! Because FFXI is hosted by pol.exe, it was searching
            // the PlayOnline folder by default.
            std::string search_path = windower_path().string();
            ::SymInitialize(process, search_path.c_str(), TRUE);

            ::STACKFRAME64 frame{};
            frame.AddrPC.Mode    = AddrModeFlat;
            frame.AddrFrame.Mode = AddrModeFlat;
            frame.AddrStack.Mode = AddrModeFlat;

            // FIX: Make a complete, isolated copy of the CPU state!
            // StackWalk64 intentionally modifies this object as it walks
            // backwards.
            ::CONTEXT context_copy = *ptr->ContextRecord;

#if defined(_M_IX86) // FFXI is a 32-bit process
            constexpr auto machine_type = IMAGE_FILE_MACHINE_I386;
            frame.AddrPC.Offset         = context_copy.Eip;
            frame.AddrFrame.Offset      = context_copy.Ebp;
            frame.AddrStack.Offset      = context_copy.Esp;
#elif defined(_M_X64)
            constexpr auto machine_type = IMAGE_FILE_MACHINE_AMD64;
            frame.AddrPC.Offset         = context_copy.Rip;
            frame.AddrFrame.Offset      = context_copy.Rbp;
            frame.AddrStack.Offset      = context_copy.Rsp;
#endif

            char symbol_buffer[sizeof(::SYMBOL_INFO) + 256];
            auto symbol = reinterpret_cast<::SYMBOL_INFO*>(symbol_buffer);
            symbol->SizeOfStruct = sizeof(::SYMBOL_INFO);
            symbol->MaxNameLen   = 255;

            for (int i = 0; i < 32; ++i)
            {
                // FIX: Pass '&context_copy' instead of the live
                // 'ptr->ContextRecord'
                if (!::StackWalk64(
                        machine_type, process, thread, &frame, &context_copy,
                        nullptr, ::SymFunctionTableAccess64,
                        ::SymGetModuleBase64, nullptr))
                {
                    break;
                }

                if (frame.AddrPC.Offset == 0)
                    break;
                ::DWORD64 displacement = 0;
                if (::SymFromAddr(
                        process, frame.AddrPC.Offset, &displacement, symbol))
                {
                    // Explicitly take the address of the first character
                    // to prevent implicit array-to-pointer decay (bounds.3)
                    auto const* name_ptr = &symbol->Name[0];
                    report << i << ": "
                           << std::string_view{name_ptr, symbol->NameLen}
                           << " + 0x" << std::hex << displacement << "\n";
                }
                else
                {
                    // SILVER BULLET FALLBACK: If the PDB fails, natively query
                    // the OS for the DLL Name and Offset!
                    auto const mod =
                        static_cast<::HMODULE>(windower::module_for(
                            reinterpret_cast<void*>(frame.AddrPC.Offset)));
                    if (mod)
                    {
                        auto const mod_name = get_module_name(mod);
                        auto const offset   = frame.AddrPC.Offset -
                                            std::bit_cast<std::uintptr_t>(mod);

                        std::string narrow_mod;
                        narrow_mod.reserve(mod_name.size());
                        for (auto const wc : mod_name)
                            narrow_mod.push_back(gsl::narrow_cast<char>(wc));

                        report << i << ": " << narrow_mod << " + 0x" << std::hex
                               << offset << "\n";
                    }
                    else
                    {
                        report << i << ": [Unknown Module] at 0x" << std::hex
                               << frame.AddrPC.Offset << "\n";
                    }
                }
            }

            ::SymCleanup(process);
            report << "```\n";
            // -------------------------------------
        }
        // --- END REPORT.MD ---
    }

    ::PROCESS_INFORMATION process_info = {};
    ::STARTUPINFOW startup_info        = {};
    startup_info.cb                    = sizeof startup_info;
    auto const result                  = ::CreateProcessW(
        reporter.c_str(), args.data(), nullptr, nullptr, false, 0, nullptr,
        nullptr, &startup_info, &process_info);
    if (result)
    {
        ::CloseHandle(process_info.hProcess);
        ::CloseHandle(process_info.hThread);
    }

    constexpr auto access = SYNCHRONIZE | PROCESS_TERMINATE;
    auto const pid        = ::GetCurrentProcessId();
    auto const handle     = ::OpenProcess(access, false, pid);
    ::TerminateProcess(handle, EXIT_FAILURE);
    ::WaitForSingleObject(handle, INFINITE);
}