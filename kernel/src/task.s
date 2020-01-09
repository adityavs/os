global context_switch
context_switch:
	push rbp
	pushfq

	; Save previous task stack pointer
	mov [rdi + 8], rsp

	; Load new task stack pointer
	mov rsp, [rsi + 8]

	; Change TSS's RSP0
	extern tss
	mov rax, [rsi + 16]
	mov qword [tss + 4], rax

	; Change CR3
	mov rax, [rsi + 24]
	mov cr3, rax

	popfq
	pop rbp
	ret
