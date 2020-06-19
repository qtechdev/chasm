# Chip8 Instructions
## OpCode Table
opcode  | mnemonic          | info
--------|-------------------|-----
00e0    | clear             | clears the display
00ee    | ret
1nnn    | jmp [addr]
2nnn    | call [addr]
3xnn    | seq [r] [v]       | skip next instruction if equal
4xnn    | sne [r] [v]       | skip next instruction if not equal
5xy0    | seqr [r] [r]      | skip next instruction if equal
6xnn    | mov [r] [v]
7xnn    | add [r] [v]
8xy0    | movr [r] [r]
8xy1    | or [r] [r]
8xy2    | and [r] [r]
8xy3    | xor [r] [r]
8xy4    | addr [r] [r]      | carry stored in `VF`
8xy5    | sub [r] [r]       | !borrow stored in `VF`
8xy6    | slr [r] [r]       | lowest bit stored in `VF`
8xy7    | rsub [r] [r]
8xye    | sll [r] [r]       | highest bit stored in `VF`
9xy0    | sner [r] [r]      | skip next instruction if not equal
annn    | movi [addr]       | stores address in `I`
bnnn    | jmpv [addr]       | jumps to address + value of `V0`
cxnn    | rand [r] [v]      | stores a random value with mask of `v`
dxyn    | draw [r] [r] [n]  | see [drawing](drawing,md)
ex9e    | keq [r]           | skip next instruction if key `Vr` pressed
exa1    | kne [r]           | skip next instruction if key `Vr` not pressed
fx07    | std [r]           | store delay timer
fx0a    | key [r]           | blocks until key press
fx15    | ldd [r]           | set delay timer
fx18    | lds [r]           | set sound timer
fx1e    | addi [r]
fx29    | sprite [r]        | store location of sprite `Vr` to `I`
fx33    | bcd [r]           | store bcd of `Vr` starting at memory location `I`
fx55    | str [r]           | store registers up to `Vr` at memory location `I`
fx65    | ldr [r]           | load registers up to `Vr` from memory location `I`

Two additional opcodes are available for this assembler.

opcode  | mnemonic          | info
--------|-------------------|-----
0000    | nop
ffff    | halt

## Value Types
Four types of values can be used as arguments.

type  | value range  
------|--------------
addr  | 0x000 - 0xfff
v     | 0x00 - 0xff
n     | 0x0 - 0xf
r     | V0 - VF      
