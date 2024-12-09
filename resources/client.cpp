#include <winsock2.h>
#include <iostream>
#include <ws2tcpip.h>

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
void send_data(SOCKET clientSocket, const string &data)                                                     // Doesnot send special data 
{
    int bytesSent = send(clientSocket, data.c_str(), data.length(), 0);
    
    if (bytesSent == SOCKET_ERROR) cerr << "Send failed with error: " << WSAGetLastError() << endl;
    else cout << "Sent data: " << data << endl;
}

int main()
{

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    WSADATA wsaData;

    
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    SOCKET clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    connect(clientSock, (sockaddr*)&serverAddr, sizeof(serverAddr));

    // const char* message = "Hello, Server!";
    // send(clientSock, message, strlen(message), 0);

    cout << "got this -> " << receive_data(clientSock) << endl;
    send_data(clientSock,"Ye lo");


    closesocket(clientSock);
    WSACleanup();
    
    return 0;
}
