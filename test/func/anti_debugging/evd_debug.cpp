//cl /EHsc .\evd_debug.cpp /link user32.lib /OUT:evd_debug.exe
#include <windows.h>
#include <stdio.h>

// bool DetectDebuggerByTiming()        [NAH]
// {
//     LARGE_INTEGER freq, start, end;
//     QueryPerformanceFrequency(&freq);
//     QueryPerformanceCounter(&start);

//     // Simple operation (debugger single-stepping will slow it)
//     for (volatile int i = 0; i < 100000; i++);

//     QueryPerformanceCounter(&end);
//     double elapsedTime = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;

//     return (elapsedTime > 0.0002); // If too slow, debugger is likely present
// }

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
    
    if (GetThreadContext(GetCurrentThread(), &ctx)) {
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
    // if (DetectDebuggerByTiming())
    // {
    //     MessageBoxA(NULL, "Debugger detected!", "Debugger detected!", MB_ICONWARNING);
    //     return 1; // Exit if debugger is found
    // }

    if (DetectHardwareBreakpoints() || IsDebuggerPresent())
    {
        MessageBoxA(NULL, "Debugger detected!", "Debugger detected!", MB_ICONWARNING);
        ClearHardwareBreakpoints();
        return 1; // Exit if debugger is found
    }

    //MessageBoxA(NULL, "Debugger not detected!", "Debugger not detected!", MB_ICONINFORMATION);
    MessageBoxA(NULL, "Moving ahead ", "Debugger not detected!", MB_ICONINFORMATION);

    return 0;
}
