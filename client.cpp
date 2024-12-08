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
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Define the server address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);  // Port number of the server
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");  // Localhost IP address

    // Connect to the server
    int connectResult = connect(clientSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
    if (connectResult == SOCKET_ERROR) {
        std::cerr << "Connect failed with error: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server!" << std::endl;

    // Send data to the server
    std::string message = "Hello from the client!";
    int sendResult = send(clientSocket, message.c_str(), message.length(), 0);
    if (sendResult == SOCKET_ERROR) {
        std::cerr << "Send failed with error: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // Receive data from the server
    char recvBuffer[512];
    int recvResult = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
    if (recvResult > 0) {
        recvBuffer[recvResult] = '\0';  // Null-terminate the received data
        std::cout << "Received from server: " << recvBuffer << std::endl;
    } else if (recvResult == 0) {
        std::cout << "Connection closed by server." << std::endl;
    } else {
        std::cerr << "Recv failed with error: " << WSAGetLastError() << std::endl;
    }

    // Cleanup
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
