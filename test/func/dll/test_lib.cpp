//cl /EHsc /LD .\test_lib.cpp /link User32.lib

#include <Windows.h>
#include <iostream>

using namespace std;

BOOL APIENTRY DllMain(
    HANDLE hModule,// Handle to DLL module
    DWORD ul_reason_for_call,// Reason for calling function
    LPVOID lpReserved ) // Reserved
{

    switch ( ul_reason_for_call )
    {
        case DLL_PROCESS_ATTACH: // A process is loading the DLL.
        {
            MessageBoxA(NULL, "DLL_PROCESS_ATTACH", "Hola", MB_ICONINFORMATION);

        }
        break;
        case DLL_THREAD_ATTACH: // A process is creating a new thread.
        {
            MessageBoxA(NULL, "DLL_THREAD_ATTACH", "Hola", MB_ICONINFORMATION);
        }
        break;
        case DLL_THREAD_DETACH: // A thread exits normally.
        {
            MessageBoxA(NULL, "DLL_THREAD_DETACH", "Hola", MB_ICONEXCLAMATION);
        }
        break;
        case DLL_PROCESS_DETACH: // A process is terminating.
        {
            MessageBoxA(NULL, "DLL_PROCESS_DETACH", "Hola", MB_ICONEXCLAMATION);
        }
        break;
    }
    return TRUE;
}