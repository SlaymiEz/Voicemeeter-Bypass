#include <cstdint>
#include <Windows.h>
#include <TlHelp32.h>

namespace {
    DWORD get_process_id(const wchar_t* process_name) {
        DWORD process_id = 0;

        const HANDLE snap_shot = CreateToolhelp32Snapshot (TH32CS_SNAPPROCESS, NULL);
        if (snap_shot == INVALID_HANDLE_VALUE)
            return process_id;

        PROCESSENTRY32W entry = {};
        entry.dwSize = sizeof(decltype(entry));
	    
        if (Process32FirstW(snap_shot, &entry) == TRUE) {
            // Check if the first handle is the one we want.
            if (_wcsicmp(process_name, entry.szExeFile) == 0)
                process_id = entry.th32ProcessID;
            else {
                while (Process32NextW(snap_shot, &entry) == TRUE) {
                    if (_wcsicmp(process_name, entry.szExeFile) == 0) {
                        process_id = entry.th32ProcessID;
                        break;
                    }
                }
            }
        }

        CloseHandle(snap_shot);

        return process_id;
    }

    std::uintptr_t get_module_base(const DWORD pid, const wchar_t* module_name) {
        std::uintptr_t module_base = 0;
	    
        HANDLE snap_shot = CreateToolhelp32Snapshot (TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
        if (snap_shot == INVALID_HANDLE_VALUE)
            return module_base;
	    
        MODULEENTRY32W entry = {};
        entry.dwSize = sizeof(decltype(entry));
	    
        if (Module32FirstW(snap_shot, &entry) == TRUE) {
            if (wcsstr (module_name, entry.szModule) != nullptr)
                module_base = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
            else {
                while (Module32NextW(snap_shot, &entry) == TRUE) {
                    if (wcsstr (module_name, entry.szModule) != nullptr) {
                        module_base = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
                        break;
                    }
                }
            }
        }
        CloseHandle(snap_shot);
        return module_base;
    }

    template <typename T>
    constexpr T Read(const std::uintptr_t& address, void* processHandle) noexcept {
        T value = { };
        ::ReadProcessMemory(processHandle, reinterpret_cast<const void*>(address), &value, sizeof(T), nullptr);
        return value;
    }

    template <typename T>
    constexpr void Write(const std::uintptr_t& address, const T& value, void* processHandle)  noexcept
    {
        ::WriteProcessMemory(processHandle, reinterpret_cast<void*>(address), &value, sizeof(T), nullptr);
    }    
}



namespace offsets {
    constexpr auto rsiPtr = 0x3BE598;
    constexpr auto timerPtr = 0x10D70;
}

INT APIENTRY WinMain(HINSTANCE, HINSTANCE, PSTR, INT) {

    DWORD pid = get_process_id(L"voicemeeter8x64.exe");

    while (pid == 0) {
        pid = get_process_id(L"voicemeeter8x64.exe");
        Sleep(5);
    }

    Sleep(500);

    const auto base = get_module_base(pid, L"voicemeeter8x64.exe");
    
    void* processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

    if (const auto rsi = Read<std::uintptr_t>(base + offsets::rsiPtr, processHandle); Read<std::int32_t>(rsi + offsets::timerPtr, processHandle)) Write(rsi + offsets::timerPtr, 0, processHandle);
    
    return 0;
}