# ============================================
# hello_world.asm
# ============================================
.data
msg: .asciiz "Hello, World!\n"

.text
.globl main
main:
    # Print string syscall
    li $v0, 4
    la $a0, msg
    syscall
    
    # Exit syscall
    li $v0, 10
    syscall
    nop

