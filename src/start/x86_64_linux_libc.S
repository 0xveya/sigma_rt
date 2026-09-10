.section .text
.global _start
.type _start, @function

.extern sigma_entry
.extern __libc_start_main

_start:
	xor %ebp, %ebp
	mov %rdx, %r9
	popq %rsi
	mov %rsp, %rdx
	andq $-16, %rsp
	pushq %rax
	pushq %rsp
	xor %ecx, %ecx
	xor %r8d, %r8d
	lea sigma_entry(%rip), %rdi
	call *__libc_start_main@GOTPCREL(%rip)
	hlt

.size _start, .-_start
.section .note.GNU-stack,"",@progbits
