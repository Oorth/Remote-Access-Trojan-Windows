//cl /EHsc /LD .\cute_lib.cpp /link User32.lib
#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        MessageBoxA(NULL, "DLL Attached", "Notification", MB_OK);
        break;
    case DLL_THREAD_ATTACH:
        MessageBoxA(NULL, "Thread Attached", "Notification", MB_OK);
        break;
    case DLL_THREAD_DETACH:
        MessageBoxA(NULL, "Thread Detached", "Notification", MB_OK);
        break;
    case DLL_PROCESS_DETACH:
        MessageBoxA(NULL, "DLL Detached", "Notification", MB_OK);
        break;
    }
    return TRUE;
}
