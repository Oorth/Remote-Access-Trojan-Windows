#include <iostream>
#include <winsock2.h>
#include <windows.h>

using namespace std;
#pragma comment(lib, "ws2_32.lib")                                                                          // Link with ws2_32.lib

int main()
{

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);                                                                      // Port 8080
    serverAddr.sin_addr.s_addr = INADDR_ANY;                                                                // Any available network interface
    
    WSADATA wsaData;

    
    WSAStartup(MAKEWORD(2, 2), &wsaData);                                                                   // Initialize Winsock
    
    SOCKET serversock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);                                                // Make socket
    int bindResult = bind(serversock, (sockaddr*)&serverAddr, sizeof(serverAddr));                                // Bind


    listen(serversock, SOMAXCONN);
    cout << "Waiting for incoming connections..." << std::endl;

    
    SOCKET clientSocket = accept(serversock, nullptr, nullptr);
    cout << "Client connected!" << std::endl;

    char buffer[512];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        cout << "Received: " << buffer << endl;

        // Echo the message back to the client
        send(clientSocket, buffer, bytesReceived, 0);
    }

    closesocket(clientSocket);
    closesocket(serversock);

    // Clean up Winsock before exiting
    WSACleanup();
    return 0;
}
