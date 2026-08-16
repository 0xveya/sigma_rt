.section .text
.global _start
.type _start, @function

.extern sigma_entry
.extern __libc_start_main

_start:

	# erm aktschually xoring register with itself makes it zero 
	# also this isa subregister if zeroed like expands and 
	# so its more fast erm gleep sub thing of rdp which sysv abi
	# page 28 says i should zero out biascly it all is like page 27 to 29
	# https://refspecs.linuxfoundation.org/elf/x86_64-abi-0.98.pdf
	xor %ebp, %ebp

	# some stuff __libc_start_main wants for dynamic linking as
	# arg 6 so guess i gotta move it in the register for it
	mov %rdx, %r9

	# move strack pointer 8 byte forward and store so we have argc
	# figure 3.9 has the stack layout
	popq %rsi

	# copy pointer to argv into rdx bc __libc_start_main wants it
	# and i kinda also need it perhaps
	mov %rsp, %rdx

	# clear button 4 bits of rsp cuz pdf tells me to 🥺
	andq $-16, %rsp

	# reserve 8 bytes of padding for stack alignment
	pushq %rax

	# push stackpointer to stack
	pushq %rsp

	# NULL ecx for libc stuff bc it wants it
	xor %ecx, %ecx

	# NULL other arg for libc
	xor %r8d, %r8d

	# basicly lets pointer to addr of the sigma_entry func into rdi
	# lea = load effective address
	lea sigma_entry(%rip), %rdi

	
	# at this point:
	#
	# rdi = sigma_entry
	# rsi = argc
	# rdx = argv
	# rcx = NULL
	# r8  = NULL
	# r9  = rtld_fini
	# stack arg = stack_end
	#
	# libc initializes its runtime and eventually calls:
	#
	# sigma_entry(argc, argv, envp)
	call *__libc_start_main@GOTPCREL(%rip)

	# __libc_start_main is not supposed to return.
	# should it somehow return, stop here instead of executing random bytes.
	hlt

# ELF metadata:
# size of _start = current assembler position minus address of _start
.size _start, .-_start

# tell the linker this object does not require an executable stack
.section .note.GNU-stack,"",@progbits
