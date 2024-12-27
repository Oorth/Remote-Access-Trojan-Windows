#include <iostream>
#include <windows.h>
#include <string>
#include <thread>
#include <tuple>
#include <atomic>

using namespace std;

int main() 
{
    SECURITY_ATTRIBUTES saAttr = {0};
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;

    HANDLE hChildStdOutRead, hChildStdOutWrite;  // stdout
    HANDLE hChildStdInRead, hChildStdInWrite;    // stdin

    // Create pipes for child process's STDOUT.
    CreatePipe(&hChildStdOutRead, &hChildStdOutWrite, &saAttr, 0);
    SetHandleInformation(hChildStdOutRead, HANDLE_FLAG_INHERIT, 0);

    // Create pipes for child process's STDIN.
    CreatePipe(&hChildStdInRead, &hChildStdInWrite, &saAttr, 0);
    SetHandleInformation(hChildStdInWrite, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW si = {0};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = hChildStdOutWrite;
    si.hStdError = hChildStdOutWrite;
    si.hStdInput = hChildStdInRead;

    PROCESS_INFORMATION pi = {0};
    wchar_t command[] = L"cmd.exe";
    if (!CreateProcessW(NULL, command, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        cerr << "CreateProcess failed with error code: " << GetLastError() << endl;
        return 1;
    }

    // Close handles not needed by the parent.
    CloseHandle(hChildStdOutWrite);
    CloseHandle(hChildStdInRead);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Lambda for reading from the child process stdout
    atomic<bool> processFinished(false);
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
                    cout << buffer;
                }
            } else if (GetLastError() == ERROR_BROKEN_PIPE)
            {
                // End of data
                break;
            }
            Sleep(10); // Sleep for a short time to prevent 100% CPU usage
        }
    });

    string cmd;
    while (true) 
    {
        cout << "> ";
        getline(cin, cmd);

        if(cmd == "exit") break;

        cmd += "\r\n";
        DWORD bytesWritten;
        if (!WriteFile(hChildStdInWrite, cmd.c_str(), cmd.length(), &bytesWritten, NULL)) {
            cerr << "WriteFile failed with error code: " << GetLastError() << endl;
            break;
        }
        FlushFileBuffers(hChildStdInWrite);
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Close the child's stdin
    processFinished.store(true);
    string exitCmd = "exit\r\n";
    DWORD bytesWritten;
    WriteFile(hChildStdInWrite, exitCmd.c_str(), exitCmd.length(), &bytesWritten, NULL);
    FlushFileBuffers(hChildStdInWrite);

    // Wait for the process to exit
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Clean up
    readThread.join();
    CloseHandle(hChildStdOutRead);
    CloseHandle(hChildStdInWrite);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    cout << "Ending program." << endl;
    return 0;
}
