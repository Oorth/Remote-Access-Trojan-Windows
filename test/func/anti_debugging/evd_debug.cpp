//cl /EHsc .\evd_debug.cpp debug_check.obj /link user32.lib Advapi32.lib /OUT:evd_debug.exe
#include <windows.h>
#include <iostream>
#include <tlhelp32.h>
//#pragma comment(lib, "Advapi32.lib")

extern "C" BOOL IsDebuggerPresentASM();
extern "C" BOOL DetectHardwareBreakpointsASM();


typedef NTSTATUS(WINAPI* pNtSetInformationThread)(
    HANDLE ThreadHandle,
    ULONG ThreadInformationClass,
    PVOID ThreadInformation,
    ULONG ThreadInformationLength
);

void HideFromDebugger()                 //NO WORK!!!!!
{
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    if (ntdll) {
        auto NtSetInformationThread = (pNtSetInformationThread)GetProcAddress(ntdll, "NtSetInformationThread");
        if (NtSetInformationThread) {
            // ThreadInformationClass 0x11 = ThreadHideFromDebugger
            NtSetInformationThread(GetCurrentThread(), 0x11, NULL, 0);
        }
    }
}

bool DetectHardwareBreakpoints()
{
    CONTEXT ctx = {0};
    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
    
    if (GetThreadContext(GetCurrentThread(), &ctx))
    {
        if (ctx.Dr0 || ctx.Dr1 || ctx.Dr2 || ctx.Dr3) return true;
    }
    return false;
}

void ClearHardwareBreakpoints()
{
    CONTEXT ctx = {0};
    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
    
    if (GetThreadContext(GetCurrentThread(), &ctx))
    {
        ctx.Dr0 = 0;
        ctx.Dr1 = 0;
        ctx.Dr2 = 0;
        ctx.Dr3 = 0;
        ctx.Dr6 = 0;
        ctx.Dr7 = 0;

        SetThreadContext(GetCurrentThread(), &ctx);
    }
}

bool SelfDebugging()                        //[No work]
{
    DWORD pid = GetCurrentProcessId();
    std::cout << "PID: " << pid << std::endl;

    HANDLE MyHandle = GetCurrentProcess();
    HANDLE tokenHandle;

    // Step 1: Open the process token
    if (!OpenProcessToken(MyHandle, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &tokenHandle))
    {
        std::cerr << "Failed to open process token" << std::endl;
        return false;
    }
    std::cout << "Opened process token" << std::endl;

    // Step 2: Lookup the LUID for SeDebugPrivilege
    LUID luid;
    if (!LookupPrivilegeValueW(NULL, L"SeDebugPrivilege", &luid))
    {
        std::cerr << "Failed to lookup privilege value" << std::endl;
        CloseHandle(tokenHandle);
        return false;
    }
    std::cout << "Looked up privilege value" << std::endl;

    //Enable SeDebugPrivilege
    TOKEN_PRIVILEGES tp;
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(tokenHandle, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
    {
        std::cerr << "Failed to adjust token privileges" << std::endl;
        CloseHandle(tokenHandle);
        return false;
    }

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
    {
        std::cerr << "The token does not have the specified privilege." << std::endl;
        CloseHandle(tokenHandle);
        return false;
    }
    std::cout << "Privilege adjusted successfully." << std::endl;

    //Attempt to attach debugger to self
    if (DebugActiveProcess(pid))
    {
        std::cout << "Successfully attached debugger to self!" << std::endl;
        CloseHandle(tokenHandle);
        return true;
    }
    else
    {
        DWORD error = GetLastError();
        std::cerr << "Error while attaching debugger: " << error << std::endl;
        CloseHandle(tokenHandle);
        return false;
    }
}

// bool SelfDebugging()
// {
//     HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
//     if (!ntdll) {
//         std::cerr << "Failed to load NTDLL" << std::endl;
//         return false;
//     }

//     auto NtSetInformationThread = (pNtSetInformationThread)GetProcAddress(ntdll, "NtSetInformationThread");
//     if (!NtSetInformationThread) {
//         std::cerr << "Failed to get NtSetInformationThread" << std::endl;
//         return false;
//     }

//     NTSTATUS status = NtSetInformationThread(GetCurrentThread(), 0x11, NULL, 0); // 0x11 = ThreadHideFromDebugger
//     if (status == 0)
//     {
//         std::cout << "Successfully hid from debugger!" << std::endl;
//         return true;
//     } else {
//         std::cerr << "Failed to hide from debugger. NTSTATUS: " << status << std::endl;
//         return false;
//     }
// }


int main()
{

    // if(SelfDebugging()) std::cout << "Self Debugging" << std::endl;
    // else std::cout << "Not Self Debugging" << std::endl;

    // if (OverwriteDebugPort())
    // {
    //     std::cout << "Successfully Overwrote Debug Port!" << std::endl;
    // }
    // else
    // {
    //     std::cerr << "Failed to overwrite Debug Port!" << std::endl;
    // }

    while(1)
    {

        std::cout << "Start" << std::endl;
        //CrashOnDebuggerAttach();
        if (IsDebuggerPresentASM())
        {
            MessageBoxA(NULL, "Debugger detected!", "Debugger detected!", MB_ICONWARNING);
        }

        // if (DetectHardwareBreakpointsASM())
        // {
        //     MessageBoxA(NULL, "Debugger detected!", "Debugger detected!", MB_ICONWARNING);
        // }    

        // if (DetectHardwareBreakpoints() || IsDebuggerPresent())
        // {
        //     MessageBoxA(NULL, "Debugger detected!", "Debugger detected!", MB_ICONWARNING);
        //     ClearHardwareBreakpoints();
        //     return 1; // Exit if debugger is found  
        // }

        //MessageBoxA(NULL, "Debugger not detected!", "Debugger not detected!", MB_ICONINFORMATION);
        std::cout << "\nmoving on \n" << std::endl;
        Sleep(1000);
    }
    return 0;
}
