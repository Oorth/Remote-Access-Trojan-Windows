; Check if debugger is present by reading the PEB flag directly
; Compatible with x64 architecture

.code
IsDebuggerPresentASM PROC
    mov rax, gs:[60h]         ; Get PEB base address from GS:[0x60]
    mov al, byte ptr [rax+2]  ; Read the BeingDebugged flag (offset 0x2)
    ret
IsDebuggerPresentASM ENDP
END
