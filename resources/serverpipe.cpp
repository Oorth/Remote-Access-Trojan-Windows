#include <windows.h>
#include <iostream>
#include <string>

using namespace std;

int main() {
    // Create named pipe for client to send output to server
    HANDLE hNamedPipeOut = CreateNamedPipeW(
        L"\\\\.\\pipe\\ClientToServerOut", // Named pipe path for stdout
        PIPE_ACCESS_INBOUND,              // Only read from pipe
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        1,                                // Number of instances
        1024,                             // Output buffer size
        1024,                             // Input buffer size
        0,                                // Default timeout
        NULL                              // Default security attributes
    );

    if (hNamedPipeOut == INVALID_HANDLE_VALUE) {
        std::cerr << "Error creating named pipe for output: " << GetLastError() << std::endl;
        return 1;
    }

    // Create named pipe for sending commands to the client (stdin)
    HANDLE hNamedPipeIn = CreateNamedPipeW(
        L"\\\\.\\pipe\\ServerToClientIn",  // Named pipe path for stdin
        PIPE_ACCESS_OUTBOUND,             // Only write to pipe
        PIPE_TYPE_MESSAGE | PIPE_WAIT,
        1,                                // Number of instances
        1024,                             // Output buffer size
        1024,                             // Input buffer size
        0,                                // Default timeout
        NULL                              // Default security attributes
    );

    if (hNamedPipeIn == INVALID_HANDLE_VALUE) {
        std::cerr << "Error creating named pipe for input: " << GetLastError() << std::endl;
        return 1;
    }

    std::cout << "Waiting for client to connect..." << std::endl;
    
    // Wait for the client to connect
    if (!ConnectNamedPipe(hNamedPipeOut, NULL) || !ConnectNamedPipe(hNamedPipeIn, NULL)) {
        std::cerr << "Error connecting to the named pipe: " << GetLastError() << std::endl;
        return 1;
    }

    std::cout << "Client connected!" << std::endl;

    // Send a command to the client
    const std::string command = "dir";  // Example: Command to execute
    DWORD bytesWritten;
    WriteFile(hNamedPipeIn, command.c_str(), command.length(), &bytesWritten, NULL);

    // Read the command output from the client
    char buffer[1024];
    DWORD bytesRead;
    if (ReadFile(hNamedPipeOut, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
        buffer[bytesRead] = '\0';  // Null-terminate the output
        std::cout << "Received from client: " << buffer << std::endl;
    } else {
        std::cerr << "Error reading output from pipe: " << GetLastError() << std::endl;
    }

    // Clean up
    CloseHandle(hNamedPipeOut);
    CloseHandle(hNamedPipeIn);

    return 0;
}
