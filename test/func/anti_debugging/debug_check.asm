;ml64 /c /Fl debug_check.asm

; Check if debugger is present by reading the PEB flag directly
; Compatible with x64 architecture

.code

IsDebuggerPresentASM PROC
    xor rax, rax                  ; Clear RAX to ensure higher bits are zero
    mov rcx, gs:[60h]             ; Get PEB base address from GS:[0x60]
    mov al, byte ptr [rcx+2]      ; Read BeingDebugged flag (offset 0x2)
    ret
IsDebuggerPresentASM ENDP


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


; DetectDebuggerTrapFlag PROC
;     pushfq                      ; Push RFLAGS to stack
;     or qword ptr [rsp], 0x100   ; Set Trap Flag (TF) in RFLAGS
;     popfq                       ; Restore modified RFLAGS

;     nop                         ; Executes normally if no debugger
;     pushfq                      ; Push flags to check if TF triggered
;     pop rax                     ; Retrieve flags

;     test rax, 0x100             ; Check if Trap Flag was set
;     setnz al                    ; AL = 1 if debugger is present
;     movzx rax, al               ; Zero-extend AL to RAX for return

;     ret
; DetectDebuggerTrapFlag ENDP


OverwriteDebugPort PROC
    mov     rax, gs:[60h]               ; Get PEB base address
    mov     rcx, 0                      ; Move 0 into RCX (temporary register)
    mov     [rax + 20h], rcx            ; Clear DebugPort by writing 0
    ret                                 ; Return from procedure
OverwriteDebugPort ENDP

CrashOnDebuggerAttach PROC
    mov rax, gs:[60h]          ; Get PEB base address
    mov rcx, 0FFFFFFFFFFFFFFFFh ; Load -1 into RCX
    mov [rax + 18h], rcx       ; Move RCX (-1) into DebugObjectHandle
    ret
CrashOnDebuggerAttach ENDP

END
