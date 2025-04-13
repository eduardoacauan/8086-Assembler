# 8086 Assembler

This is an unfinished 8086 (16-bit) assembler, written in the C programming language.

# How it works:
Given a file like file.asm:
```asm
mov ax, 0xDEAD
int 0x10
```
It gets translated into a stack-machine intermediate representation:
```asm
REGISTER
IMMEDIATE 0xDEAD
MOV
IMMEDIATE 0x10
INT
```
After a second pass (constant folding and instruction merging), it becomes:
```asm
MOV
INT
```
Finally, it is translated into raw machine code:
```asm
0xB8 0xAD 0xDE 0xCD 0x10
```
