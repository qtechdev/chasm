# Drawing
opcode  | mnemonic
--------|---------
dxyn    | draw [r_x] [r_y] [n]

Draws an 8 pixel wide and `n` pixel tall sprite at the coordinates
(`Vr_x`, `Vr_y`).  
Sprite data is stored in memory starting at the address held by `I`.  
Each pixel in the sprite is xor'd with the current screen data; any overlap
results in `VF` being set.
