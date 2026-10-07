; Interrupt stubs for New-OS
; NASM syntax, 64-bit

%macro ISR_NOERR 1
global isr_stub_%1
isr_stub_%1:
    push 0              ; dummy error code
    push %1             ; interrupt number
    jmp isr_common
%endmacro

%macro ISR_ERR 1
global isr_stub_%1
isr_stub_%1:
    push %1             ; interrupt number (error code already pushed by CPU)
    jmp isr_common
%endmacro

%macro IRQ 2
global irq_stub_%1
irq_stub_%1:
    push 0
    push %2
    jmp irq_common
%endmacro

; CPU Exceptions
ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_ERR   30
ISR_NOERR 31

; IRQs
IRQ 0, 32       ; Timer
IRQ 1, 33       ; Keyboard

extern exception_handler
extern pic_eoi
extern keyboard_handler

isr_common:
    ; Save registers
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rdi, [rsp + 15*8]      ; interrupt number
    mov rsi, [rsp + 16*8]      ; error code
    call exception_handler

    ; Restore (won't reach here usually)
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    add rsp, 16                ; remove int_no and err_code
    iretq

irq_common:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rdi, [rsp + 15*8]      ; irq number (32 or 33)

    cmp rdi, 33
    jne .not_keyboard
    call keyboard_handler
.not_keyboard:

    ; Send EOI
    mov rdi, [rsp + 15*8]
    sub rdi, 32                ; convert to IRQ number 0-15
    call pic_eoi

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    add rsp, 16
    iretq
