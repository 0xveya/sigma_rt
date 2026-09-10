.section .text
.global _start
.type _start, @function

.extern sigma_entry
.extern sigma_tls_init
_start:
	xor %ebp, %ebp
	mov (%rsp), %r12d
	lea 8(%rsp), %r13
	lea 16(%rsp,%r12,8), %r14
	andq $-16, %rsp
	mov %r14, %rdi
	call sigma_tls_init
	test %al, %al
	jz .Ltls_failed
	mov %r12d, %edi
	mov %r13, %rsi
	mov %r14, %rdx
	call sigma_entry
	mov %eax, %edi
	mov $60, %eax
	syscall

.Ltls_failed:
	mov $127, %edi
	mov $60, %eax
	syscall
	ud2

# ELF metadata:
# size of _start = current assembler position minus address of _start
.size _start, .-_start

# tell the linker this object does not require an executable stack
.section .note.GNU-stack,"",@progbits
