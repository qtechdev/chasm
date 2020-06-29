# qChip assembly language

## instructions
OpCodes are specified in the [opcode table](opcode_table.md).

## labels
Labels can be used to automatically calculate address offsets.  
_MUST_ begin with a colon `:` and contain only alphabetic characters `a - z` or
underscores `_` .  
A label _MAY_ be defined after use.

## data
Data can be passed directly with the psudo-opcode `$` .  
A whole word (16 bits) _MUST_ be given to ensure alignment.
