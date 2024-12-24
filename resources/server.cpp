#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <sstream>
#include <vector>
#include <random>
#include <iomanip>

#pragma comment(lib, "ws2_32.lib")

// Base64 encoding function (ONLY for the handshake key)
std::string base64_encode(const std::string& in) {
    std::string out;
    std::vector<int> T(256);
    int i, j, k;
    for (i = 0; i < 256; i++) {
        T[i] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(i >> 2) + ((i & 3) << 4)];
    }
    for (i = 0; i < in.size(); i += 3) {
        j = ((unsigned char)in[i] << 16) + ((i + 1 < in.size()) ? ((unsigned char)in[i + 1] << 8) : 0) + ((i + 2 < in.size()) ? (unsigned char)in[i + 2] : 0);
        out += (char)T[(j >> 18) & 0x3F];
        out += (char)T[(j >> 12) & 0x3F];
        out += (i + 1 < in.size()) ? (char)T[(j >> 6) & 0x3F] : '=';
        out += (i + 2 < in.size()) ? (char)T[j & 0x3F] : '=';
    }
    return out;
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080); // Server port
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr); // Server IP

    if (connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // Generate WebSocket Key (for the handshake ONLY)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 255);
    std::string key;
    for (int i = 0; i < 16; ++i) {
        key += static_cast<char>(distrib(gen));
    }
    std::string encodedKey = base64_encode(key);

    // WebSocket Handshake (using the base64 encoded key)
    std::string handshake = "GET / HTTP/1.1\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: " + encodedKey + "\r\n"
        "Host: localhost:8080\r\n\r\n";
    send(sock, handshake.c_str(), handshake.length(), 0);

    char buffer[1024];
    int bytesReceived = recv(sock, buffer, 1024, 0);
    if (bytesReceived > 0) {
        std::string receivedMessage(buffer, bytesReceived);
        std::cout << "Handshake Response:\n" << receivedMessage << std::endl;

        if (receivedMessage.find("101 Switching Protocols") == std::string::npos) {
            std::cerr << "Incorrect handshake response from server.\n";
            closesocket(sock);
            WSACleanup();
            return 1;
        }

    } else {
        std::cerr << "Handshake failed or connection closed.\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // Send and Receive Data (Simplified - Plain Text - NO ENCODING HERE)
    while (true) {
        std::string messageToSend;
        std::cout << "Enter message to send (or 'exit' to quit): ";
        std::getline(std::cin, messageToSend);

        if (messageToSend == "exit") {
            break;
        }

        send(sock, messageToSend.c_str(), messageToSend.length(), 0); // Send raw text

        bytesReceived = recv(sock, buffer, 1024, 0);
        if (bytesReceived > 0) {
            std::string receivedMessage(buffer, bytesReceived);
            std::cout << "Received: " << receivedMessage << std::endl;
        } else if (bytesReceived == 0) {
            std::cout << "Server closed the connection.\n";
            break;
        } else {
            std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
            break;
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}