;r1 - add(n), val(n)
;r2 - add(values[i])
;r3 - val(i)
;r4 - val(values[i])
;r5 - add(values[j])
;r6 - val(j)
;r7 - val(values[j])
;r8 - n -1
;r9 - tmp

     .data
    
array: 	.word 3, 4, 1, 2, 5, 8, 6, 7, 9, 10
n:      .word 10

        .text
        .global main

main:	addi r1, r0, n;		r1=n
	lw r1, 0(r1); 		r1 = val(n) = 10
	
	addi r2, r0, array;	r2 = add(array[0])

        add r3, r0, r0; 	i = 0
        
	addi r8, r1, -1; 	r8 = n - 1 = 9

loop1:	slt r9, r3, r8;		r9 = r3 < r8
	beqz r9, end
	
	addi r5, r2, 4;		r5 = add(array[j=1])
	lw r4, 0(r2);		r4 = val(values[i=0])
	addi r6, r3, 1;		r6 - j = i + 1 

loop2:	lw r7, 0(r5);		r7 = val(values[j=1])
	slt r9, r4, r7;
	beqz r9, if;

	sw 0(r2), r7;	guarda em array[i] valor de r7 (array[j])
	sw 0(r5), r4;	guarda em array[j] valor de r4 (array[i])

if:	addi r6, r6, 1;		r6 = j + 1
	addi r5, r5, 4;		r5 = proxima posicao do array
	slt r9, r6, r1;
	bnez r9, loop2;

	addi r3, r3, 1;		r3 = i + 1
	addi r2, r2, 4;		r2 = proxima posicao do array
	j loop1;
	
end: 	trap 0
