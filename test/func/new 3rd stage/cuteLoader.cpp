//cl /EHsc .\cuteLoader.cpp /link /OUT:cuteLoader.exe
#include <windows.h>
#include <iostream>

LPCSTR dllPath = "C:\\malware\\RAT Windows\\test\\func\\new 3rd stage\\target_code.dll";

typedef int (*pmain_stuff)();
pmain_stuff main_stuff;

int main()
{
    HINSTANCE hDLL = LoadLibraryA(dllPath);
    if (hDLL == nullptr)
    {
        std::cerr << "Failed to load DLL: " << GetLastError() << std::endl;
        return 1;
    }
    std::cout << "Loaded dll" << std::endl;

    main_stuff = (pmain_stuff)GetProcAddress(hDLL, "?main_thing@@YAHXZ");
    if (!main_stuff)
    {
        std::cerr << "Failed to get main stuff address "<< GetLastError() << std::endl;
        FreeLibrary(hDLL);
        return 1;
    }
    //std::cout << "got path " << std::endl;

    main_stuff();

    FreeLibrary(hDLL); std::cout << "Freed hDLL " << std::endl;
    return 0;
}