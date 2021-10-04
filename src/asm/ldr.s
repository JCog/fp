.section  .text, "ax", @progbits
.set      nomips16
.set      nomicromips

.global   _start
.ent      _start
.type     _start, @function
_start:
li  $a0, 0x2800000
li  $a1, (END)
lui $a2, 0x8040
jal (DMA_COPY)
j 0x80400000
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
.end _start
.size _start, . - _start
