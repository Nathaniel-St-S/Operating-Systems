# ============================================
# snake.asm - Terminal Snake Game
# ============================================
# Controls: w=up, a=left, s=down, d=right, q=quit
# The snake grows when eating food (@)
# Game ends on collision with walls or self
# ============================================

.data
# Game constants
WIDTH: .word 40
HEIGHT: .word 20
INITIAL_LENGTH: .word 3

# Snake data (max 800 segments for 40x20 grid)
snake_x: .space 800        # X coordinates
snake_y: .space 800        # Y coordinates
snake_length: .word 3
head_idx: .word 0
tail_idx: .word 0

# Direction (dx, dy)
dir_x: .word 1
dir_y: .word 0

# Food position
food_x: .word 10
food_y: .word 10

# Game state
game_over: .word 0
score: .word 0

# Display characters
char_wall: .asciiz "#"
char_snake: .asciiz "O"
char_food: .asciiz "@"
char_space: .asciiz " "
char_newline: .asciiz "\n"

# Messages
msg_title: .asciiz "=== SNAKE GAME ===\n"
msg_controls: .asciiz "Controls: W=Up A=Left S=Down D=Right Q=Quit\n"
msg_score: .asciiz "Score: "
msg_game_over: .asciiz "\n\n*** GAME OVER ***\n"
msg_final_score: .asciiz "Final Score: "

.text
.globl main

main:
    # Initialize game
    jal init_game
    nop
    
    # Print title
    li $v0, 4
    la $a0, msg_title
    syscall
    
    li $v0, 4
    la $a0, msg_controls
    syscall
    
    li $v0, 4
    la $a0, char_newline
    syscall

game_loop:
    # Check if game over
    lw $t0, game_over
    bne $t0, $zero, end_game
    nop
    
    # Clear screen
    li $v0, 11              # syscall 11: clear screen
    syscall
    
    # Draw game
    jal draw_game
    nop
    
    # Get input (non-blocking)
    li $v0, 12              # syscall 12: read char non-blocking
    syscall
    move $a0, $v0           # input char in $a0
    jal handle_input
    nop
    
    # Update game state
    jal update_game
    nop
    
    # Delay
    li $v0, 13              # syscall 13: sleep milliseconds
    li $a0, 150             # 150ms delay
    syscall
    
    j game_loop
    nop

end_game:
    # Print game over message
    li $v0, 4
    la $a0, msg_game_over
    syscall
    
    li $v0, 4
    la $a0, msg_final_score
    syscall
    
    lw $a0, score
    li $v0, 1
    syscall
    
    li $v0, 4
    la $a0, char_newline
    syscall
    
    # Exit
    li $v0, 10
    syscall
    nop

# ============================================
# init_game: Initialize game state
# ============================================
init_game:
    addiu $sp, $sp, -4
    sw $ra, 0($sp)
    
    # Initialize snake in middle of screen
    lw $t0, WIDTH
    srl $t0, $t0, 1         # width / 2
    lw $t1, HEIGHT
    srl $t1, $t1, 1         # height / 2
    
    # Head at (width/2, height/2)
    la $t2, snake_x
    sw $t0, 0($t2)
    la $t2, snake_y
    sw $t1, 0($t2)
    
    # Second segment
    addiu $t0, $t0, -1
    la $t2, snake_x
    sw $t0, 4($t2)
    la $t2, snake_y
    sw $t1, 4($t2)
    
    # Third segment
    addiu $t0, $t0, -1
    la $t2, snake_x
    sw $t0, 8($t2)
    la $t2, snake_y
    sw $t1, 8($t2)
    
    # Spawn food
    jal spawn_food
    nop
    
    lw $ra, 0($sp)
    addiu $sp, $sp, 4
    jr $ra
    nop

# ============================================
# draw_game: Draw the game board
# ============================================
draw_game:
    addiu $sp, $sp, -20
    sw $ra, 16($sp)
    sw $s0, 12($sp)
    sw $s1, 8($sp)
    sw $s2, 4($sp)
    sw $s3, 0($sp)
    
    # Print score
    li $v0, 4
    la $a0, msg_score
    syscall
    
    lw $a0, score
    li $v0, 1
    syscall
    
    li $v0, 4
    la $a0, char_newline
    syscall
    
    # Draw top wall
    lw $s0, WIDTH
    addiu $s0, $s0, 2       # +2 for side walls
    li $s1, 0
draw_top_wall:
    bge $s1, $s0, draw_top_done
    nop
    li $v0, 4
    la $a0, char_wall
    syscall
    addiu $s1, $s1, 1
    j draw_top_wall
    nop
draw_top_done:
    li $v0, 4
    la $a0, char_newline
    syscall
    
    # Draw game area
    lw $s0, HEIGHT
    li $s1, 0               # y counter
draw_row_loop:
    bge $s1, $s0, draw_rows_done
    nop
    
    # Left wall
    li $v0, 4
    la $a0, char_wall
    syscall
    
    # Draw row contents
    lw $s2, WIDTH
    li $s3, 0               # x counter
draw_col_loop:
    bge $s3, $s2, draw_col_done
    nop
    
    # Check if this is food position
    lw $t0, food_x
    lw $t1, food_y
    bne $s3, $t0, not_food
    nop
    bne $s1, $t1, not_food
    nop
    # Draw food
    li $v0, 4
    la $a0, char_food
    syscall
    j next_col
    nop
    
not_food:
    # Check if this is snake position
    move $a0, $s3
    move $a1, $s1
    jal is_snake_segment
    nop
    beq $v0, $zero, not_snake
    nop
    # Draw snake
    li $v0, 4
    la $a0, char_snake
    syscall
    j next_col
    nop
    
not_snake:
    # Draw space
    li $v0, 4
    la $a0, char_space
    syscall
    
next_col:
    addiu $s3, $s3, 1
    j draw_col_loop
    nop
    
draw_col_done:
    # Right wall
    li $v0, 4
    la $a0, char_wall
    syscall
    
    # Newline
    li $v0, 4
    la $a0, char_newline
    syscall
    
    addiu $s1, $s1, 1
    j draw_row_loop
    nop
    
draw_rows_done:
    # Draw bottom wall
    lw $s0, WIDTH
    addiu $s0, $s0, 2
    li $s1, 0
draw_bottom_wall:
    bge $s1, $s0, draw_bottom_done
    nop
    li $v0, 4
    la $a0, char_wall
    syscall
    addiu $s1, $s1, 1
    j draw_bottom_wall
    nop
draw_bottom_done:
    li $v0, 4
    la $a0, char_newline
    syscall
    
    lw $s3, 0($sp)
    lw $s2, 4($sp)
    lw $s1, 8($sp)
    lw $s0, 12($sp)
    lw $ra, 16($sp)
    addiu $sp, $sp, 20
    jr $ra
    nop

# ============================================
# is_snake_segment: Check if (x,y) is snake
# Args: $a0 = x, $a1 = y
# Returns: $v0 = 1 if snake, 0 otherwise
# ============================================
is_snake_segment:
    lw $t0, snake_length
    li $t1, 0
    la $t2, snake_x
    la $t3, snake_y
    
check_segment_loop:
    bge $t1, $t0, not_segment
    nop
    
    sll $t4, $t1, 2         # offset = i * 4
    add $t5, $t2, $t4
    lw $t5, 0($t5)          # snake_x[i]
    bne $t5, $a0, next_segment
    nop
    
    add $t6, $t3, $t4
    lw $t6, 0($t6)          # snake_y[i]
    bne $t6, $a1, next_segment
    nop
    
    # Found match
    li $v0, 1
    jr $ra
    nop
    
next_segment:
    addiu $t1, $t1, 1
    j check_segment_loop
    nop
    
not_segment:
    li $v0, 0
    jr $ra
    nop

# ============================================
# handle_input: Process player input
# Args: $a0 = character
# ============================================
handle_input:
    # Check for 'q' or 'Q' (quit)
    li $t0, 113             # 'q'
    beq $a0, $t0, input_quit
    nop
    li $t0, 81              # 'Q'
    beq $a0, $t0, input_quit
    nop
    
    # Check for 'w' or 'W' (up)
    li $t0, 119             # 'w'
    beq $a0, $t0, input_up
    nop
    li $t0, 87              # 'W'
    beq $a0, $t0, input_up
    nop
    
    # Check for 'a' or 'A' (left)
    li $t0, 97              # 'a'
    beq $a0, $t0, input_left
    nop
    li $t0, 65              # 'A'
    beq $a0, $t0, input_left
    nop
    
    # Check for 's' or 'S' (down)
    li $t0, 115             # 's'
    beq $a0, $t0, input_down
    nop
    li $t0, 83              # 'S'
    beq $a0, $t0, input_down
    nop
    
    # Check for 'd' or 'D' (right)
    li $t0, 100             # 'd'
    beq $a0, $t0, input_right
    nop
    li $t0, 68              # 'D'
    beq $a0, $t0, input_right
    nop
    
    jr $ra
    nop

input_quit:
    li $t0, 1
    sw $t0, game_over
    jr $ra
    nop

input_up:
    lw $t0, dir_y
    li $t1, 1
    beq $t0, $t1, input_done  # Can't go up if going down
    nop
    li $t0, 0
    sw $t0, dir_x
    li $t0, -1
    sw $t0, dir_y
    jr $ra
    nop

input_down:
    lw $t0, dir_y
    li $t1, -1
    beq $t0, $t1, input_done  # Can't go down if going up
    nop
    li $t0, 0
    sw $t0, dir_x
    li $t0, 1
    sw $t0, dir_y
    jr $ra
    nop

input_left:
    lw $t0, dir_x
    li $t1, 1
    beq $t0, $t1, input_done  # Can't go left if going right
    nop
    li $t0, -1
    sw $t0, dir_x
    li $t0, 0
    sw $t0, dir_y
    jr $ra
    nop

input_right:
    lw $t0, dir_x
    li $t1, -1
    beq $t0, $t1, input_done  # Can't go right if going left
    nop
    li $t0, 1
    sw $t0, dir_x
    li $t0, 0
    sw $t0, dir_y
    jr $ra
    nop

input_done:
    jr $ra
    nop

# ============================================
# update_game: Update game state
# ============================================
update_game:
    addiu $sp, $sp, -4
    sw $ra, 0($sp)
    
    # Calculate new head position
    la $t0, snake_x
    lw $t1, 0($t0)          # current head x
    lw $t2, dir_x
    add $t1, $t1, $t2       # new head x
    
    la $t0, snake_y
    lw $t3, 0($t0)          # current head y
    lw $t4, dir_y
    add $t3, $t3, $t4       # new head y
    
    # Check wall collision
    bltz $t1, collision
    nop
    lw $t5, WIDTH
    bge $t1, $t5, collision
    nop
    bltz $t3, collision
    nop
    lw $t5, HEIGHT
    bge $t3, $t5, collision
    nop
    
    # Check self collision
    move $a0, $t1
    move $a1, $t3
    jal is_snake_segment
    nop
    bne $v0, $zero, collision
    nop
    
    # Check food collision
    lw $t5, food_x
    bne $t1, $t5, no_food
    nop
    lw $t5, food_y
    bne $t3, $t5, no_food
    nop
    
    # Ate food - grow snake
    lw $t5, snake_length
    addiu $t5, $t5, 1
    sw $t5, snake_length
    
    lw $t5, score
    addiu $t5, $t5, 10
    sw $t5, score
    
    jal spawn_food
    nop
    j move_snake
    nop
    
no_food:
    # Remove tail
    lw $t5, snake_length
    addiu $t5, $t5, -1
    sll $t5, $t5, 2
    la $t6, snake_x
    add $t6, $t6, $t5
    la $t7, snake_y
    add $t7, $t7, $t5
    
move_snake:
    # Shift all segments
    lw $t5, snake_length
    addiu $t5, $t5, -1
shift_loop:
    blez $t5, shift_done
    nop
    
    sll $t6, $t5, 2
    la $t7, snake_x
    add $t7, $t7, $t6
    lw $t8, -4($t7)
    sw $t8, 0($t7)
    
    la $t7, snake_y
    add $t7, $t7, $t6
    lw $t8, -4($t7)
    sw $t8, 0($t7)
    
    addiu $t5, $t5, -1
    j shift_loop
    nop
    
shift_done:
    # Add new head
    la $t0, snake_x
    sw $t1, 0($t0)
    la $t0, snake_y
    sw $t3, 0($t0)
    
    lw $ra, 0($sp)
    addiu $sp, $sp, 4
    jr $ra
    nop

collision:
    li $t0, 1
    sw $t0, game_over
    lw $ra, 0($sp)
    addiu $sp, $sp, 4
    jr $ra
    nop

# ============================================
# spawn_food: Spawn food at random location
# ============================================
spawn_food:
    addiu $sp, $sp, -4
    sw $ra, 0($sp)
    
    # Simple pseudo-random: use current time
    li $v0, 14              # syscall 14: get time
    syscall
    move $t0, $v0
    
    # X = time % WIDTH
    lw $t1, WIDTH
    divu $t0, $t1
    mfhi $t2
    sw $t2, food_x
    
    # Y = (time / WIDTH) % HEIGHT
    lw $t1, HEIGHT
    divu $t0, $t1
    mflo $t0
    divu $t0, $t1
    mfhi $t2
    sw $t2, food_y
    
    lw $ra, 0($sp)
    addiu $sp, $sp, 4
    jr $ra
    nop
