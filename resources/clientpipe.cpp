#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

int main() {
    // Connect to the named pipe for receiving commands (stdin)
    HANDLE hNamedPipeIn = CreateFileW(
        L"\\\\.\\pipe\\ServerToClientIn", // Named pipe path for stdin
        GENERIC_READ | GENERIC_WRITE,     // Read and write access
        0,                               // No sharing
        NULL,                            // Default security attributes
        OPEN_EXISTING,                   // Open the existing pipe
        0,                               // No special attributes
        NULL                              // No template file
    );

    if (hNamedPipeIn == INVALID_HANDLE_VALUE) {
        std::cerr << "Error connecting to named pipe for input: " << GetLastError() << std::endl;
        return 1;
    }

    // Connect to the named pipe for sending output to the server (stdout)
    HANDLE hNamedPipeOut = CreateFileW(
        L"\\\\.\\pipe\\ClientToServerOut", // Named pipe path for stdout
        GENERIC_READ | GENERIC_WRITE,      // Read and write access
        0,                                // No sharing
        NULL,                             // Default security attributes
        OPEN_EXISTING,                    // Open the existing pipe
        0,                                // No special attributes
        NULL                               // No template file
    );

    if (hNamedPipeOut == INVALID_HANDLE_VALUE) {
        std::cerr << "Error connecting to named pipe for output: " << GetLastError() << std::endl;
        return 1;
    }

    std::cout << "Connected to server!" << std::endl;

    // Read command from the server
    char buffer[1024];
    DWORD bytesRead;
    if (ReadFile(hNamedPipeIn, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
        buffer[bytesRead] = '\0';  // Null-terminate the string
        std::cout << "Command received: " << buffer << std::endl;

        // Execute the command using cmd.exe
        // Using CreateProcess to launch cmd.exe
        STARTUPINFOW si = {0};
        PROCESS_INFORMATION pi = {0};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput = hNamedPipeIn;
        si.hStdOutput = hNamedPipeOut;
        si.hStdError = hNamedPipeOut;

        std::wstring cmd = L"cmd.exe /C " + std::wstring(buffer, buffer + bytesRead);
        if (!CreateProcessW(NULL, &cmd[0], NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
            std::cerr << "Error executing command: " << GetLastError() << std::endl;
            return 1;
        }

        // Wait for the command to execute and close the process
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    // Clean up
    CloseHandle(hNamedPipeIn);
    CloseHandle(hNamedPipeOut);

    return 0;
}
