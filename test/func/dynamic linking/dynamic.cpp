#include <windows.h>
#include <iostream>
#include <stdio.h>

typedef FARPROC(WINAPI *MyGetProcAddress)(HMODULE, LPCSTR);

void* FindExportAddress(HMODULE hModule, const char* funcName)
{

std::cout << "Finding " << funcName << " in module " << hModule << std::endl;
    // Access the PE headers of the loaded module
    IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)hModule;
    IMAGE_NT_HEADERS* ntHeaders = (IMAGE_NT_HEADERS*)((BYTE*)hModule + dosHeader->e_lfanew);
std::cout << "NT Headers: " << ntHeaders << std::endl;

    // Get the export directory
    IMAGE_EXPORT_DIRECTORY* exportDir = (IMAGE_EXPORT_DIRECTORY*)((BYTE*)hModule + ntHeaders->OptionalHeader.DataDirectory[0].VirtualAddress);
std::cout << "Export Directory: " << exportDir << std::endl;
std::cout << "--------------------------------------------------------" << std::endl;

    // Get the list of function names and addresses
    DWORD* nameRVAs = (DWORD*)((BYTE*)hModule + exportDir->AddressOfNames);
    WORD* ordRVAs = (WORD*)((BYTE*)hModule + exportDir->AddressOfNameOrdinals);
    DWORD* funcRVAs = (DWORD*)((BYTE*)hModule + exportDir->AddressOfFunctions);
std::cout << "Name RVAs: " << nameRVAs << std::endl;
std::cout << "Ord RVAs: " << ordRVAs << std::endl;
std::cout << "Func RVAs: " << funcRVAs << std::endl;
std::cout << "--------------------------------------------------------" << std::endl;

    // Search through the list of function names
    for (DWORD i = 0; i < exportDir->NumberOfNames; ++i)
    {
        char* funcNameFromExport = (char*)((BYTE*)hModule + nameRVAs[i]);
//std::cout << "Checking " << funcNameFromExport << std::endl;
        if (strcmp(funcNameFromExport, funcName) == 0)
        {

std::cout << "Found " << funcName << " at index " << i << std::endl;
            DWORD funcRVA = funcRVAs[ordRVAs[i]];
std::cout << "Function RVA: " << funcRVA << std::endl;
std::cout << "========================================================" << std::endl;
            return (void*)((BYTE*)hModule + funcRVA);

        }
    }
    return nullptr;  // Return nullptr if the function is not found

}

int main()
{
    // Load kernel32.dll dynamically
    HMODULE hKernel32 = LoadLibraryA("kernel32.dll");
    if (!hKernel32)
    {
        std::cerr << "Failed to load kernel32.dll" << std::endl;
        return 1;
    }   std::cout << "========================================================\nLoaded kernel32.dll" << std::endl;

    MyGetProcAddress pGetProcAddress;

    // Find the address of GetProcAddress manually
    pGetProcAddress = (MyGetProcAddress)FindExportAddress(hKernel32, "GetProcAddress");
    if (!pGetProcAddress)
    {
        std::cerr << "Failed to find GetProcAddress" << std::endl;
        return 1;
    }
    std::cout << "!!!!! GetProcAddress address: " << pGetProcAddress << std::endl;


    // Use GetProcAddress to get other function addresses dynamically
    FARPROC pCreateFile = pGetProcAddress(hKernel32, "CreateFileA");
    if (pCreateFile)
    {
        // Cast the address to the correct function pointer type
        typedef HANDLE(WINAPI *CreateFileFn)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
        CreateFileFn MyCreateFile = (CreateFileFn)pCreateFile;

        // Create the file before attempting to open it
        HANDLE hFile = MyCreateFile("myfile.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);

        // Now attempt to open the file
        hFile = MyCreateFile("myfile.txt", GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            std::cout << "Opened myfile.txt" << std::endl;
            CloseHandle(hFile);
        } 
        else std::cerr << "Failed to open myfile.txt" << std::endl;
    }
    else std::cerr << "Failed to find CreateFileA" << std::endl;
    std::cout << "========================================================" << std::endl;
    // Free the library
    FreeLibrary(hKernel32);

    return 0;
}
