//cl /EHsc .\obfs.cpp /link /OUT:obfs.exe
#include <iostream>
#include <windows.h>
#define X_C(c) static_cast<char>((c) ^ 0xFF)

int main()
{

    char obfuscatedUser32[] = { X_C('u'), X_C('s'), X_C('e'), X_C('r'), X_C('3'), X_C('2'), X_C('.'), X_C('d'), X_C('l'), X_C('l'), 0 };
    char obfuscatedSetWinHookEx[] = { X_C('S'), X_C('e'), X_C('t'), X_C('W'), X_C('i'), X_C('n'), X_C('d'), X_C('o'), X_C('w'), X_C('s'), X_C('H'), X_C('o'), X_C('o'), X_C('k'), X_C('E'), X_C('x'), 0 };
    char obfuscatedUnhookWinHookEx[] = { X_C('U'), X_C('n'), X_C('h'), X_C('o'), X_C('o'), X_C('k'), X_C('W'), X_C('i'), X_C('n'), X_C('d'), X_C('o'), X_C('w'), X_C('s'), X_C('H'), X_C('o'), X_C('o'), X_C('k'), X_C('E'), X_C('x'), 0 };
    char obfuscatedCallNxtHookEx[] = { X_C('C'), X_C('a'), X_C('l'), X_C('l'), X_C('N'), X_C('e'), X_C('x'), X_C('t'), X_C('H'), X_C('o'), X_C('o'), X_C('k'), X_C('E'), X_C('x'), 0 };

    for (int i = 0; obfuscatedUser32[i] != 0; i++) obfuscatedUser32[i] ^= 0xFF;
    for (int i = 0; obfuscatedSetWinHookEx[i] != 0; i++) obfuscatedSetWinHookEx[i] ^= 0xFF;
    for (int i = 0; obfuscatedUnhookWinHookEx[i] != 0; i++) obfuscatedUnhookWinHookEx[i] ^= 0xFF;
    for (int i = 0; obfuscatedCallNxtHookEx[i] != 0; i++) obfuscatedCallNxtHookEx[i] ^= 0xFF;

    std::cout << obfuscatedUser32 << std::endl;
    std::cout << obfuscatedSetWinHookEx << std::endl;
    std::cout << obfuscatedUnhookWinHookEx << std::endl;
    std::cout << obfuscatedCallNxtHookEx << std::endl;

    
    HMODULE hUser32 = (HMODULE)LoadLibraryA(obfuscatedUser32);
    if (hUser32 == NULL) {
        std::cout << "Failed to load user32.dll" << std::endl;
        return 1;
    } std::cout << "[Loaded user32.dll]" << std::endl;
    FreeLibrary(hUser32);
    std::cout << "[Unloaded user32.dll]" << std::endl;

    SecureZeroMemory(obfuscatedUser32, sizeof(obfuscatedUser32));
    SecureZeroMemory(obfuscatedSetWinHookEx, sizeof(obfuscatedSetWinHookEx));
    SecureZeroMemory(obfuscatedUnhookWinHookEx, sizeof(obfuscatedUnhookWinHookEx));
    SecureZeroMemory(obfuscatedCallNxtHookEx, sizeof(obfuscatedCallNxtHookEx));

    std::cout << " |||| "<< obfuscatedUser32 << std::endl;

    return 0;
}