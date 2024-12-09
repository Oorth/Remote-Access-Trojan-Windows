#include <iostream>
#include <winsock2.h>
#include <windows.h>

using namespace std;
#pragma comment(lib, "ws2_32.lib");                                                                         // Link with ws2_32.lib

void send_data(SOCKET clientSocket, const string &data)                                                     // Doesnot send special data 
{
    int bytesSent = send(clientSocket, data.c_str(), data.length(), 0);
    
    if (bytesSent == SOCKET_ERROR) cerr << "Send failed with error: " << WSAGetLastError() << endl;
    else cout << "Sent data: " << data << endl;
}
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

int main()
{

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);                                                                      // Port 8080
    serverAddr.sin_addr.s_addr = INADDR_ANY;                                                                // Any available network interface
    
    WSADATA wsaData;

    
    WSAStartup(MAKEWORD(2, 2), &wsaData);                                                                   // Initialize Winsock
    
    SOCKET serversock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);                                          // Make socket
    int bindResult = bind(serversock, (sockaddr*)&serverAddr, sizeof(serverAddr));                          // Bind


    listen(serversock, SOMAXCONN);
    cout << "Waiting for incoming connections..." << endl;

    
    SOCKET clientSocket = accept(serversock, nullptr, nullptr);
    cout << "Client connected!" << endl;

    send_data(clientSocket," Gib Shell pls UwU ");
    cout << "got this -> " << receive_data(clientSocket) << endl;
    

    closesocket(clientSocket);
    closesocket(serversock);

    WSACleanup();
    return 0;
}
