.data
start: .asciiz "STARTING PROGRAM\n"
invalidt: .asciiz "\nInvalid Number!\n"
exit: .asciiz "\nEXIT...\n"
ts1: .asciiz "\nIN THREAD 1\n"
ts2: .asciiz "\nIN THREAD 2\n"
ts3: .asciiz "\nIN THREAD 3\n"
compt: .asciiz "\nOperation Completed\n"
mank: .asciiz "\nMIN\n"
t1in: .asciiz "inside of t1\n"

.text
.globl main
main:
menu:

la $a0, start
li $v0, 4
syscall

la $a0, t1
la $a1, ($sp)
li $v0, 18
syscall
t1:

la $a0, t2
addi $sp,  $sp, -1024
la $a1, ($sp)
addi $sp,  $sp, 1024
li $v0, 18
syscall


la $a0, t3
addi $sp,  $sp, -2048
la $a1, ($sp)
addi $sp,  $sp, 2048
li $v0, 18
syscall

  and $t1,$t1,$zero
  bac: 
  beq $t1,10000,exitt
    la $a0, mank
    li $v0, 4
   # syscall
   addi $t1,$t1,1
    j bac

            la $a0, start
        li $v0, 4
    syscall
j exitt

exitt:
la $a0, exit
li $v0, 4
syscall
li $v0, 10
syscall


    
t2:
    
    la $a0, ts1
    li $v0, 4
    syscall
    
    ba: 
    beq $t2,15,exit2
    la $a0, t1in
    li $v0, 4
    syscall
    addi $t2,$t2,1
    j ba
    
exit2:

    jr $ra
    
t3:
    la $a0, ts2
    li $v0, 4
    syscall
    cb: j bd
    bd:
    j cb
    
    jr $ra

