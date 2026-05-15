bits 64
extern web_brain

section .data
    intent_expl:  db "explique", 0
    intent_expl2: db "oque", 0
    intent_expl3: db "explicar?", 0
    intent_expl4: db "oque?", 0
    corrigir1:    db "/CORRIGIR1", 0
    corrigir2:    db "/CORRIGIR2", 0
    corrigir3:    db "/CORRIGIR3", 0
    gerar:        db "gerar", 0
    verificar:    db "verifique", 0
    execute: db "execute", 0
    um: db "um", 0
    a:  db "a", 0
    e:  db "e", 0
    i:  db "i", 0
    o:  db "o", 0
    u:  db "u", 0

section .bss
    buffer:      resb 1024
    bufferword:  resb 1024
    bufferfinal: resb 1024
    token:       resb 64

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
    mov byte [buffer + rax - 1], 0
    mov r12, rax
    xor rbx, rbx
    xor rdi, rdi
    xor r13, r13
    xor r11, r11
    mov byte [bufferfinal], 0
    call parser
    lea rsi, [bufferword]
    lea rdi, [bufferfinal + 1]
.copy_to_final:
    mov al, [rsi]
    mov [rdi], al
    inc rsi
    inc rdi
    cmp al, 0
    jne .copy_to_final
    mov rdi, bufferfinal
    call web_brain
.exit_err:
    pop rbp
    mov rax, 0
    ret

parser:
    cmp rbx, r12
    jae .force_end_token
    mov al, [buffer + rbx]
    cmp al, ' '
    je .handle_jmp
.haschar:
    mov [token + rdi], al
    inc rdi
    inc rbx
    cmp rbx, r12
    jae .force_end_token
    mov al, [buffer + rbx]
    cmp al, ' '
    je .end_token
    jmp .haschar 
.handle_jmp:
    inc rbx
    jmp parser
.force_end_token:
    cmp rdi, 0
    je .end_parser
.end_token:
    mov byte [token + rdi], 0
    lea rsi, [token]
    lea rdi, [um]
    call strcmp
    jc remove_token
    lea rsi, [token]
    lea rdi, [a]
    call strcmp
    jc remove_token
    lea rsi, [token]
    lea rdi, [e]
    call strcmp
    jc remove_token
    lea rsi, [token]
    lea rdi, [i]
    call strcmp
    jc remove_token
    lea rsi, [token]
    lea rdi, [o]
    call strcmp
    jc remove_token
    lea rsi, [token]
    lea rdi, [u]
    call strcmp
    jc remove_token
    lea rsi, [token]
    lea rdi, [intent_expl]
    call strcmp
    mov r11, 1
    jc selectintent
    lea rsi, [token]
    lea rdi, [intent_expl4]
    call strcmp
    mov r11, 1
    jc selectintent
    lea rsi, [token]
    lea rdi, [intent_expl2]
    call strcmp
    mov r11, 1
    jc selectintent
    lea rsi, [token]
    lea rdi, [intent_expl3]
    call strcmp
    mov r11, 1
    jc selectintent
    lea rsi, [token]
    lea rdi, [corrigir1]
    call strcmp
    mov r11, 2
    jc selectintent
    lea rsi, [token]
    lea rdi, [corrigir2]
    call strcmp
    mov r11, 3
    jc selectintent
    lea rsi, [token]
    lea rdi, [corrigir3]
    call strcmp
    mov r11, 4
    jc selectintent
    lea rsi, [token]
    lea rdi, [verificar]
    call strcmp
    mov r11, 5
    jc selectintent
    lea rsi, [token]
    lea rdi, [gerar]
    call strcmp
    mov r11, 6
    lea rsi, [token]
    lea rdi, [execute]
    call strcmp
    mov r11, 7
    jc selectintent
    lea rsi, [token]
    xor rcx, rcx
.copy_loop:
    mov al, [rsi + rcx]
    cmp al, 0
    je .copy_done
    mov [bufferword + r13], al
    inc r13
    inc rcx
    jmp .copy_loop
.copy_done:
    mov byte [bufferword + r13], ' '
    inc r13
    xor rdi, rdi
    jmp parser
.end_parser:
    cmp r13, 0
    je .real_end
    mov byte [bufferword + r13 - 1], 0 
.real_end:
    ret

remove_token:
    xor rdi, rdi
    inc rbx
    jmp parser

strcmp:
    push rax
    push rbx
    xor rcx, rcx
.loop:
    mov al, [rsi + rcx]
    mov bl, [rdi + rcx]
    cmp al, bl
    jne .not_equal
    cmp al, 0
    je .equal
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

selectintent:
    cmp r11, 1
    je .intentexplicar
    cmp r11, 2
    je .intentcorrigir1
    cmp r11, 3
    je .intentcorrigir2
    cmp r11, 4
    je .intentcorrigir3
    cmp r11, 5
    je .intentverificar
    cmp r11, 6
    je .intentgerar
    cmp r11, 7
    je .intentexecute
    jmp remove_token
.intentexplicar:
    mov [bufferfinal], 0x01
    jmp remove_token
.intentcorrigir1:
    mov [bufferfinal], 0x02
    jmp remove_token
.intentcorrigir2:
    mov [bufferfinal], 0x03
    jmp remove_token
.intentcorrigir3:
    mov [bufferfinal], 0x04
    jmp remove_token
.intentverificar:
    mov [bufferfinal], 0x05
    jmp remove_token
.intentgerar:
    mov [bufferfinal], 0x06
    jmp remove_token
.intentexecute:
    mov [bufferfinal], 0x07
    jmp remove_token
    
