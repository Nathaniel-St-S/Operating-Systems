# ============================================
# factorial.asm
# ============================================
.data
result_msg: .asciiz "Factorial result: "
newline: .asciiz "\n"

.text
.globl main
main:
    # Calculate 5!
    li $a0, 5
    jal factorial
    nop                    # Branch delay slot preserved!

    # Print "Factorial result: "
    move $t0, $v0          # Save factorial result
    la $a0, result_msg
    li $v0, 4
    syscall

    # Print numeric result
    move $a0, $t0
    li $v0, 1
    syscall

    # Print newline
    la $a0, newline
    li $v0, 4
    syscall
    
    # Exit
    li $v0, 10
    syscall
    nop

factorial:
    # Base case: if n < 2, return 1
    slti $t0, $a0, 2
    beq $t0, $zero, recursive
    li $v0, 1             # Delay slot - return 1
    jr $ra
    nop                   # Delay slot
    
recursive:
    # Save registers
    addiu $sp, $sp, -8
    sw $ra, 4($sp)
    sw $a0, 0($sp)
    
    # Call factorial(n-1)
    addiu $a0, $a0, -1
    jal factorial
    nop                   # Delay slot preserved
    
    # Restore and multiply
    lw $a0, 0($sp)
    lw $ra, 4($sp)
    addiu $sp, $sp, 8
    
    mult $a0, $v0
    mflo $v0
    
    jr $ra
    nop                   # Delay slot
