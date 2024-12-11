#include <iostream>
#include <windows.h>
#include <string>
using namespace std;

int main() 
{
    SECURITY_ATTRIBUTES saAttr = {0};
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    
    HANDLE hChildStdOutRead, hChildStdOutWrite;                             //stdout
    HANDLE hChildStdInRead, hChildStdInWrite;                               //stdin

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
    CreateProcessW(NULL, command, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
    
    // Close handles not needed by the parent.
    CloseHandle(hChildStdOutWrite);
    CloseHandle(hChildStdInRead);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // Write a command to the child process.
    const char* cmd = "dir\n";  // Example command.
    DWORD bytesWritten;
    WriteFile(hChildStdInWrite, cmd, strlen(cmd), &bytesWritten, NULL);
    FlushFileBuffers(hChildStdInWrite); // Ensure the command is sent.
    // Read the output from the child process.
    char buffer[4096];
    DWORD bytesRead;
    while (ReadFile(hChildStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) 
    {
        buffer[bytesRead] = '\0';  // Null-terminate the output.
        cout << buffer;
    }
    
    // Cleanup.
    CloseHandle(hChildStdOutRead);
    CloseHandle(hChildStdInWrite);
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}