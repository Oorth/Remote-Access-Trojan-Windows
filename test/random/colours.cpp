#include <iostream>
#include <windows.h>

using namespace std;

int main()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // Reset color to default
    SetConsoleTextAttribute(hConsole, 7);
    cout << "This word is ";

    // Set color to red
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED); 
    cout << "red" << endl;

    SetConsoleTextAttribute(hConsole, 7);
    system("pause");

    return 0;
}