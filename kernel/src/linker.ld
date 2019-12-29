ENTRY(boot)
SECTIONS {
	. = 0x7C00;
	.boot : { *(.boot) }
	.text : { *(.text) }
}
