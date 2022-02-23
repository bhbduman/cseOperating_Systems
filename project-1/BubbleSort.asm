.data
#int arraySize = MAX_SIZE;
arraySize: 	.word 0
#int arr[MAX_SIZE];
arr: 		.space 400
#int num;
num: 		.word 0
hyphen: 	.asciiz " "
arrSize: .asciiz "\nEnter arraySize\n"
nums: .asciiz "\nEnter Numbers\n"
.text
#.globl main
main:
    
    la $a0, arrSize
    li $v0, 4
    syscall
	li $v0, 5
	syscall
	sw  $v0,arraySize
	
	la $a0, nums
    	li $v0, 4
   	 syscall
    
	and $s0, $s0, $zero
	lw  $t0, arraySize
	li $s1,0
fill:	
	beq $s0,$t0,bubble_sort	
	li $v0, 5
	syscall
	
	sw   $v0, arr($s1)
	add  $s1,$s1,4
	addi $s0,$s0,1
	j fill

bubble_sort:
	and $s7, $s7, $zero
	lw  $t0, arraySize
	addi $t0, $t0, -1#n-1
O_loop:	
	beq $t0,$s7, print#(i = 0; i < n-1; i++)
		and $s6, $s6, $zero#j
		lw  $t1, arraySize
		addi $t1, $t1, -1
		sub $t1, $t1, $s7#n-i-1
	I_loop:	
		beq $t0,$s6, out
		
			sll $s6,$s6,2#j+1
			lw $t4, arr($s6)#arr[j]
			srl $s6,$s6,2
			addi $t6,$s6,1
			sll $t6,$t6,2#
			lw $t5, arr($t6)#arr[j+1]	
		ble $t4, $t5, continue
			sll $s6,$s6,2
			lw $t6, arr($s6)
			sw $t5, arr($s6)
			srl $s6,$s6,2
			
			addi $t3,$s6,1
			sll $t3,$t3,2#
			sw $t6,arr($t3)			
			
		continue:
		addi $s6, $s6, 1
		j I_loop
	out:
	addi $s7, $s7, 1
	j O_loop
	



print:	and $s0, $s0, $zero
	lw  $t0, arraySize
	li $s1,0	
p_loop:
	beq $s0,$t0,halt	
    	#cout >> arr[i];
	li $v0, 1
	lw   $a0, arr($s1)
	syscall
	li $v0, 4
		la  $a0, hyphen
		syscall
	
	add  $s1,$s1,4
	addi $s0,$s0,1
	j p_loop

halt:	
	li $v0, 10 # syscall code 10 is for exit.
	syscall
	
	
