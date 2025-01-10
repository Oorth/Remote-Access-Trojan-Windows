#include <iostream>
#include <windows.h>
#include <tlhelp32.h>

// Function to find a process by name and return its process ID
DWORD GetProcessIDByName(const wchar_t* processName) {
    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32W);

    // Create a snapshot of all running processes
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create snapshot" << std::endl;
        return 0;
    }

    // Iterate over all processes
    if (Process32FirstW(hSnapshot, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, processName) == 0) {  // Use _wcsicmp for wide strings
                CloseHandle(hSnapshot);
                return pe.th32ProcessID;  // Return the process ID
            }
        } while (Process32NextW(hSnapshot, &pe));  // Use Process32NextW for wide strings
    }

    CloseHandle(hSnapshot);
    return 0;  // Not found
}

int main()
{
    // Step 1: Get the process ID of CMD (using wide string for Unicode)
    DWORD pid = GetProcessIDByName(L"cmd.exe");
    if (pid == 0) {
        std::cerr << "CMD not found!" << std::endl;
        return 1;
    }

    std::cout << "CMD PID: " << pid << std::endl;

    // Step 2: Open the CMD process with necessary permissions
    HANDLE hProcess = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_CREATE_THREAD, FALSE, pid);
    if (hProcess == NULL) {
        std::cerr << "Failed to open CMD process!" << std::endl;
        return 1;
    }

    // Step 3: Allocate memory for the simplified shellcode
    LPVOID pRemoteMemory = VirtualAllocEx(hProcess, NULL, 1024, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (pRemoteMemory == NULL) {
        std::cerr << "Failed to allocate memory in target process!" << std::endl;
        CloseHandle(hProcess);
        return 1;
    }

    // Step 4: Write a simplified shellcode that just exits the thread
    unsigned char simpleExitShellcode[] = {
        0xC3 // return instruction to exit the thread
    };

    // Step 5: Write the shellcode into the allocated memory
    BOOL writeResult = WriteProcessMemory(hProcess, pRemoteMemory, simpleExitShellcode, sizeof(simpleExitShellcode), NULL);
    if (!writeResult) {
        std::cerr << "Failed to write memory to CMD!" << std::endl;
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    std::cout << "Simplified shellcode written to memory." << std::endl;

    // Step 6: Create a remote thread to execute the shellcode
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pRemoteMemory, NULL, 0, NULL);
    if (hThread == NULL) {
        std::cerr << "Failed to create remote thread in CMD!" << std::endl;
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    std::cout << "Remote thread created." << std::endl;

    // Wait for the thread to finish
    WaitForSingleObject(hThread, INFINITE);

    // Step 7: Check the exit code of the remote thread
    DWORD threadExitCode = 0;
    if (GetExitCodeThread(hThread, &threadExitCode)) {
        std::cout << "Remote thread final exit code: " << threadExitCode << std::endl;
    } else {
        std::cerr << "Failed to get remote thread exit code!" << std::endl;
    }

    // Clean up
    VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    std::cout << "Simplified shellcode executed in CMD process!" << std::endl;

    return 0;
}
