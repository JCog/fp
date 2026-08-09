#ifndef DISASM_H
#define DISASM_H

#include "types.h"

#define OP(inst)                ((inst) >> 26)
#define RS(inst)                (((inst) >> 21) & 0x1F)
#define RT(inst)                (((inst) >> 16) & 0x1F)
#define RD(inst)                (((inst) >> 11) & 0x1F)
#define SA(inst)                (((inst) >> 6) & 0x1F)
#define FUNCT(inst)             ((inst) & 0x3F)
#define IMM(inst)               ((s16)((inst) & 0xFFFF))
#define TARGET(inst, pc)        (((pc) & 0xF0000000) | (((inst) & 0x3FFFFFF) << 2))
#define CODE(inst)              (((inst) >> 16) & 0x3FF)
#define FS(inst)                (((inst) >> 11) & 0x1F)
#define FT(inst)                (((inst) >> 16) & 0x1F)
#define FD(inst)                (((inst) >> 6) & 0x1F)
#define BRANCH_TARGET(inst, pc) ((pc) + 4 + (IMM(inst) << 2))

const char *disasmInstruction(u32 inst, u32 pc);

#endif
