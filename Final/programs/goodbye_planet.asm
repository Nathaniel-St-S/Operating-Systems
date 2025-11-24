# ============================================
# goodbye_planet.asm
# ============================================
.data
goodbye: .asciiz "Goodbye, Planet Earth!\n"
countdown_msg: .asciiz "Countdown: "

.text
.globl main
main:
    # Print countdown message
    li $v0, 4
    la $a0, countdown_msg
    syscall
    
    # Countdown from 3
    li $t0, 3
countdown_loop:
    beq $t0, $zero, say_goodbye
    nop                   # Delay slot
    
    # Print number
    move $a0, $t0
    li $v0, 1
    syscall
    
    # Decrement
    addiu $t0, $t0, -1
    j countdown_loop
    nop                   # Delay slot
    
say_goodbye:
    # Print goodbye message
    li $v0, 4
    la $a0, goodbye
    syscall
    
    # Exit
    li $v0, 10
    syscall
    nop

