//cl /EHsc .\evd_debug.cpp debug_check.obj /link user32.lib /OUT:evd_debug.exe
#include <windows.h>
#include <iostream>

extern "C" BOOL IsDebuggerPresentASM();
extern "C" BOOL DetectHardwareBreakpointsASM();

bool SelfDebug()
{
    if (DebugActiveProcess(GetCurrentProcessId()))
    {
        std::cout << "attached selfDebug" << std::endl;
        return true;                                        //successfully attached to ourselves
    }
    std::cout<<"not attached selfDebug"<<std::endl;
    return false;                                           //a debugger is already attached
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


int main()
{

    //SelfDebug();          [BROKEN]

    while(1)
    {
        // if (DetectDebuggerByTiming())
        // {
        //     MessageBoxA(NULL, "Debugger detected!", "Debugger detected!", MB_ICONWARNING);
        //     return 1; // Exit if debugger is found
        // }

        //MessageBoxA(NULL, "Start", "Start", MB_ICONINFORMATION);
        std::cout << "Start" << std::endl;

        // if (IsDebuggerPresentASM())
        // {
        //     MessageBoxA(NULL, "Debugger detected!", "Debugger detected!", MB_ICONWARNING);
        // }

        if (DetectHardwareBreakpointsASM())
        {
            MessageBoxA(NULL, "Debugger detected!", "Debugger detected!", MB_ICONWARNING);
        }

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
