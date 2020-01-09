bits 16
section .boot
global boot
boot:
	; Setup stack
	xor ax, ax
	mov ss, ax
	mov sp, 0x7000

	; Set up registers
	xor ax, ax
	mov ds, ax
	mov es, ax

	; Clear screen and reset cursor
	mov ah, 0x0
	mov al, 0x3
	int 0x10

	; Check if extended read is available
	mov ah, 0x41
	mov bx, 0x55aa
	mov dl, 0x80
	int 0x13
	jc extensions_error

	; Extended read sectors from disk
	mov ah, 0x42
	mov si, dap
	mov dl, 0x80
	int 0x13
	jc read_error

	; Jumping to next stage
	jmp main16

extensions_error:
	mov si, .msg
	call print_string

	cli
	hlt

	.msg db "BIOS extensions are not supported.", 13, 10, 0

read_error:
	mov si, .msg
	call print_string

	cli
	hlt

	.msg db "Error reading disk.", 13, 10, 0


print_string:
	mov ah, 0xe
.loop:
	lodsb
	or al, al
	jz .done
	int 0x10
	jmp .loop
.done:
	ret


; Data
dap:
.size		db 0x10
.unused		db 0x0
.sectors	dw 0x80
.offset		dw 0x0
.segment	dw 0x7E0
.lba		dq 0x1

; Padding
times 510 - ($ - $$) db 0
dw 0xaa55

main16:
	call get_mmap

	; Set up paging
	; zero out a 16KiB buffer at 0x1000
	mov edi, 0x1000
	mov ecx, 0x1000
	xor eax, eax
	cld
	rep stosd

	mov edi, 0x1000
	; build P4
	lea eax, [es:di + 0x1000]
	or eax, 0x3
	mov [es:di], eax

	; build P3
	lea eax, [es:di + 0x2000]
	or eax, 0x3
	mov [es:di + 0x1000], eax

	; build P2
	lea eax, [es:di + 0x3000]
	or eax, 0x3
	mov [es:di + 0x2000], eax

	; build P1s
	lea di, [di + 0x3000]
	mov eax, 0x3
.loop:
	mov [es:di], eax
	add eax, 0x1000
	add di, 8
	cmp eax, 0x200000	; 2 MiB
	jb .loop

	; Disable IRQs
	mov al, 0xFF
	out 0xA1, al
	out 0x21, al

	; Load a zero length IDT so that any NMI causes a triple fault
	lidt [idt.descriptor]

	; Enter long mode
	; set PAE and PGE bits
	mov eax, 0b10100000
	mov cr4, eax

	; point CR3 at the P4
	mov edx, 0x1000
	mov cr3, edx

	; set the LME bit
	mov ecx, 0xC0000080
	rdmsr
	or eax, 0x100
	wrmsr

	; activate long mode
	mov ebx, cr0
	or ebx, 0x80000001
	mov cr0, ebx

	; Load the GDT
	lgdt [gdt.descriptor]

	; Load the TSS
	mov ax, gdt.tss
	ltr ax

	; Long mode jump
	jmp gdt.code:main64

get_mmap:
	xor ebp, ebp

	; 0x500 will contain the number of entries (ebp)
	mov di, 0x508
	xor ebx, ebx
	mov edx, 0x534D4150
	mov eax, 0xE820
	mov [es:di + 20], dword 1
	mov ecx, 24
	int 0x15
	jc short .error
	mov edx, 0x534D4150
	cmp eax, edx
	jne short .error
	test ebx, ebx
	je short .error
	jmp short .jmpin
.e820lp:
	mov eax, 0xe820
	mov [es:di + 20], dword 1
	mov ecx, 24
	int 0x15
	jc short .e820f
	mov edx, 0x534D4150
.jmpin:
	jcxz .skipentry
	cmp cl, 20
	jbe short .notext
	test byte [es:di + 20], 1
	je short .skipentry
.notext:
	mov ecx, [es:di + 8]
	or ecx, [es:di + 12]
	jz .skipentry
	inc bp
	add di, 24
.skipentry:
	test ebx, ebx
	jne short .e820lp
.e820f:
	mov [0x500], bp
	clc
	ret

.error:
	mov si, .msg
	call print_string

	cli
	hlt

	.msg db "Error mapping memory.", 0

; Global descriptor table
align 8
gdt:
	dq 0x0000000000000000
.code: equ $ - gdt
	dq 0x00209A0000000000
.data: equ $ - gdt
	dq 0x0000920000000000
.user_data: equ $ - gdt
	dq 0x0000F20000000000
.user_code: equ $ - gdt
	dq 0x0020FA0000000000
.tss: equ $ - gdt
	dw tss.size & 0xFFFF
	dw tss.base & 0xFFFF
	db (tss.base >> 16) & 0xFF
	db 0b11101001
	db 0b00010000 | (tss.size >> 16) & 0xF
	db (tss.base >> 24) & 0xFF
	dd (tss.base >> 32) & 0xFFFFFFFF
	dd 0
.descriptor:
	dw $ - gdt - 1
	dd gdt

; Task State Segment
align 8
global tss
tss:
.base: equ tss - $$
	dd 0		; reserved
	dq 0		; rsp0
	dq 0		; rsp1
	dq 0		; rsp2
	dd 0		; reserved
	dd 0		; reserved
	dq 0		; ist1
	dq 0		; ist2
	dq 0		; ist3
	dq 0		; ist4
	dq 0		; ist5
	dq 0		; ist6
	dq 0		; ist7
	dd 0		; reserved
	dd 0		; reserved
	dw 0		; reserved
	dw 0		; iopb offset
.size: equ $ - tss


; Interrupt descriptor table
align 8
idt:
.descriptor:
	dw 0
	dd 0

; Long mode code
bits 64
main64:
	; Set up registers
	mov ax, gdt.data
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	extern kernel_main
	call kernel_main

	cli
	hlt
