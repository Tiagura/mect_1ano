; somar os valores de um array

        .data
array:  .word 1,2,3,4,5
nelem:  .word 5
sum:    .space 4
        .text
        .global main

main:   addi r1, r0, nelem
	nop;
	nop;
        lw r1, 0(r1)
        
        addi r2, r0, array       ; r2 = array[0]

        addi r3, r0, 0          ; sum = 0

        addi r4, r0, 0          ; i = 0
loop:   lw r5, 0(r2)

	nop;
	nop;

        add r3, r3, r5          ; sum += array[i]
        addi r4, r4, 1          ; i += 1
        addi r2, r2, 4          ; r2 = arra[i]
	
	nop;

        sub r7, r1, r4
	
	nop;
	nop;

        bnez r7, loop

	nop;
	nop;
	nop;

        addi r6, r0, sum

	nop;
	nop;

        sw 0(r6), r3

        trap 0
