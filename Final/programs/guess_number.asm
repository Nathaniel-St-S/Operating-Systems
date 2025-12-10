# ============================================
# guess_number.asm - Number Guessing Game
# ============================================
# A simple interactive game - guess a number between 1 and 100
# Great fallback if terminal games have display issues
# ============================================

.data
msg_welcome: .asciiz "\n╔════════════════════════════════════╗\n"
msg_welcome2: .asciiz "║   GUESS THE NUMBER GAME (1-100)    ║\n"
msg_welcome3: .asciiz "╚════════════════════════════════════╝\n\n"
msg_prompt: .asciiz "Enter your guess (or 0 to quit): "
msg_too_low: .asciiz "Too low! Try again.\n\n"
msg_too_high: .asciiz "Too high! Try again.\n\n"
msg_correct: .asciiz "\n🎉 Congratulations! You got it!\n"
msg_attempts: .asciiz "It took you "
msg_attempts2: .asciiz " attempts.\n"
msg_quit: .asciiz "\nThanks for playing! The number was "
msg_newline: .asciiz "\n"
msg_play_again: .asciiz "\nPlay again? (1=Yes, 0=No): "
msg_goodbye: .asciiz "\nGoodbye! Thanks for playing!\n\n"

secret_number: .word 0
attempts: .word 0
playing: .word 1

.text
.globl main

main:
game_start:
    # Generate random number between 1 and 100
    li $v0, 14              # get time
    syscall
    li $t1, 100
    divu $v0, $t1
    mfhi $t0
    addiu $t0, $t0, 1       # Make it 1-100 instead of 0-99
    sw $t0, secret_number
    
    # Reset attempts
    sw $zero, attempts
    
    # Print welcome message
    li $v0, 4
    la $a0, msg_welcome
    syscall
    la $a0, msg_welcome2
    syscall
    la $a0, msg_welcome3
    syscall

guess_loop:
    # Print prompt
    li $v0, 4
    la $a0, msg_prompt
    syscall
    
    # Read integer
    li $v0, 5               # syscall 5: read integer
    syscall
    move $t1, $v0           # Store guess in $t1
    
    # Check for quit (0)
    beq $t1, $zero, quit_game
    nop
    
    # Increment attempts
    lw $t2, attempts
    addiu $t2, $t2, 1
    sw $t2, attempts
    
    # Load secret number
    lw $t0, secret_number
    
    # Compare guess with secret
    beq $t1, $t0, correct_guess
    nop
    blt $t1, $t0, too_low
    nop
    
    # Too high
    li $v0, 4
    la $a0, msg_too_high
    syscall
    j guess_loop
    nop

too_low:
    li $v0, 4
    la $a0, msg_too_low
    syscall
    j guess_loop
    nop

correct_guess:
    # Print congratulations
    li $v0, 4
    la $a0, msg_correct
    syscall
    
    # Print attempts
    la $a0, msg_attempts
    syscall
    lw $a0, attempts
    li $v0, 1
    syscall
    li $v0, 4
    la $a0, msg_attempts2
    syscall
    
    # Ask to play again
    la $a0, msg_play_again
    syscall
    li $v0, 5
    syscall
    
    bne $v0, $zero, game_start
    nop
    
    j end_game
    nop

quit_game:
    # Show the secret number
    li $v0, 4
    la $a0, msg_quit
    syscall
    lw $a0, secret_number
    li $v0, 1
    syscall
    li $v0, 4
    la $a0, msg_newline
    syscall

end_game:
    li $v0, 4
    la $a0, msg_goodbye
    syscall
    
    # Exit
    li $v0, 10
    syscall
    nop
