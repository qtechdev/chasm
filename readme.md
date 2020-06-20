# chasm

A chip8 assembler.

- assembles chip8 assembly as found in [instructions.md](docs/instructions.md)
- can handle labels used by jump instructions
- calculate label offsets with a fist pass (allows use before definition)

# TODO
- add a way to specify data in assembly file
- add pseudo instructions? (loop, mul, div, etc.)
- add easy syntax for functions (auto calling conventions)
- add support for multiple source files
- add comments
- add support for other number bases (dec, bin, oct?)
