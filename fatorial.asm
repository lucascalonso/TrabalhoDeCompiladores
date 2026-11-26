# Codigo MIPS gerado pelo compilador C-
.data
    _newline: .asciiz "\n"
    .align 2

.text
.globl main
    j main

# Funcao: fatorial
fatorial:
    subu $sp, $sp, 8
    sw $fp, 4($sp)
    sw $ra, 0($sp)
    move $fp, $sp
    addiu $t2, $fp, 8
    lw $t0, 0($t2)
    subu $sp, $sp, 4
    sw $t0, 0($sp)
    li $t0, 1
    lw $t1, 0($sp)
    addu $sp, $sp, 4
    sle $t0, $t1, $t0
    beq $t0, $zero, L_else_0
    li $t0, 1
    move $v0, $t0
    j fatorial_fim
    j L_endif_1
L_else_0:
    addiu $t2, $fp, 8
    lw $t0, 0($t2)
    subu $sp, $sp, 4
    sw $t0, 0($sp)
    addiu $t2, $fp, 8
    lw $t0, 0($t2)
    subu $sp, $sp, 4
    sw $t0, 0($sp)
    li $t0, 1
    lw $t1, 0($sp)
    addu $sp, $sp, 4
    sub $t0, $t1, $t0
    subu $sp, $sp, 4
    sw $t0, 0($sp)
    jal fatorial
    addu $sp, $sp, 4
    move $t0, $v0
    lw $t1, 0($sp)
    addu $sp, $sp, 4
    mul $t0, $t1, $t0
    move $v0, $t0
    j fatorial_fim
L_endif_1:
fatorial_fim:
    move $sp, $fp
    lw $ra, 0($sp)
    lw $fp, 4($sp)
    addu $sp, $sp, 8
    jr $ra

# Funcao: main
main:
    subu $sp, $sp, 8
    sw $fp, 4($sp)
    sw $ra, 0($sp)
    move $fp, $sp
    subu $sp, $sp, 4
    li $v0, 5
    syscall
    move $t0, $v0
    subu $sp, $sp, 4
    sw $t0, 0($sp)
    addiu $t2, $fp, -4
    lw $t0, 0($sp)
    addu $sp, $sp, 4
    sw $t0, 0($t2)
    addiu $t2, $fp, -4
    lw $t0, 0($t2)
    subu $sp, $sp, 4
    sw $t0, 0($sp)
    jal fatorial
    addu $sp, $sp, 4
    move $t0, $v0
    move $a0, $t0
    li $v0, 1
    syscall
    la $a0, _newline
    li $v0, 4
    syscall
main_fim:
    li $v0, 10
    syscall

