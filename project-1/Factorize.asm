.data

num:   .word 0


invalidnum: 	.asciiz "Invalid Number"
newline: 	.asciiz "\n"
hyphen: 	.asciiz "-"
nume:   .asciiz "Enter Number"


.text 
#.globl main
main:
    li $v0, 4
	la   $a0, nume
	syscall
	li $v0, 5
	syscall
	sw  $v0,num

	lw $s1, num
	bgt $s1, $zero, continue
	
	li $v0, 4
	la   $a0, invalidnum
	syscall
	j halt
	
continue:
	li $v0, 1
	lw $a0,num
	syscall# for num itself
	
	lw $s1, num
	
	
	lw  $t0, num
out_for:
	addi $s1,$s1,-1
	beq  $s1,$zero, halt
	
	and $s0, $s0, $zero
	lw  $t0, num
	in_for:
		sub $t0,$t0,$s1

		blt $t0,$zero, out_for
		bne  $t0,$zero,continuein		
		li $v0, 4
		la  $a0, hyphen
		syscall
		
		li $v0, 1
		la  $a0,($s1)
		syscall
		
		
		continuein:
			j in_for		
		
		
		
	addi $t0, $t0,-1
	j out_for
			
	


	
	
	
	
	



halt:
li $v0, 4
	la   $a0, newline
	syscall
	li $v0, 10 # syscall code 10 is for exit.
	syscall
