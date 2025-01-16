// /cl /EHsc /LD .\test_lib.cpp
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
            int MessageBox(
            HWND    hWnd,
            LPCTSTR lpText,
            LPCTSTR lpCaption,
            UINT    uType
            );

            cout << "DLL_PROCESS_ATTACH" << endl;

        }
        break;
        case DLL_THREAD_ATTACH: // A process is creating a new thread.
        {
            // int MessageBox(
            // HWND    hWnd,
            // LPCTSTR lpText,
            // LPCTSTR lpCaption,
            // UINT    uType
            // );
        }
        break;
        case DLL_THREAD_DETACH: // A thread exits normally.
        {
            cout << "[thread deatch]";
        }
        break;
    }
    return TRUE;
}