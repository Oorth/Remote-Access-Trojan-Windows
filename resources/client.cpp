#include <winsock2.h>
#include <iostream>
#include <ws2tcpip.h>

HANDLE hChildStdOutRead, hChildStdOutWrite;                             //stdout
HANDLE hChildStdInRead, hChildStdInWrite;                               //stdin

using namespace std;
#pragma comment(lib, "ws2_32.lib")

string receive_data(SOCKET clientSocket)
{
    char buffer[512];                                                                           // Buffer to store received data
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

    if (bytesReceived > 0)
    {
        buffer[bytesReceived] = '\0';                                                           // Null-terminate the string
        string receivedData(buffer);                                                            // Store the data in a string
        return receivedData;
    }
    else if (bytesReceived == 0) cerr << "Connection closed by server." << std::endl;
    else cerr << "Receive failed with error: " << WSAGetLastError() << std::endl;

    return "";                                                                                  // Return an empty string in case of error or connection closure
}
void send_data(SOCKET clientSocket, const string &data)                                                  // Doesnot send special data 
{
    int bytesSent = send(clientSocket, data.c_str(), data.length(), 0);
    
    if (bytesSent == SOCKET_ERROR) cerr << "Send failed with error: " << WSAGetLastError() << endl;
    else cout << "Sent data: " << data << endl;
}
void give_command(const std::string& command)
{
    string cmd = command + "\n";
    // Write a command to the child process.
    DWORD bytesWritten;
    WriteFile(hChildStdInWrite, cmd.c_str(), cmd.length(), &bytesWritten, NULL);
    FlushFileBuffers(hChildStdInWrite); // Ensure the command is sent.

}

int main()
{

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    SOCKET clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //connect(clientSock, (sockaddr*)&serverAddr, sizeof(serverAddr));


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //cout << "got this -> " << receive_data(clientSock) << endl;
    //send_data(clientSock,"Ye lo");
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



    give_command("dir");
    //give_command(receive_data(clientSock));

    //send_data(clientSock, const string &data)       
    
    // Read the output from the child process.
    // char buffer[4096];
    // DWORD bytesRead;
    // while (ReadFile(hChildStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) 
    // {
    //     buffer[bytesRead] = '\0';  // Null-terminate the output.
    //     cout << buffer;
    // }

// Start reading from the pipe with a timeout
    char buffer[4096];
    DWORD bytesRead = 0;
    DWORD timeout = 5000;  // 5 seconds timeout
    DWORD startTime = GetTickCount();

    while (TRUE)
    {
        // Check the time elapsed
        DWORD elapsedTime = GetTickCount() - startTime;
        if (elapsedTime >= timeout)
        {
            // Timeout reached
            cout << "Timed out while reading output." << endl;
            break;
        }

        // Try to read from the pipe
        if (ReadFile(hChildStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
        {
            buffer[bytesRead] = '\0';  // Null-terminate the buffer
            cout << "Received Output: " << buffer << endl;
            break;  // Successfully read, break out of the loop
        }
        else
        {
            // Sleep for a while to avoid busy-waiting
            Sleep(100);  // Sleep 10 milliseconds
        }
    }

    cout<< "here";

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    CloseHandle(hChildStdOutRead);      // Make sure to close the read end of the pipe after reading.
    WaitForSingleObject(pi.hProcess, INFINITE);         // Wait for the child process to finish.
    
    //CloseHandle(hChildStdInWrite);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
 
    //send_data(clientSock," closing!! ");

    closesocket(clientSock);
    WSACleanup();
    
    return 0;
}
