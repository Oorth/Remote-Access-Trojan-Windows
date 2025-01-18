//cl /EHsc /LD .\test_lib.cpp /link User32.lib

#include <Windows.h>

// Function prototype for the ReflectiveLoader
extern "C" __declspec(dllexport) void ReflectiveLoader();

// Entry point for reflective injection
void ReflectiveLoader()
{
    MessageBoxA(NULL, "Reflective Loader Executed!", "Info", MB_ICONINFORMATION);

    // Perform additional tasks here (e.g., resolve imports, call payload)
    // This would normally include parsing the PE header, resolving addresses, etc.
}

// DLL entry point
BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH: {
            DisableThreadLibraryCalls(hModule); // Call the Windows API directly.
            ReflectiveLoader(); // Call the reflective loader.
            break;
        }
        case DLL_PROCESS_DETACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
    }
    return TRUE;
}