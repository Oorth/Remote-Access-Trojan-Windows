#include <windows.h>
#include <iostream>

int main()
{
    HINSTANCE hinstLib;
    BOOL fFreeResult;

    // Load the DLL
    hinstLib = LoadLibrary(TEXT("cute_lib.dll"));

    // If the handle is valid, the DLL was loaded successfully
    if (hinstLib != NULL) {
        std::cout << "DLL loaded successfully!" << std::endl;

        // Free the DLL module
        fFreeResult = FreeLibrary(hinstLib);

        // Check if the DLL was freed successfully
        if (fFreeResult) {
            std::cout << "DLL freed successfully!" << std::endl;
        } else {
            std::cerr << "Failed to free the DLL." << std::endl;
        }
    } else {
        std::cerr << "Failed to load the DLL." << std::endl;
    }

    return 0;
}