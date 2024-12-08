#include <winsock2.h>
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")  // Link with ws2_32.lib

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaResult != 0) {
        std::cerr << "WSAStartup failed with error: " << wsaResult << std::endl;
        return 1;
    }

    // Create a socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;char buffer[512];
    }

    // Bind the socket to an address and port
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8080);

    int bindResult = bind(serverSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
    if (bindResult == SOCKET_ERROR) {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    int listenResult = listen(serverSocket, 5);  // Max backlog queue size = 5
    if (listenResult == SOCKET_ERROR) {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server is listening on port 8080..." << std::endl;

    // Accept an incoming connection
    SOCKET clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Client connected!" << std::endl;

    // Receive data from the client
    char recvBuffer[512];
    int recvResult = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
    if (recvResult > 0) {
        recvBuffer[recvResult] = '\0';  // Null-terminate the received data
        std::cout << "Received: " << recvBuffer << std::endl;
    } else if (recvResult == 0) {
        std::cout << "Connection closed by client." << std::endl;
    } else {
        std::cerr << "Recv failed with error: " << WSAGetLastError() << std::endl;
    }

    // Send data to the client
    std::string message = "Hello from the server!";
    int sendResult = send(clientSocket, message.c_str(), message.length(), 0);
    if (sendResult == SOCKET_ERROR) {
        std::cerr << "Send failed with error: " << WSAGetLastError() << std::endl;
    }

    // Cleanup
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
