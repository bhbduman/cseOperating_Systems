.data
menut: .asciiz "\n1-Show Primes\n2-Factorize\n3-BubbleSort\n0-Exit\n"
invalidt: .asciiz "\nInvalid Number!\n"
exit: .asciiz "\nEXIT...\n"
compt: .asciiz "\nOperation Completed\n"
.text
.globl main
main:
menu:
la $a0, menut
li $v0, 4
syscall

cont:
li $v0, 5
syscall 
move $t0, $v0

blt $t0, 0, invalid
bgt $t0, 3, invalid
beq $t0, 0, exitt
j run

invalid:
la $a0, invalidt
li $v0, 4
syscall
j menu

run:
move $a0, $t0
li $v0, 18
syscall
la $a0, compt
li $v0, 4
syscall
j menu

exitt:
la $a0, exit
li $v0, 4
syscall
li $v0, 10
syscall


