#include <windows.h>
#include <iostream>
// #include <stdio.h>
#include <string>
#include <thread>

using namespace std;

// Get the DLL file name from the command-line argument
//LPSTR dll_file = argv[1];
LPCSTR dll_file = "test_lib.dll";                                                      //!!!!!!!!!!!!!!!NOte the datatype!!!!!!!!!!!!!!!!!!!

void func()
{
    HINSTANCE hDLL = LoadLibraryA(dll_file);
    if (hDLL == NULL)
    {
        printf("Failed to load DLL: %d\n", GetLastError());
        //return EXIT_FAILURE;
    }
    printf("\n\nDLL loaded successfully!\n\n");
    FreeLibrary(hDLL);
}

int main(int argc, char* argv[])
{

    //thread t(func);
    //t.join();

    //Load the DLL
    HINSTANCE hDLL = LoadLibraryA(dll_file);
    if (hDLL == NULL)
    {
        printf("Failed to load DLL: %d\n", GetLastError());
        return EXIT_FAILURE;
    }

    FreeLibrary(hDLL);

    return EXIT_SUCCESS;
}

