bits 64
extern web_brain

section .data
    intent_expl:    db "explique", 0
    intent_verif:   db "verifique", 0
    intent_gerar:   db "gerar", 0
    
    lang_python:    db "python", 0
    lang_cpp:       db "cpp", 0
    lang_asm:       db "asm", 0

    um: db "um", 0
    a:  db "a", 0
    o:  db "o", 0

section .bss
    buffer:         resb 1024
    bufferfinal:    resb 1024
    token:          resb 64

section .text
    global main

main:
    push rbp
    
    mov rax, 0
    mov rdi, 0
    mov rsi, buffer
    mov rdx, 1024
    syscall

    cmp rax, 1
    jle .exit_err
    
    mov r12, rax
    mov byte [buffer + rax - 1], 0
    
    xor rbx, rbx
    mov byte [bufferfinal], 0
    mov byte [bufferfinal + 1], 0

.main_loop:
    call get_next_token
    cmp rdi, 0
    je .call_brain

    lea rsi, [token]
    lea rdi, [intent_verif]
    call strcmp
    jc .handle_verificar

    lea rsi, [token]
    lea rdi, [intent_expl]
    call strcmp
    jc .handle_explicar

    jmp .main_loop

.handle_verificar:
    mov byte [bufferfinal], 0x05
    call get_next_token
    
    lea rsi, [token]
    lea rdi, [lang_python]
    call strcmp
    jc .set_py

    lea rsi, [token]
    lea rdi, [lang_cpp]
    call strcmp
    jc .set_cpp

    jmp .copy_raw_rest

.set_py:
    mov byte [bufferfinal + 1], 0x10
    jmp .copy_raw_rest

.set_cpp:
    mov byte [bufferfinal + 1], 0x20
    jmp .copy_raw_rest

.handle_explicar:
    mov byte [bufferfinal], 0x01
    jmp .main_loop

.copy_raw_rest:
    lea rdi, [bufferfinal + 2]
.raw_loop:
    cmp rbx, r12
    jae .call_brain
    mov al, [buffer + rbx]
    mov [rdi], al
    inc rbx
    inc rdi
    jmp .raw_loop

.call_brain:
    mov rdi, bufferfinal
    call web_brain

.exit_err:
    pop rbp
    mov rax, 0
    ret

get_next_token:
    xor rdi, rdi
.skip_spaces:
    cmp rbx, r12
    jae .end_get
    mov al, [buffer + rbx]
    cmp al, ' '
    jne .collect
    inc rbx
    jmp .skip_spaces
.collect:
    mov [token + rdi], al
    inc rdi
    inc rbx
    cmp rbx, r12
    jae .end_get
    mov al, [buffer + rbx]
    cmp al, ' '
    je .end_get
    jmp .collect
.end_get:
    mov byte [token + rdi], 0
    ret

strcmp:
    push rax
    push rbx
    xor rcx, rcx
.loop:
    mov al, [rsi + rcx]
    mov bl, [rdi + rcx]
    cmp al, bl
    jne .not_equal
    test al, al
    jz .equal
    inc rcx
    jmp .loop
.equal:
    stc
    jmp .done
.not_equal:
    clc
.done:
    pop rbx
    pop rax
    ret
