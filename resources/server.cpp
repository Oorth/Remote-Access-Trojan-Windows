#include <winsock2.h>
#include <iostream>
#include <ws2tcpip.h>
#include <Windows.h>
#include <string>
#include <thread>
#include <tuple>
#include <atomic>

using namespace std;
#pragma comment(lib, "ws2_32.lib")                                                              // Link with ws2_32.lib

void send_data(SOCKET clientSocket, const string &data)                                         // Doesnot send special data 
{
    int bytesSent = send(clientSocket, data.c_str(), data.length(), 0);
    if (bytesSent == SOCKET_ERROR) cerr << "Send failed with error: " << WSAGetLastError() << endl;
}
string receive_data(SOCKET clientSocket)
{
    char buffer[1024];                                                                           // Buffer to store received data
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

int main()
{
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
    {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) 
    {
        std::cerr << "socket failed.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8081);                                                                      
    serverAddr.sin_addr.s_addr = INADDR_ANY;                                                    // Any available network interface
    
    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "bind failed.\n";
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }
    
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "listen failed.\n";
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }
    
    cout << "Waiting for incoming connections..." << endl;
    SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
    
    cout << "\nClient connected!" << endl << "_______________________________________________________________\n";
    
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    atomic<bool> processFinished(false);
    
    auto readThread = std::thread([&]()
    {
        while (!processFinished.load())
        {
            string data = receive_data(clientSocket);
            if (data.empty()) break;                                                            // Client has disconnected or there was an error
            cout << data;
            Sleep(10);                                                                          // Sleep for a short time to prevent 100% CPU usage
        }
    });

    auto writeThread = std::thread([&]()
    {
        string cmd;
        while (!processFinished.load())                                                                     
        {
            if (getline(cin, cmd))                                                              //getline to allow multi-word commands
            {
                if (cmd == "exit")
                {
                    send_data(clientSocket, "exit");
                    processFinished.store(true);
                    break;
                }
                send_data(clientSocket, cmd);
            }
            Sleep(10);
        }
    });

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // Wait for process threads to finish
    readThread.join();
    writeThread.join();
    processFinished.store(true);

    // Cleanup and shutdown
    cout << "Client disconnected. Server shutting down..." << endl;
    
    closesocket(clientSocket);
    closesocket(listenSocket);

    WSACleanup();
    return 0;
}
