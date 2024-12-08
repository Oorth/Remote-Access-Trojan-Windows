#include <winsock2.h>
#include <iostream>
#include <ws2tcpip.h>

using namespace std;
#pragma comment(lib, "ws2_32.lib")

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

    const char* message = "Hello, Server!";
    send(clientSock, message, strlen(message), 0);

    char buffer[512];
    int bytesReceived = recv(clientSock, buffer, sizeof(buffer), 0);
    if (bytesReceived > 0)
    {
        buffer[bytesReceived] = '\0';
        cout << "Received from server: " << buffer << endl;
    }

    closesocket(clientSock);
    WSACleanup();
    
    return 0;
}
