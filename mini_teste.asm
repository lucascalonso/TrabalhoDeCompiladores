# Codigo MIPS gerado pelo compilador C-
.data
    _newline: .asciiz "\n"
    .align 2
    x: .word 0

.text
.globl main
    j main

# Funcao: f
f:
    subu $sp, $sp, 8
    sw $fp, 4($sp)
    sw $ra, 0($sp)
    move $fp, $sp
    subu $sp, $sp, 8
    lw $t2, 8($fp)
    subu $sp, $sp, 4
    sw $t2, 0($sp)
    li $t0, 0
    lw $t2, 0($sp)
    addu $sp, $sp, 4
    sll $t0, $t0, 2
    addu $t2, $t2, $t0
    lw $t0, 0($t2)
    subu $sp, $sp, 4
    sw $t0, 0($sp)
    addiu $t2, $fp, -8
    lw $t0, 0($sp)
    addu $sp, $sp, 4
    sw $t0, 0($t2)
    la $t2, x
    lw $t0, 0($t2)
    subu $sp, $sp, 4
    sw $t0, 0($sp)
    addiu $t2, $fp, 12
    lw $t0, 0($t2)
    lw $t1, 0($sp)
    addu $sp, $sp, 4
    add $t0, $t1, $t0
    move $v0, $t0
    j f_fim
f_fim:
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
    subu $sp, $sp, 8
    li $t0, 2
    subu $sp, $sp, 4
    sw $t0, 0($sp)
    addiu $t2, $fp, -8
    move $t0, $t2
    subu $sp, $sp, 4
    sw $t0, 0($sp)
    jal f
    addu $sp, $sp, 8
    move $t0, $v0
    subu $sp, $sp, 4
    sw $t0, 0($sp)
    la $t2, x
    lw $t0, 0($sp)
    addu $sp, $sp, 4
    sw $t0, 0($t2)
main_fim:
    li $v0, 10
    syscall

