# ============================================
# 2048.asm - Terminal 2048 Game
# ============================================
# Controls: w=up, a=left, s=down, d=right, q=quit
# Combine tiles with same numbers to reach 2048!
# ============================================

.data
# Board (4x4 grid)
board: .space 64            # 16 words (4 bytes each)

# Game state
score: .word 0
game_over: .word 0
won: .word 0

# Display strings
msg_title: .asciiz "\n╔════════════════════════════════════╗\n"
msg_title2: .asciiz "║          2048 GAME                 ║\n"
msg_title3: .asciiz "╚════════════════════════════════════╝\n"
msg_controls: .asciiz "Controls: W/A/S/D to move, Q to quit\n"
msg_score: .asciiz "Score: "
msg_game_over: .asciiz "\n╔════════════════════════════════════╗\n"
msg_game_over2: .asciiz "║         GAME OVER!                 ║\n"
msg_game_over3: .asciiz "╚════════════════════════════════════╝\n"
msg_you_win: .asciiz "\n╔════════════════════════════════════╗\n"
msg_you_win2: .asciiz "║      YOU WIN! REACHED 2048!        ║\n"
msg_you_win3: .asciiz "╚════════════════════════════════════╝\n"
msg_continue: .asciiz "Press any key to continue or Q to quit...\n"

# Board drawing
line_top: .asciiz "╔══════╦══════╦══════╦══════╗\n"
line_mid: .asciiz "╠══════╬══════╬══════╬══════╣\n"
line_bot: .asciiz "╚══════╩══════╩══════╩══════╝\n"
cell_start: .asciiz "║"
cell_empty: .asciiz "      "
newline: .asciiz "\n"
space: .asciiz " "

.text
.globl main

main:
    # Initialize game
    jal init_board
    nop
    
    # Add two starting tiles
    jal add_random_tile
    nop
    jal add_random_tile
    nop

game_loop:
    # Clear screen
    li $v0, 11
    syscall
    
    # Draw board
    jal draw_board
    nop
    
    # Check if game over or won
    lw $t0, game_over
    bne $t0, $zero, handle_game_over
    nop
    
    lw $t0, won
    bne $t0, $zero, handle_win
    nop
    
    # Get input
    li $v0, 4
    la $a0, msg_controls
    syscall
    
get_input:
    li $v0, 12              # read char non-blocking
    syscall
    
    # If no input, try again
    beq $v0, $zero, get_input
    nop
    
    move $a0, $v0
    jal handle_input
    nop
    
    # If quit, exit
    lw $t0, game_over
    bne $t0, $zero, end_game
    nop
    
    # If board changed, add new tile
    beq $v0, $zero, game_loop
    nop
    
    jal add_random_tile
    nop
    
    # Check if game over
    jal check_game_over
    nop
    
    j game_loop
    nop

handle_win:
    li $v0, 4
    la $a0, msg_you_win
    syscall
    la $a0, msg_you_win2
    syscall
    la $a0, msg_you_win3
    syscall
    la $a0, msg_continue
    syscall
    
    # Wait for input
wait_after_win:
    li $v0, 12
    syscall
    beq $v0, $zero, wait_after_win
    nop
    
    # Check if quit
    li $t0, 113             # 'q'
    beq $v0, $t0, end_game
    nop
    li $t0, 81              # 'Q'
    beq $v0, $t0, end_game
    nop
    
    # Continue playing
    li $t0, 0
    sw $t0, won
    j game_loop
    nop

handle_game_over:
    li $v0, 4
    la $a0, msg_game_over
    syscall
    la $a0, msg_game_over2
    syscall
    la $a0, msg_game_over3
    syscall
    j end_game
    nop

end_game:
    li $v0, 10
    syscall
    nop

# ============================================
# init_board: Initialize empty board
# ============================================
init_board:
    la $t0, board
    li $t1, 0
    li $t2, 16
init_loop:
    bge $t1, $t2, init_done
    nop
    sw $zero, 0($t0)
    addiu $t0, $t0, 4
    addiu $t1, $t1, 1
    j init_loop
    nop
init_done:
    jr $ra
    nop

# ============================================
# draw_board: Draw the game board
# ============================================
draw_board:
    addiu $sp, $sp, -20
    sw $ra, 16($sp)
    sw $s0, 12($sp)
    sw $s1, 8($sp)
    sw $s2, 4($sp)
    sw $s3, 0($sp)
    
    # Print title
    li $v0, 4
    la $a0, msg_title
    syscall
    la $a0, msg_title2
    syscall
    la $a0, msg_title3
    syscall
    
    # Print score
    la $a0, msg_score
    syscall
    lw $a0, score
    li $v0, 1
    syscall
    li $v0, 4
    la $a0, newline
    syscall
    la $a0, newline
    syscall
    
    # Print top line
    li $v0, 4
    la $a0, line_top
    syscall
    
    # Print rows
    li $s0, 0               # row counter
draw_row_loop:
    li $t0, 4
    bge $s0, $t0, draw_done
    nop
    
    # Print row cells
    li $s1, 0               # col counter
draw_col_loop:
    li $t0, 4
    bge $s1, $t0, draw_col_done
    nop
    
    # Print cell start
    li $v0, 4
    la $a0, cell_start
    syscall
    
    # Get cell value
    move $a0, $s0
    move $a1, $s1
    jal get_cell
    nop
    move $s2, $v0
    
    # If zero, print empty
    bne $s2, $zero, draw_number
    nop
    li $v0, 4
    la $a0, cell_empty
    syscall
    j next_cell
    nop
    
draw_number:
    # Print number centered (max 4 digits for 2048)
    move $a0, $s2
    jal count_digits
    nop
    move $s3, $v0
    
    # Print leading spaces
    li $t0, 6
    sub $t0, $t0, $s3
    srl $t0, $t0, 1         # (6 - digits) / 2
print_lead_space:
    blez $t0, print_num
    nop
    li $v0, 4
    la $a0, space
    syscall
    addiu $t0, $t0, -1
    j print_lead_space
    nop
    
print_num:
    move $a0, $s2
    li $v0, 1
    syscall
    
    # Print trailing spaces
    li $t0, 6
    sub $t0, $t0, $s3
    sub $t0, $t0, $s3
    li $t1, 6
    sub $t0, $t1, $t0
    sub $t0, $t0, $s3
print_trail_space:
    blez $t0, next_cell
    nop
    li $v0, 4
    la $a0, space
    syscall
    addiu $t0, $t0, -1
    j print_trail_space
    nop
    
next_cell:
    addiu $s1, $s1, 1
    j draw_col_loop
    nop
    
draw_col_done:
    # Print cell end and newline
    li $v0, 4
    la $a0, cell_start
    syscall
    la $a0, newline
    syscall
    
    # Print separator or bottom line
    addiu $s0, $s0, 1
    li $t0, 4
    bge $s0, $t0, print_bottom
    nop
    li $v0, 4
    la $a0, line_mid
    syscall
    j draw_row_loop
    nop
    
print_bottom:
    li $v0, 4
    la $a0, line_bot
    syscall
    
draw_done:
    lw $s3, 0($sp)
    lw $s2, 4($sp)
    lw $s1, 8($sp)
    lw $s0, 12($sp)
    lw $ra, 16($sp)
    addiu $sp, $sp, 20
    jr $ra
    nop

# ============================================
# get_cell: Get value at board[row][col]
# Args: $a0 = row, $a1 = col
# Returns: $v0 = value
# ============================================
get_cell:
    sll $t0, $a0, 2         # row * 4
    add $t0, $t0, $a1       # + col
    sll $t0, $t0, 2         # * 4 (word size)
    la $t1, board
    add $t1, $t1, $t0
    lw $v0, 0($t1)
    jr $ra
    nop

# ============================================
# set_cell: Set value at board[row][col]
# Args: $a0 = row, $a1 = col, $a2 = value
# ============================================
set_cell:
    sll $t0, $a0, 2
    add $t0, $t0, $a1
    sll $t0, $t0, 2
    la $t1, board
    add $t1, $t1, $t0
    sw $a2, 0($t1)
    jr $ra
    nop

# ============================================
# count_digits: Count digits in number
# Args: $a0 = number
# Returns: $v0 = digit count
# ============================================
count_digits:
    li $v0, 0
    beq $a0, $zero, count_zero
    nop
count_loop:
    beq $a0, $zero, count_done
    nop
    addiu $v0, $v0, 1
    li $t0, 10
    divu $a0, $t0
    mflo $a0
    j count_loop
    nop
count_zero:
    li $v0, 1
count_done:
    jr $ra
    nop

# ============================================
# add_random_tile: Add 2 or 4 to random empty cell
# ============================================
add_random_tile:
    addiu $sp, $sp, -8
    sw $ra, 4($sp)
    sw $s0, 0($sp)
    
    # Find empty cells
    li $t0, 0               # empty count
    li $t1, 0               # cell index
    la $t2, board
find_empty_loop:
    li $t3, 16
    bge $t1, $t3, find_empty_done
    nop
    lw $t4, 0($t2)
    bne $t4, $zero, not_empty
    nop
    addiu $t0, $t0, 1
not_empty:
    addiu $t2, $t2, 4
    addiu $t1, $t1, 1
    j find_empty_loop
    nop
find_empty_done:
    
    # If no empty cells, return
    beq $t0, $zero, add_tile_done
    nop
    
    # Choose random empty cell
    li $v0, 14              # get time
    syscall
    divu $v0, $t0
    mfhi $s0                # random index among empty cells
    
    # Find the nth empty cell
    li $t1, 0               # cell index
    li $t2, 0               # empty counter
    la $t3, board
find_nth_empty:
    li $t4, 16
    bge $t1, $t4, add_tile_done
    nop
    lw $t5, 0($t3)
    bne $t5, $zero, skip_cell
    nop
    beq $t2, $s0, found_cell
    nop
    addiu $t2, $t2, 1
skip_cell:
    addiu $t3, $t3, 4
    addiu $t1, $t1, 1
    j find_nth_empty
    nop
    
found_cell:
    # 90% chance of 2, 10% chance of 4
    li $v0, 14
    syscall
    li $t0, 10
    divu $v0, $t0
    mfhi $t0
    li $t1, 2
    bne $t0, $zero, store_value
    nop
    li $t1, 4
store_value:
    sw $t1, 0($t3)
    
add_tile_done:
    lw $s0, 0($sp)
    lw $ra, 4($sp)
    addiu $sp, $sp, 8
    jr $ra
    nop

# ============================================
# handle_input: Process move
# Args: $a0 = character
# Returns: $v0 = 1 if board changed, 0 otherwise
# ============================================
handle_input:
    addiu $sp, $sp, -4
    sw $ra, 0($sp)
    
    li $v0, 0               # default: no change
    
    # Check for quit
    li $t0, 113             # 'q'
    beq $a0, $t0, input_quit
    nop
    li $t0, 81              # 'Q'
    beq $a0, $t0, input_quit
    nop
    
    # Check for up
    li $t0, 119             # 'w'
    beq $a0, $t0, move_up
    nop
    li $t0, 87              # 'W'
    beq $a0, $t0, move_up
    nop
    
    # Check for left
    li $t0, 97              # 'a'
    beq $a0, $t0, move_left
    nop
    li $t0, 65              # 'A'
    beq $a0, $t0, move_left
    nop
    
    # Check for down
    li $t0, 115             # 's'
    beq $a0, $t0, move_down
    nop
    li $t0, 83              # 'S'
    beq $a0, $t0, move_down
    nop
    
    # Check for right
    li $t0, 100             # 'd'
    beq $a0, $t0, move_right
    nop
    li $t0, 68              # 'D'
    beq $a0, $t0, move_right
    nop
    
    j input_done
    nop

input_quit:
    li $t0, 1
    sw $t0, game_over
    j input_done
    nop

move_up:
    jal move_board_up
    nop
    j input_done
    nop

move_down:
    jal move_board_down
    nop
    j input_done
    nop

move_left:
    jal move_board_left
    nop
    j input_done
    nop

move_right:
    jal move_board_right
    nop
    j input_done
    nop

input_done:
    lw $ra, 0($sp)
    addiu $sp, $sp, 4
    jr $ra
    nop

# ============================================
# move_board_left: Move all tiles left
# Returns: $v0 = 1 if changed
# ============================================
move_board_left:
    addiu $sp, $sp, -24
    sw $ra, 20($sp)
    sw $s0, 16($sp)
    sw $s1, 12($sp)
    sw $s2, 8($sp)
    sw $s3, 4($sp)
    sw $s4, 0($sp)
    
    li $s4, 0               # changed flag
    
    li $s0, 0               # row
move_left_row:
    li $t0, 4
    bge $s0, $t0, move_left_done
    nop
    
    # Process row
    li $s1, 0               # target col
    li $s2, 0               # source col
move_left_col:
    li $t0, 4
    bge $s2, $t0, move_left_row_done
    nop
    
    # Get source cell
    move $a0, $s0
    move $a1, $s2
    jal get_cell
    nop
    move $s3, $v0
    
    # Skip if empty
    beq $s3, $zero, move_left_next
    nop
    
    # Check if can merge with target
    bne $s1, $s2, check_merge
    nop
    addiu $s2, $s2, 1
    addiu $s1, $s1, 1
    j move_left_col
    nop
    
check_merge:
    move $a0, $s0
    move $a1, $s1
    jal get_cell
    nop
    
    beq $v0, $zero, place_tile
    nop
    bne $v0, $s3, no_merge
    nop
    
    # Merge
    sll $t0, $s3, 1         # value * 2
    move $a0, $s0
    move $a1, $s1
    move $a2, $t0
    jal set_cell
    nop
    
    # Clear source
    move $a0, $s0
    move $a1, $s2
    move $a2, $zero
    jal set_cell
    nop
    
    # Update score
    lw $t1, score
    add $t1, $t1, $t0
    sw $t1, score
    
    # Check for 2048
    li $t2, 2048
    bne $t0, $t2, merge_done
    nop
    li $t2, 1
    sw $t2, won
    
merge_done:
    li $s4, 1
    addiu $s1, $s1, 1
    j move_left_next
    nop
    
no_merge:
    addiu $s1, $s1, 1
    
place_tile:
    # Place tile at target
    move $a0, $s0
    move $a1, $s1
    move $a2, $s3
    jal set_cell
    nop
    
    # Clear source if different
    bne $s1, $s2, clear_source
    nop
    j skip_clear
    nop
    
clear_source:
    move $a0, $s0
    move $a1, $s2
    move $a2, $zero
    jal set_cell
    nop
    li $s4, 1
    
skip_clear:
    addiu $s1, $s1, 1
    
move_left_next:
    addiu $s2, $s2, 1
    j move_left_col
    nop
    
move_left_row_done:
    addiu $s0, $s0, 1
    j move_left_row
    nop
    
move_left_done:
    move $v0, $s4
    lw $s4, 0($sp)
    lw $s3, 4($sp)
    lw $s2, 8($sp)
    lw $s1, 12($sp)
    lw $s0, 16($sp)
    lw $ra, 20($sp)
    addiu $sp, $sp, 24
    jr $ra
    nop

# ============================================
# move_board_right: Move all tiles right
# Returns: $v0 = 1 if changed
# ============================================
move_board_right:
    addiu $sp, $sp, -24
    sw $ra, 20($sp)
    sw $s0, 16($sp)
    sw $s1, 12($sp)
    sw $s2, 8($sp)
    sw $s3, 4($sp)
    sw $s4, 0($sp)
    
    li $s4, 0
    li $s0, 0
move_right_row:
    li $t0, 4
    bge $s0, $t0, move_right_done
    nop
    
    li $s1, 3               # target col (rightmost)
    li $s2, 3               # source col
move_right_col:
    bltz $s2, move_right_row_done
    nop
    
    move $a0, $s0
    move $a1, $s2
    jal get_cell
    nop
    move $s3, $v0
    
    beq $s3, $zero, move_right_next
    nop
    
    bne $s1, $s2, check_merge_right
    nop
    addiu $s2, $s2, -1
    addiu $s1, $s1, -1
    j move_right_col
    nop
    
check_merge_right:
    move $a0, $s0
    move $a1, $s1
    jal get_cell
    nop
    
    beq $v0, $zero, place_tile_right
    nop
    bne $v0, $s3, no_merge_right
    nop
    
    sll $t0, $s3, 1
    move $a0, $s0
    move $a1, $s1
    move $a2, $t0
    jal set_cell
    nop
    
    move $a0, $s0
    move $a1, $s2
    move $a2, $zero
    jal set_cell
    nop
    
    lw $t1, score
    add $t1, $t1, $t0
    sw $t1, score
    
    li $t2, 2048
    bne $t0, $t2, merge_done_right
    nop
    li $t2, 1
    sw $t2, won
    
merge_done_right:
    li $s4, 1
    addiu $s1, $s1, -1
    j move_right_next
    nop
    
no_merge_right:
    addiu $s1, $s1, -1
    
place_tile_right:
    move $a0, $s0
    move $a1, $s1
    move $a2, $s3
    jal set_cell
    nop
    
    bne $s1, $s2, clear_source_right
    nop
    j skip_clear_right
    nop
    
clear_source_right:
    move $a0, $s0
    move $a1, $s2
    move $a2, $zero
    jal set_cell
    nop
    li $s4, 1
    
skip_clear_right:
    addiu $s1, $s1, -1
    
move_right_next:
    addiu $s2, $s2, -1
    j move_right_col
    nop
    
move_right_row_done:
    addiu $s0, $s0, 1
    j move_right_row
    nop
    
move_right_done:
    move $v0, $s4
    lw $s4, 0($sp)
    lw $s3, 4($sp)
    lw $s2, 8($sp)
    lw $s1, 12($sp)
    lw $s0, 16($sp)
    lw $ra, 20($sp)
    addiu $sp, $sp, 24
    jr $ra
    nop

# ============================================
# move_board_up: Move all tiles up
# Returns: $v0 = 1 if changed
# ============================================
move_board_up:
    addiu $sp, $sp, -24
    sw $ra, 20($sp)
    sw $s0, 16($sp)
    sw $s1, 12($sp)
    sw $s2, 8($sp)
    sw $s3, 4($sp)
    sw $s4, 0($sp)
    
    li $s4, 0
    li $s0, 0               # col
move_up_col:
    li $t0, 4
    bge $s0, $t0, move_up_done
    nop
    
    li $s1, 0               # target row
    li $s2, 0               # source row
move_up_row:
    li $t0, 4
    bge $s2, $t0, move_up_col_done
    nop
    
    move $a0, $s2
    move $a1, $s0
    jal get_cell
    nop
    move $s3, $v0
    
    beq $s3, $zero, move_up_next
    nop
    
    bne $s1, $s2, check_merge_up
    nop
    addiu $s2, $s2, 1
    addiu $s1, $s1, 1
    j move_up_row
    nop
    
check_merge_up:
    move $a0, $s1
    move $a1, $s0
    jal get_cell
    nop
    
    beq $v0, $zero, place_tile_up
    nop
    bne $v0, $s3, no_merge_up
    nop
    
    sll $t0, $s3, 1
    move $a0, $s1
    move $a1, $s0
    move $a2, $t0
    jal set_cell
    nop
    
    move $a0, $s2
    move $a1, $s0
    move $a2, $zero
    jal set_cell
    nop
    
    lw $t1, score
    add $t1, $t1, $t0
    sw $t1, score
    
    li $t2, 2048
    bne $t0, $t2, merge_done_up
    nop
    li $t2, 1
    sw $t2, won
    
merge_done_up:
    li $s4, 1
    addiu $s1, $s1, 1
    j move_up_next
    nop
    
no_merge_up:
    addiu $s1, $s1, 1
    
place_tile_up:
    move $a0, $s1
    move $a1, $s0
    move $a2, $s3
    jal set_cell
    nop
    
    bne $s1, $s2, clear_source_up
    nop
    j skip_clear_up
    nop
    
clear_source_up:
    move $a0, $s2
    move $a1, $s0
    move $a2, $zero
    jal set_cell
    nop
    li $s4, 1
    
skip_clear_up:
    addiu $s1, $s1, 1
    
move_up_next:
    addiu $s2, $s2, 1
    j move_up_row
    nop
    
move_up_col_done:
    addiu $s0, $s0, 1
    j move_up_col
    nop
    
move_up_done:
    move $v0, $s4
    lw $s4, 0($sp)
    lw $s3, 4($sp)
    lw $s2, 8($sp)
    lw $s1, 12($sp)
    lw $s0, 16($sp)
    lw $ra, 20($sp)
    addiu $sp, $sp, 24
    jr $ra
    nop

# ============================================
# move_board_down: Move all tiles down
# Returns: $v0 = 1 if changed
# ============================================
move_board_down:
    addiu $sp, $sp, -24
    sw $ra, 20($sp)
    sw $s0, 16($sp)
    sw $s1, 12($sp)
    sw $s2, 8($sp)
    sw $s3, 4($sp)
    sw $s4, 0($sp)
    
    li $s4, 0
    li $s0, 0
move_down_col:
    li $t0, 4
    bge $s0, $t0, move_down_done
    nop
    
    li $s1, 3               # target row
    li $s2, 3               # source row
move_down_row:
    bltz $s2, move_down_col_done
    nop
    
    move $a0, $s2
    move $a1, $s0
    jal get_cell
    nop
    move $s3, $v0
    
    beq $s3, $zero, move_down_next
    nop
    
    bne $s1, $s2, check_merge_down
    nop
    addiu $s2, $s2, -1
    addiu $s1, $s1, -1
    j move_down_row
    nop
    
check_merge_down:
    move $a0, $s1
    move $a1, $s0
    jal get_cell
    nop
    
    beq $v0, $zero, place_tile_down
    nop
    bne $v0, $s3, no_merge_down
    nop
    
    sll $t0, $s3, 1
    move $a0, $s1
    move $a1, $s0
    move $a2, $t0
    jal set_cell
    nop
    
    move $a0, $s2
    move $a1, $s0
    move $a2, $zero
    jal set_cell
    nop
    
    lw $t1, score
    add $t1, $t1, $t0
    sw $t1, score
    
    li $t2, 2048
    bne $t0, $t2, merge_done_down
    nop
    li $t2, 1
    sw $t2, won
    
merge_done_down:
    li $s4, 1
    addiu $s1, $s1, -1
    j move_down_next
    nop
    
no_merge_down:
    addiu $s1, $s1, -1
    
place_tile_down:
    move $a0, $s1
    move $a1, $s0
    move $a2, $s3
    jal set_cell
    nop
    
    bne $s1, $s2, clear_source_down
    nop
    j skip_clear_down
    nop
    
clear_source_down:
    move $a0, $s2
    move $a1, $s0
    move $a2, $zero
    jal set_cell
    nop
    li $s4, 1
    
skip_clear_down:
    addiu $s1, $s1, -1
    
move_down_next:

# ============================================
# check_game_over: Check if no moves left
# ============================================
check_game_over:
    addiu $sp, $sp, -8
    sw $ra, 4($sp)
    sw $s0, 0($sp)
    
    # Check for empty cells
    li $t0, 0
    la $t1, board
check_empty_loop:
    li $t2, 16
    bge $t0, $t2, check_merges
    nop
    lw $t3, 0($t1)
    beq $t3, $zero, game_not_over
    nop
    addiu $t1, $t1, 4
    addiu $t0, $t0, 1
    j check_empty_loop
    nop
    
check_merges:
    # Check for possible merges (adjacent same values)
    li $s0, 0
check_merge_loop:
    li $t0, 16
    bge $s0, $t0, set_game_over
    nop
    
    # Get current cell value
    srl $t1, $s0, 2         # row = i / 4
    andi $t2, $s0, 3        # col = i % 4
    move $a0, $t1
    move $a1, $t2
    jal get_cell
    nop
    move $t3, $v0
    
    # Check right neighbor
    li $t4, 3
    bge $t2, $t4, check_down
    nop
    addiu $a1, $t2, 1
    move $a0, $t1
    jal get_cell
    nop
    beq $v0, $t3, game_not_over
    nop
    
check_down:
    # Check down neighbor
    li $t4, 3
    bge $t1, $t4, next_merge_check
    nop
    addiu $a0, $t1, 1
    move $a1, $t2
    jal get_cell
    nop
    beq $v0, $t3, game_not_over
    nop
    
next_merge_check:
    addiu $s0, $s0, 1
    j check_merge_loop
    nop
    
set_game_over:
    li $t0, 1
    sw $t0, game_over
    
game_not_over:
    lw $s0, 0($sp)
    lw $ra, 4($sp)
    addiu $sp, $sp, 8
    jr $ra
    nop
