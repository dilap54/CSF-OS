.data
	hello_str:
		.string "Hello, world!\n"

.text
	.globl  _start

_start:
	// системный вызов write
	movl    $4, %eax
	movl    $1, %ebx
	movl    $hello_str, %ecx
	movl    $14, %edx
	int     $0x80

	movl    $1, %eax
	movl    $0, %ebx
	int     $0x80
	