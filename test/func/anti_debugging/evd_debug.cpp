#include <windows.h>
#include <iostream>
#include <tlhelp32.h>
#include <psapi.h>
#include <thread>

extern "C" BOOL IsDebuggerPresentASM();
extern "C" BOOL DetectHardwareBreakpointsASM();

///////////////////////////////////////////////////////////////////////////////
#define Use_IsDebuggerPresent 0                     //[Works]
#define Use_IsDebuggerPresentASM 1                  //[Works]

#define Use_DetectSoftwareBreakpoints 0             //[same Always shows debugger is present :( ]
#define Use_DetectHardwareBreakpoints 0             //[Works]
#define Use_DetectHardwareBreakpointsASM 0          //[ C0000096 exception_priv_instruction, NEEDS DRIVER ]
#define Use_ClearHardwareBreakpoints 0              //[ Might be working ]

#define Use_SelfDebugging 0
///////////////////////////////////////////////////////////////////////////////

volatile bool exitProgram = false;

typedef NTSTATUS(WINAPI* pNtSetInformationThread)
(
    HANDLE ThreadHandle,
    ULONG ThreadInformationClass,
    PVOID ThreadInformation,
    ULONG ThreadInformationLength
);

void HideFromDebugger()                 //NO WORK!!!!!
{
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    if (ntdll)
    {
        auto NtSetInformationThread = (pNtSetInformationThread)GetProcAddress(ntdll, "NtSetInformationThread");
        if (NtSetInformationThread)
        {
            // ThreadInformationClass 0x11 = ThreadHideFromDebugger
            NTSTATUS status = NtSetInformationThread(GetCurrentThread(), 0x11, NULL, 0);
            if (status != 0)
            {
                std::cerr << "Failed to hide from debugger. NTSTATUS: " << status << std::endl;
            }
        }
    }
}

bool DetectHardwareBreakpoints()
{
    CONTEXT ctx = {0};
    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;

    if (!GetThreadContext(GetCurrentThread(), &ctx))
    {
        std::cerr << "Failed to get thread context." << std::endl;
        return false;
    }

    if (GetThreadContext(GetCurrentThread(), &ctx))
    {
        if (ctx.Dr0 || ctx.Dr1 || ctx.Dr2 || ctx.Dr3) return true;
    }
    return false;
}

BOOL ClearHardwareBreakpoints()
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
        return true;
    }
    std::cerr << "Failed to clear hardware breakpoints." << std::endl;
    return false;
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

    if (!AdjustTokenPrivileges(tokenHandle, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL) || GetLastError() != ERROR_SUCCESS)
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
    if (!DebugActiveProcess(pid))
    {
        DWORD error = GetLastError();
        std::cerr << "Error while attaching debugger: " << error << std::endl;
        CloseHandle(tokenHandle);
        return false;
    }
    std::cout << "Successfully attached debugger to self!" << std::endl;
    CloseHandle(tokenHandle);
    return true;
}

bool SelfDebugging2()
{
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    if (!ntdll) {
        std::cerr << "Failed to load NTDLL" << std::endl;
        return false;
    }

    auto NtSetInformationThread = (pNtSetInformationThread)GetProcAddress(ntdll, "NtSetInformationThread");
    if (!NtSetInformationThread) {
        std::cerr << "Failed to get NtSetInformationThread" << std::endl;
        return false;
    }

    NTSTATUS status = NtSetInformationThread(GetCurrentThread(), 0x11, NULL, 0); // 0x11 = ThreadHideFromDebugger
    if (status == 0)
    {
        std::cout << "Successfully hid from debugger!" << std::endl;
        return true;
    } else {
        std::cerr << "Failed to hide from debugger. NTSTATUS: " << status << std::endl;
        return false;
    }
}

bool GetModuleInfo(HMODULE &hModule, MODULEINFO &modInfo)
{
    hModule = GetModuleHandle(NULL);  // Get handle to the current executable
    if (!hModule) {
        std::cerr << "Failed to get module handle." << std::endl;
        return false;
    }

    if (!GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(MODULEINFO))) {
        std::cerr << "Failed to get module information." << std::endl;
        return false;
    }

    return true;
}


bool DetectSoftwareBreakpoints()
{
    HMODULE hModule;
    MODULEINFO modInfo;

    // Get base address and size of the current module
    if (!GetModuleInfo(hModule, modInfo))
    {
        return false;
    }

    BYTE* baseAddress = static_cast<BYTE*>(modInfo.lpBaseOfDll);  // Start address
    SIZE_T moduleSize = modInfo.SizeOfImage;                      // Size of the module

    std::cout << "Scanning module from: " << static_cast<void*>(baseAddress)
              << " to " << static_cast<void*>(baseAddress + moduleSize) << std::endl;

    // Loop through the code section looking for 0xCC
    for (SIZE_T i = 0; i < moduleSize; ++i)
    {
        if (*(baseAddress + i) == 0xCC)
        {
            std::cerr << "Software breakpoint detected at address: "
                      << static_cast<void*>(baseAddress + i) << std::endl;
            return true;  // Breakpoint found
        }
    }

    std::cout << "No software breakpoints detected." << std::endl;
    return false;  // No breakpoints found
}

void DebuggingThread()
{
    while(!exitProgram)
    {
        #if Use_DetectSoftwareBreakpoints
            if (DetectSoftwareBreakpoints())
            {
                std::cout << "Debugger detected!" << std::endl;
                exitProgram = true;
            }
        #endif

        #if Use_DetectHardwareBreakpoints
            if (DetectHardwareBreakpoints())
            {
                std::cout << "Debugger detected!" << std::endl;
                //exitProgram = true;
            }
        #endif

        #if Use_DetectHardwareBreakpointsASM
            if (DetectHardwareBreakpointsASM())
            {
                std::cout << "Debugger detected!" << std::endl;
                exitProgram = true;
            }
        #endif

        #if Use_IsDebuggerPresent
            if (IsDebuggerPresent())
            {
                std::cout << "Debugger detected! from IsDebuggerPresent" << std::endl;
                exitProgram = true; // Exit if debugger is found  
            }
        #endif

        #if Use_IsDebuggerPresentASM
            if (IsDebuggerPresentASM())
            {
                std::cout << "Debugger detected! from Use_IsDebuggerPresentASM" << std::endl;
                exitProgram = true;
            }
        #endif

        #if Use_ClearHardwareBreakpoints
            if (DetectHardwareBreakpoints())    
            {
                std::cout << "Debugger detected!" << std::endl;

                if(ClearHardwareBreakpoints()) std::cout << "Hardware breakpoints cleared!" << std::endl;
                else std::cout << "Failed to clear hardware breakpoints!" << std::endl;
            }
        #endif

        Sleep(1000);
    }
}

int main()
{
    #if Use_SelfDebugging
        if(SelfDebugging()) std::cout << "Self Debugging" << std::endl;
        else std::cout << "Not Self Debugging" << std::endl;
    #endif

    std::thread debugThread(DebuggingThread);
    int i=0;
    while (!exitProgram)
    {
        std::cout << "Main Thread: " << i++ << std::endl;
        Sleep(1000);
    }
    
    debugThread.join();
    return 0;
}
