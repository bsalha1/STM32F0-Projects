.cpu cortex-m0
.thumb
.syntax unified
.fpu softvfp

.data
.align 4

.global source
source: .word 10, 8, 2, 4, 5, 9, 12, 8, 9, 10, 11

.global sum
sum: .word 0

.global str
str: .string "hello, 01234 world! 56789++"
// Your global variables go here


.text
.global intsub
intsub:
    // Your code for intsub goes here
	movs r0, #0
for1:
	cmp r0, #10
	bge endfor1

    ldr r1, =source
	lsls r2, r0, #2
	ldr r1, [r1, r2] // r1 = source[i]

	ldr r2, =sum
	ldr r3, [r2]     // r3 = sum

	adds r3, r1      // r3 = sum + source[i]
	str r3, [r2]     // sum = r3

if1:
	cmp r3, #12
	ble else1
then1:
    ldr r1, =source
    subs r2, r0, #1  // r2 = i - 1
	lsls r2, #2
	ldr r1, [r1, r2] // r1 = source[i - 1]

	subs r3, r1 // r3 = sum - source[i - 1]
	ldr r2, =sum
	str r3, [r2] // sum = r3
	b endif1
else1:
	subs r3, #1 // r3 = sum - 1
	ldr r2, =sum
	str r3, [r2] // sum = r3
endif1:

	adds r0, #1
	b for1

endfor1:
    bx lr



.global charsub
charsub:
    // Your code for charsub goes here
    movs r0, #0 // x = 0
	ldr r1, =str
for2:
	ldrb r2, [r1, r0] // r2 = str[x]
	cmp r2, #0
	beq endfor2 // str[x] == 0 -> break out

if2:
	cmp r2, #0x30
	blt endif2   // r2 < '0' -> break out

	cmp r2, #0x65
	bgt endif2   // r2 > 'e' -> break out
then2:
	movs r2, #0x2f
	strb r2, [r1, r0]

endif2:
	adds r0, #1 // x++
	b for2

endfor2:
    bx lr


.global login
login: .string "bsalha" // Make sure you put your login here.
.align 2
.global main
main:
    bl autotest // uncomment AFTER you debug your subroutines
    bl intsub
    bl charsub
    bkpt
