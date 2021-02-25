.cpu cortex-m0
.thumb
.syntax unified
.fpu softvfp

.global login
login: .string "bsalha"
hello_str: .string "Hello, %s!\n"
.align 2
.global hello
hello:
	push {lr}
	ldr r0, =hello_str
	ldr r1, =login
	bl printf
	pop  {pc}

showmult2_str: .string "%d * %d = %d\n"
.align 2
.global showmult2
showmult2:
	push {lr}
	movs r2, r0 // r2 = a
	movs r3, r1 // r3 = b

	ldr r0, =showmult2_str
	movs r1, r2 // r1 = a
	movs r2, r3 // r2 = b
	muls r3, r1 // r3 = a*b
	bl printf
	pop  {pc}

showmult3_str: .string "%d * %d * %d = %d\n"
.align 2
.global showmult3
showmult3:
	push {lr}
	movs r3, r0 // r5 = a

	muls r3, r1 // r3=a*b
	muls r3, r2 // r3=a*b*c

	sub sp, #4
	str r3, [sp, #0] // arg4 = a*b*c
	movs r3, r2 // arg3 = c
	movs r2, r1 // arg2 = b
	movs r1, r0 // arg1 = a
	ldr r0, =showmult3_str // arg0 = format
	bl printf
	add sp, #4
	pop {pc}

listing_str: .string "%s %05d %s %d students in %s, %d\n"
.global listing
listing:
	push {r4, lr}
	// r0 = school
	// r1 = course
	// r2 = verb
	// r3 = enrollment

	ldr r4, [sp, #12]
	sub sp, #4
	str r4, [sp, #0] // arg6 = year

	ldr r4, [sp, #12]
	sub sp, #4
	str r4, [sp, #0] // arg5 = season

	sub sp, #4
	str r3, [sp, #0] // arg4 = enrollment

	movs r3, r2 // arg3 = verb
	movs r2, r1 // arg2 = course
	movs r1, r0 // arg1 = school
	ldr r0, =listing_str // arg0 = format

	bl printf

	add sp, #12

	pop {r4, pc}

.global trivial
trivial:
	push {r4, lr}

	sub sp, #400
	mov r2, sp  // r3 = &tmp[0]

	movs r1, #0 // r1 = x
for1:
	cmp r1, #100
	bge endfor1

	adds r3, r1, #1 // r3 = x + 1

	lsls r4, r1, #2
	str r3, [r2, r4] // tmp[x] = x + 1

	movs r1, r3 // x = x + 1
	b for1
endfor1:

if1:
	cmp r0, #99
	bls endif1
then1:
	movs r0, #99
endif1:

	lsls r4, r0, #2
	ldr r0, [r2, r4] // return tmp[n]
	add sp, #400
	pop {r4, pc}

.global reverse_puts
reverse_puts:
	push {r4, r5, r6, r7, lr}

	movs r4, r0 // r4 = s
	bl strlen   // r0 = len

	adds r1, r0, #4 // len + 4
	movs r2, #3
	bics r1, r2 // r1 = newlen = (len + 4) & ~3

	mov r5, sp
	subs r5, r1
	mov sp, r5
	mov r5, sp // r5 = buffer[newlen]

	movs r2, #0
	strb r2, [r5, r0] // buffer[len] = 0

	movs r3, #0 // r3 = x
for2:
	subs r2, r0, #1 // r2 = len-1
	cmp r3, r2
	bhi endfor2

	movs r7, #1
	subs r2, r0, r3 // r2 = len - x
	subs r2, r7     // r2 = len - x - 1

	ldrb r6, [r4, r3] // r6 = s[x]
	strb r6, [r5, r2] // buffer[len-x-1] = s[x]

	adds r3, #1
	b for2
endfor2:
	movs r0, r5

	movs r4, r1
	bl puts // puts(buffer)

	add sp, r4
	pop {r4, r5, r6, r7, pc}

.global sumsq
sumsq:
	push {r4, r5, r6, lr}

	sub sp, #400
	mov r2, sp // r2 = &tmp[0]

if2:
	cmp r0, #99
	bls endif2
then2:
	movs r0, #99 // a = 99
endif2:


if3:
	cmp r1, #99
	bls endif3
then3:
	movs r1, #99 // b = 99
endif3:

	movs r3, #1 // r3 = step
if4:
	cmp r0, r1
	bne else4
then4:
	movs r3, #0 // step = 0
	b endif4
else4:


if5:
	cmp r0, r1
	bls endif5
then5:
	subs r3, #2 // step = -1
endif5:


endif4:


	movs r4, #0 // r4 = x
for3:
	cmp r4, #100
	bgt endfor3

	movs r5, r4
	muls r5, r5 // r5 = x * x
	lsls r6, r4, #2
	str r5, [r2, r6] // tmp[x] = x * x
	//bkpt
	adds r4, #1
	b for3
endfor3:


	movs r5, #0 // r5 = sum
	movs r4, r0 // x = a
for4:
	lsls r6, r4, #2
	ldr r6, [r2, r6] // r6 = tmp[x]
	adds r5, r6 // sum += tmp[x]


if6:
	cmp r4, r1
	beq return_sum

	adds r4, r3 // x += step
	b for4
endfor4:

return_sum:
	movs r0, r5
	add sp, #400
	pop {r4, r5, r6, pc}
