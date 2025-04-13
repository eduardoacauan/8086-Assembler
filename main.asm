; 8086 assembler
; useful for 16 bit bootloaders
			org  0x7c00

nowarns; ; disable warnings
trace;   ; enables assembler output

main:		mov		ah, 0x0E
			mov		al, 65
			int     0x10
			
			mov		bx, ax
			add		bx, cx

; this is a sample program