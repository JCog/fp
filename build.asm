.n64
.relativeinclude on

.create "rom/fp.z64", 0
.incbin "rom/base.z64"

.definelabel PAYLOAD_ROM, 0x02800000
.definelabel PAYLOAD_RAM, 0x80400000
.definelabel FP_RAM,      PAYLOAD_RAM + 0x40
.definelabel DMA_FUNC,    0x800296FC

;=================================================
; Base ROM Editing Region
;=================================================
.headersize (0x80025C00 - 0x00001000)

;hook into early boot function, load payload
.org 0x8004AA2C ;rom 0x00025E2C
	lui   a0, hi(PAYLOAD_ROM)     ;payload rom: 0x02800000
	li    a1, (END - PAYLOAD_RAM + PAYLOAD_ROM) ;payload end
	lui   a2, hi(PAYLOAD_RAM)     ;payload ram: 0x80400000
	jal   DMA_FUNC                ;dma
	nop
	jal   init                    ;displaced boot routine
	nop

;graph thread main hook
;original function call will be displaced in c code. it takes no arguments
.org 0x80026C88 ;ROM 0x2088
jal FP_RAM

;=================================================
; New Code Region
;=================================================
.headersize (PAYLOAD_RAM - PAYLOAD_ROM)
.org PAYLOAD_RAM
init:
	;displaced
	lui   s0, 0x800A
	addiu s0, s0, 0xED68
	daddu a0, s0, r0
	addiu a1, r0, 0x0003
	lui   a2, 0x8005
	addiu a2, a2, 0xAAB8
	lui   v0, 0x800A
	jr    ra
	nop

.org FP_RAM
.incbin "bin/fp.bin"
.align 8
END:
.close