# qch_asm
Converts qChip assembly to chip8 instructions.

- see [assembly.md](docs/assembly.md) for spec
- can handle labels used by jump instructions
- calculate label offsets with a fist pass (allows use before definition)
- add a way to specify data in assembly file

# TODO
- add pseudo instructions? (loop, mul, div, etc.)
- add easy syntax for functions (auto calling conventions)
- add support for multiple source files
- add comments
- add support for other number bases (dec, bin, oct?)
