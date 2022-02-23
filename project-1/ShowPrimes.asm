.data
num: 		.word 1000
num2:		.word 500

nline:		.asciiz "\n"
prime:		.asciiz " Prime "


.text
#.globl main
main:
	li $t0, 1#number
out_loop:
	bgt $t0,1000,halt
	
	and $t1,$t1,$zero#count
	li $t2,2#i
	sra $t3,$t0,1 #number/2
inner:	bgt $t2,$t3,go_outer
		la $t4,($t0)
	mod:	sub $t4,$t4,$t2
		blt $t4,$zero,continue
		bne $t4,$zero, mod
			addi $t1,$t1,1
			j go_outer	
	
	continue:
		addi $t2, $t2,1
		j inner	
	go_outer:
            
			li $v0, 4
			la $a0, nline
			syscall
            li $v0, 1
			la $a0, ($t0)
			syscall
			
		bne $t1,$zero,go_outer2
		beq $t0,1,go_outer2
			#li $v0, 1
			#la $a0, ($t0)
			#syscall
			li $v0, 4
			la $a0, prime
			syscall
			
			
	go_outer2:
		addi $t0,$t0,1
		j out_loop
		
		
	
		
	
	
	
	
	
	
	
	
	
halt:	
	li $v0, 10 # syscall code 10 is for exit.
	syscall
