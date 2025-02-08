;ml64 /c /Fl debug_check.asm

; Check if debugger is present by reading the PEB flag directly
; Compatible with x64 architecture

.code

IsDebuggerPresentASM PROC
    mov rax, gs:[60h]               ; Get PEB base address from GS:[0x60]
    mov al, byte ptr [rax+2]        ; Read BeingDebugged flag (offset 0x2)
    ret
IsDebuggerPresentASM ENDP

DetectDebuggerTrapFlag PROC
    pushfq                      ; Push RFLAGS to stack
    or qword ptr [rsp], 0x100   ; Set Trap Flag (TF) in RFLAGS
    popfq                       ; Restore modified RFLAGS

    nop                         ; Executes normally if no debugger
    pushfq                      ; Push flags to check if TF triggered
    pop rax                     ; Retrieve flags

    test rax, 0x100             ; Check if Trap Flag was set
    setnz al                    ; AL = 1 if debugger is present
    movzx rax, al               ; Zero-extend AL to RAX for return

    ret
DetectDebuggerTrapFlag ENDP

DetectHardwareBreakpointsASM PROC
    push rbx                        ; Save registers to avoid corrupting them

    mov rax, dr0
    mov rbx, dr1
    mov rcx, dr2
    mov rdx, dr3

    or rax, rbx                     
    or rax, rcx                     
    or rax, rdx

    setnz al                        ; If reg set, AL = 1, else AL = 0
    movzx rax, al                   ; Zero-extend AL to RAX

    pop rbx                         
DetectHardwareBreakpointsASM ENDP

END
