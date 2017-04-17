global long_mode_start
global halt_wrapper
global VGA_clear
global VGA_display_char
global VGA_display_str

extern kmain

section .text
bits 64
long_mode_start:
    ; load 0 into all data segment registers
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

	;print OKAY
	mov rax, 0x2f592f412d4b2d4f
    mov qword [0xb8000], rax

    call kmain
    hlt

halt_wrapper:
    hlt

VGA_clear:
    ; set the value of rax to 4 ascii spaces
    mov rax, 0x0720072007200720
    ;initialize the counter to 0 in rbx
    mov rbx, 0
    ;initalize rcx to the beginning of the vga array
    mov rcx, 0xb8000
LOOP_START:
    mov qword [rcx], rax ;4
    add rcx, 0x8
    inc rbx
    cmp rbx, 500
    jl LOOP_START

    ret

VGA_display_char:
    ; preserve rax
    push rax
    ; load the current vga buffer pointer
    mov rax, [vga_buf_cur]

    ; check if the character is a newline
    cmp rdi, [newline_char]
    je VGA_print_newline

    ; format the register of the input variable to be white
    or rdi, [white_on_black]
    ; print the character to the current terminal position
    mov [rax], rdi
    ; increment the current terminal postion
    inc rax
    inc rax
    ; save the current terminal position
    mov [vga_buf_cur], rax

disp_char_end:
    ; restore rax
    pop rax
    ret

VGA_print_newline:
    ; set rbx to every newline until that address is past
    ; the current buffer pointer
    ; then set the current buffer pointer to the rbx
    mov rbx, [vga_buf_beg]
nl_loop:
    add byte rbx, [vga_buf_line_len]
    cmp rbx, rax
    jna nl_loop

    mov [vga_buf_cur], rbx
    jmp disp_char_end

VGA_display_str:
    ; preserve registers
    push rax

    mov rax, rdi
disp_loop:
    ; grab the character from the current string pointer
    mov rdi, [rax]
    and rdi, 0xff
    
    ; check for null terminator
    cmp rdi, 0
    
    ; if null terminator, finish the routine
    je disp_end

    ; else print the character
    call VGA_display_char
    inc rax
    jmp disp_loop
disp_end:
    pop rax
    ret

section .data
bits 64
vga_buf_beg DQ 0xb8000
vga_buf_cur DQ 0xb8000
vga_buf_line_len DQ 0xa0
vga_char_len DD 0x2
white_on_black DD 0x0700
newline_char DB 0x0a