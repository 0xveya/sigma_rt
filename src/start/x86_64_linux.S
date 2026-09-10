.section .text
.global _start
.type _start, @function

.extern sigma_entry
_start:
	xor %ebp, %ebp
	mov (%rsp), %edi
	lea 8(%rsp), %rsi
	lea 16(%rsp,%rdi,8), %rdx
	andq $-16, %rsp
	call sigma_entry
	mov %eax, %edi
	mov $60, %eax
	syscall
	ud2

# ELF metadata:
# size of _start = current assembler position minus address of _start
.size _start, .-_start

# tell the linker this object does not require an executable stack
.section .note.GNU-stack,"",@progbits
