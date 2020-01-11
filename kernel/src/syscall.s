global syscall_handler
syscall_handler:
	push rcx
	mov rcx, r10

	extern syscall_handlers
	mov r10, [rax * 8 + syscall_handlers]
	call r10
	pop rcx
	o64 sysret
