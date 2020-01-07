global syscall_handler
syscall_handler:
	push rcx
	mov rcx, r10

	extern syscall_handlers
	mov r10, [rax * 8 + syscall_handlers]
	test r10, r10
	je .skip
	call r10
.skip:
	pop rcx
	o64 sysret
