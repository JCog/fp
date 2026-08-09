#include "disasm.h"
#include "macros.h"
#include "types.h"
#include <stdio.h>

typedef enum {
    INST_FMT_INVALID,
    INST_FMT_T_S_HEX,
    INST_FMT_T_S_SIMM,
    INST_FMT_T_HEX,
    INST_FMT_S_HEX,
    INST_FMT_T_OFF_S,
    INST_FMT_TARGET,
    INST_FMT_NONE,
    INST_FMT_D_S_T,
    INST_FMT_D_T_SA,
    INST_FMT_D_T_S,
    INST_FMT_S_T,
    INST_FMT_D_S,
    INST_FMT_D,
    INST_FMT_S,
    INST_FMT_S_T_BRANCH,
    INST_FMT_S_BRANCH,
    INST_FMT_FT_OFF_S,
    INST_FMT_CACHE
} InstFormat;

typedef struct {
    const char *mnemonic;
    InstFormat format;
} InstDesc;

static const char *gprNames[] = {"zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2",
                                 "t3",   "t4", "t5", "t6", "t7", "s0", "s1", "s2", "s3", "s4", "s5",
                                 "s6",   "s7", "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"};

static const char *fprNames[] = {"f0",  "f1",  "f2",  "f3",  "f4",  "f5",  "f6",  "f7",  "f8",  "f9",  "f10",
                                 "f11", "f12", "f13", "f14", "f15", "f16", "f17", "f18", "f19", "f20", "f21",
                                 "f22", "f23", "f24", "f25", "f26", "f27", "f28", "f29", "f30", "f31"};

static const char *cop1FmtNames[] = {[0x10] = "s", [0x11] = "d", [0x14] = "w", [0x15] = "l"};

static const char *cop1Funcs[64] = {[0x00] = "add",     [0x01] = "sub",     [0x02] = "mul",    [0x03] = "div",
                                    [0x04] = "sqrt",    [0x05] = "abs",     [0x06] = "mov",    [0x07] = "neg",
                                    [0x08] = "round.l", [0x09] = "trunc.l", [0x0A] = "ceil.l", [0x0B] = "floor.l",
                                    [0x0C] = "round.w", [0x0D] = "trunc.w", [0x0E] = "ceil.w", [0x0F] = "floor.w",
                                    [0x20] = "cvt.s",   [0x21] = "cvt.d",   [0x24] = "cvt.w",  [0x25] = "cvt.l",
                                    [0x30] = "c.f",     [0x31] = "c.un",    [0x32] = "c.eq",   [0x33] = "c.ueq",
                                    [0x34] = "c.olt",   [0x35] = "c.ult",   [0x36] = "c.ole",  [0x37] = "c.ule",
                                    [0x38] = "c.sf",    [0x39] = "c.ngle",  [0x3A] = "c.seq",  [0x3B] = "c.ngl",
                                    [0x3C] = "c.lt",    [0x3D] = "c.nge",   [0x3E] = "c.le",   [0x3F] = "c.ngt"};

static const char *cop1Moves[] = {[0x00] = "mfc1", [0x01] = "dmfc1", [0x02] = "cfc1",
                                  [0x04] = "mtc1", [0x05] = "dmtc1", [0x06] = "ctc1"};

static const char *cop0Moves[] = {[0x00] = "mfc0", [0x01] = "dmfc0", [0x04] = "mtc0", [0x05] = "dmtc0"};

static const char *cop0Funcs[64] = {
    [0x01] = "tlbr", [0x02] = "tlbwi", [0x06] = "tlbwr", [0x08] = "tlbp", [0x18] = "eret"};

static const char *cop0RegNames[] = {"Index",    "Random",   "EntryLo0", "EntryLo1", "Context",  "PageMask", "Wired",
                                     "Reserved", "BadVAddr", "Count",    "EntryHi",  "Compare",  "Status",   "Cause",
                                     "EPC",      "PRId",     "Config",   "LLAddr",   "WatchLo",  "WatchHi",  "XContext",
                                     "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "ECC",      "CacheErr",
                                     "TagLo",    "TagHi",    "ErrorEPC", "Reserved"};

static const InstDesc opcodes[64] = {
    {"special"},
    {"regimm"},
    {"j", INST_FMT_TARGET},
    {"jal", INST_FMT_TARGET},
    {"beq", INST_FMT_S_T_BRANCH},
    {"bne", INST_FMT_S_T_BRANCH},
    {"blez", INST_FMT_S_BRANCH},
    {"bgtz", INST_FMT_S_BRANCH},
    {"addi", INST_FMT_T_S_SIMM},
    {"addiu", INST_FMT_T_S_SIMM},
    {"slti", INST_FMT_T_S_SIMM},
    {"sltiu", INST_FMT_T_S_SIMM},
    {"andi", INST_FMT_T_S_HEX},
    {"ori", INST_FMT_T_S_HEX},
    {"xori", INST_FMT_T_S_HEX},
    {"lui", INST_FMT_T_HEX},
    {"cop0"},
    {"cop1"},
    {"cop2"},
    {"invalid"},
    {"beql", INST_FMT_S_T_BRANCH},
    {"bnel", INST_FMT_S_T_BRANCH},
    {"blezl", INST_FMT_S_BRANCH},
    {"bgtzl", INST_FMT_S_BRANCH},
    {"daddi", INST_FMT_T_S_SIMM},
    {"daddiu", INST_FMT_T_S_SIMM},
    {"ldl", INST_FMT_T_OFF_S},
    {"ldr", INST_FMT_T_OFF_S},
    {"invalid"},
    {"invalid"},
    {"invalid"},
    {"invalid"},
    {"lb", INST_FMT_T_OFF_S},
    {"lh", INST_FMT_T_OFF_S},
    {"lwl", INST_FMT_T_OFF_S},
    {"lw", INST_FMT_T_OFF_S},
    {"lbu", INST_FMT_T_OFF_S},
    {"lhu", INST_FMT_T_OFF_S},
    {"lwr", INST_FMT_T_OFF_S},
    {"lwu", INST_FMT_T_OFF_S},
    {"sb", INST_FMT_T_OFF_S},
    {"sh", INST_FMT_T_OFF_S},
    {"swl", INST_FMT_T_OFF_S},
    {"sw", INST_FMT_T_OFF_S},
    {"sdl", INST_FMT_T_OFF_S},
    {"sdr", INST_FMT_T_OFF_S},
    {"swr", INST_FMT_T_OFF_S},
    {"cache", INST_FMT_CACHE},
    {"ll", INST_FMT_T_OFF_S},
    {"lwc1", INST_FMT_FT_OFF_S},
    {"lwc2"},
    {"invalid"},
    {"lld", INST_FMT_T_OFF_S},
    {"ldc1", INST_FMT_FT_OFF_S},
    {"ldc2"},
    {"ld", INST_FMT_T_OFF_S},
    {"sc", INST_FMT_T_OFF_S},
    {"swc1", INST_FMT_FT_OFF_S},
    {"swc2"},
    {"invalid"},
    {"scd", INST_FMT_T_OFF_S},
    {"sdc1", INST_FMT_FT_OFF_S},
    {"sdc2"},
    {"sd", INST_FMT_T_OFF_S}
};

static const InstDesc specialFuncts[64] = {
    {"sll", INST_FMT_D_T_SA},
    {"invalid"},
    {"srl", INST_FMT_D_T_SA},
    {"sra", INST_FMT_D_T_SA},
    {"sllv", INST_FMT_D_T_S},
    {"invalid"},
    {"srlv", INST_FMT_D_T_S},
    {"srav", INST_FMT_D_T_S},
    {"jr", INST_FMT_S},
    {"jalr", INST_FMT_D_S},
    {"invalid"},
    {"invalid"},
    {"syscall", INST_FMT_NONE},
    {"break", INST_FMT_NONE},
    {"invalid"},
    {"sync", INST_FMT_NONE},
    {"mfhi", INST_FMT_D},
    {"mthi", INST_FMT_S},
    {"mflo", INST_FMT_D},
    {"mtlo", INST_FMT_S},
    {"dsllv", INST_FMT_D_T_S},
    {"invalid"},
    {"dsrlv", INST_FMT_D_T_S},
    {"dsrav", INST_FMT_D_T_S},
    {"mult", INST_FMT_S_T},
    {"multu", INST_FMT_S_T},
    {"div", INST_FMT_S_T},
    {"divu", INST_FMT_S_T},
    {"dmult", INST_FMT_S_T},
    {"dmultu", INST_FMT_S_T},
    {"ddiv", INST_FMT_S_T},
    {"ddivu", INST_FMT_S_T},
    {"add", INST_FMT_D_S_T},
    {"addu", INST_FMT_D_S_T},
    {"sub", INST_FMT_D_S_T},
    {"subu", INST_FMT_D_S_T},
    {"and", INST_FMT_D_S_T},
    {"or", INST_FMT_D_S_T},
    {"xor", INST_FMT_D_S_T},
    {"nor", INST_FMT_D_S_T},
    {"invalid"},
    {"invalid"},
    {"slt", INST_FMT_D_S_T},
    {"sltu", INST_FMT_D_S_T},
    {"dadd", INST_FMT_D_S_T},
    {"daddu", INST_FMT_D_S_T},
    {"dsub", INST_FMT_D_S_T},
    {"dsubu", INST_FMT_D_S_T},
    {"tge", INST_FMT_S_T},
    {"tgeu", INST_FMT_S_T},
    {"tlt", INST_FMT_S_T},
    {"tltu", INST_FMT_S_T},
    {"teq", INST_FMT_S_T},
    {"invalid"},
    {"tne", INST_FMT_S_T},
    {"invalid"},
    {"dsll", INST_FMT_D_T_SA},
    {"invalid"},
    {"dsrl", INST_FMT_D_T_SA},
    {"dsra", INST_FMT_D_T_SA},
    {"dsll32", INST_FMT_D_T_SA},
    {"invalid"},
    {"dsrl32", INST_FMT_D_T_SA},
    {"dsra32", INST_FMT_D_T_SA}
};

static const InstDesc regimmRt[32] = {
    {"bltz", INST_FMT_S_BRANCH},
    {"bgez", INST_FMT_S_BRANCH},
    {"bltzl", INST_FMT_S_BRANCH},
    {"bgezl", INST_FMT_S_BRANCH},
    {"invalid"},
    {"invalid"},
    {"invalid"},
    {"invalid"},
    {"tgei", INST_FMT_S_HEX},
    {"tgeiu", INST_FMT_S_HEX},
    {"tlti", INST_FMT_S_HEX},
    {"tltiu", INST_FMT_S_HEX},
    {"teqi", INST_FMT_S_HEX},
    {"invalid"},
    {"tnei", INST_FMT_S_HEX},
    {"invalid"},
    {"bltzal", INST_FMT_S_BRANCH},
    {"bgezal", INST_FMT_S_BRANCH},
    {"bltzall", INST_FMT_S_BRANCH},
    {"bgezall", INST_FMT_S_BRANCH},
    {"invalid"}
};

#define INST_SPECIAL 0
#define INST_REGIMM  1
#define INST_COP0    16
#define INST_COP1    17

static char instBuf[40] = {0};

static bool disasmCop0(u32 inst) {
    u32 rs = RS(inst);

    if (rs < ARRAY_LENGTH(cop0Moves) && cop0Moves[rs] != NULL) {
        snprintf(instBuf, sizeof(instBuf), "%-5s %s, %s", cop0Moves[rs], gprNames[RT(inst)], cop0RegNames[RD(inst)]);
        return TRUE;
    }

    // CO == 1
    if (rs == 0x10 && cop0Funcs[FUNCT(inst)] != NULL) {
        snprintf(instBuf, sizeof(instBuf), "%s", cop0Funcs[FUNCT(inst)]);
        return TRUE;
    }

    return FALSE;
}

static bool disasmCop1(u32 inst, u32 pc) {
    u32 rs = RS(inst);
    u32 funct = FUNCT(inst);
    const char *fmt;
    const char *name;
    char mnemonic[12];

    if (rs == 0x02 || rs == 0x06) {
        // cfc1 and ctc1 directly reference control registers
        u32 fcr = FS(inst);

        if (fcr == 0) {
            snprintf(instBuf, sizeof(instBuf), "%-5s %s, FIR", cop1Moves[rs], gprNames[RT(inst)]);
        } else if (fcr == 31) {
            snprintf(instBuf, sizeof(instBuf), "%-5s %s, FCSR", cop1Moves[rs], gprNames[RT(inst)]);
        } else {
            snprintf(instBuf, sizeof(instBuf), "%-5s %s, fcr%ld", cop1Moves[rs], gprNames[RT(inst)], fcr);
        }
        return TRUE;
    }

    // cpu <-> fpu register data transfer
    if (rs < ARRAY_LENGTH(cop1Moves) && cop1Moves[rs] != NULL) {
        snprintf(instBuf, sizeof(instBuf), "%-5s %s, %s", cop1Moves[rs], gprNames[RT(inst)], fprNames[FS(inst)]);
        return TRUE;
    }

    if (rs == 0x08) {
        static const char *bc1[] = {"bc1f", "bc1t", "bc1fl", "bc1tl"};

        snprintf(instBuf, sizeof(instBuf), "%-5s 0x%08lx", bc1[RT(inst) & 3], BRANCH_TARGET(inst, pc));
        return TRUE;
    }

    fmt = (rs < ARRAY_LENGTH(cop1FmtNames)) ? cop1FmtNames[rs] : NULL;
    name = cop1Funcs[funct];
    if (fmt == NULL || name == NULL) {
        return FALSE;
    }
    snprintf(mnemonic, sizeof(mnemonic), "%s.%s", name, fmt);

    if (funct < 0x04) {
        snprintf(instBuf, sizeof(instBuf), "%-5s %s, %s, %s", mnemonic, fprNames[FD(inst)], fprNames[FS(inst)],
                 fprNames[FT(inst)]);
    } else if (funct >= 0x30) {
        snprintf(instBuf, sizeof(instBuf), "%-5s %s, %s", mnemonic, fprNames[FS(inst)], fprNames[FT(inst)]);
    } else {
        snprintf(instBuf, sizeof(instBuf), "%-5s %s, %s", mnemonic, fprNames[FD(inst)], fprNames[FS(inst)]);
    }

    return TRUE;
}

const char *disasmInstruction(u32 inst, u32 pc) {
    u32 op = OP(inst);
    InstDesc instOp = opcodes[op];

    if (inst == 0) {
        snprintf(instBuf, sizeof(instBuf), "nop");
        return instBuf;
    }

    if (op == INST_SPECIAL) {
        instOp = specialFuncts[FUNCT(inst)];
    } else if (op == INST_REGIMM) {
        instOp = regimmRt[RT(inst)];
    } else if (op == INST_COP0 && disasmCop0(inst)) {
        return instBuf;
    } else if (op == INST_COP1 && disasmCop1(inst, pc)) {
        return instBuf;
    }
    switch (instOp.format) {
        case INST_FMT_T_S_SIMM:
            snprintf(instBuf, sizeof(instBuf), "%-5s %s, %s, %d", instOp.mnemonic, gprNames[RT(inst)],
                     gprNames[RS(inst)], IMM(inst));
            break;
        case INST_FMT_T_S_HEX:
            snprintf(instBuf, sizeof(instBuf), "%-5s %s, %s, 0x%04x", instOp.mnemonic, gprNames[RT(inst)],
                     gprNames[RS(inst)], (u16)IMM(inst));
            break;
        case INST_FMT_T_HEX:
            snprintf(instBuf, sizeof(instBuf), "%-5s %s, 0x%04x", instOp.mnemonic, gprNames[RT(inst)], (u16)IMM(inst));
            break;
        case INST_FMT_S_HEX:
            snprintf(instBuf, sizeof(instBuf), "%-5s %s, 0x%04x", instOp.mnemonic, gprNames[RS(inst)], (u16)IMM(inst));
            break;
        case INST_FMT_T_OFF_S:
            snprintf(instBuf, sizeof(instBuf), "%-5s %s, %d(%s)", instOp.mnemonic, gprNames[RT(inst)], IMM(inst),
                     gprNames[RS(inst)]);
            break;
        case INST_FMT_TARGET:
            snprintf(instBuf, sizeof(instBuf), "%-5s 0x%08lx", instOp.mnemonic, TARGET(inst, pc));
            break;
        case INST_FMT_NONE:
            if (FUNCT(inst) == 15) { // sync
                snprintf(instBuf, sizeof(instBuf), "%s", instOp.mnemonic);
                break;
            }
            // break and syscall
            snprintf(instBuf, sizeof(instBuf), "%s 0x%lx", instOp.mnemonic, CODE(inst));
            break;
        case INST_FMT_D_S_T:
            snprintf(instBuf, sizeof(instBuf), "%-5s %s, %s, %s", instOp.mnemonic, gprNames[RD(inst)],
                     gprNames[RS(inst)], gprNames[RT(inst)]);
            break;
        case INST_FMT_D_T_SA:
            snprintf(instBuf, sizeof(instBuf), "%-5s %s, %s, 0x%lx", instOp.mnemonic, gprNames[RD(inst)],
                     gprNames[RT(inst)], SA(inst));
            break;
        case INST_FMT_D_T_S:
            snprintf(instBuf, sizeof(instBuf), "%-5s %s, %s, %s", instOp.mnemonic, gprNames[RD(inst)],
                     gprNames[RT(inst)], gprNames[RS(inst)]);
            break;
        case INST_FMT_S_T:
            snprintf(instBuf, sizeof(instBuf), "%-5s %s, %s", instOp.mnemonic, gprNames[RS(inst)], gprNames[RT(inst)]);
            break;
        case INST_FMT_D_S:
            if (RD(inst) == 31) {
                snprintf(instBuf, sizeof(instBuf), "%-5s %s", instOp.mnemonic, gprNames[RS(inst)]);
            } else {
                snprintf(instBuf, sizeof(instBuf), "%-5s %s, %s", instOp.mnemonic, gprNames[RD(inst)],
                         gprNames[RS(inst)]);
            }
            break;
        case INST_FMT_D: snprintf(instBuf, sizeof(instBuf), "%-5s %s", instOp.mnemonic, gprNames[RD(inst)]); break;
        case INST_FMT_S: snprintf(instBuf, sizeof(instBuf), "%-5s %s", instOp.mnemonic, gprNames[RS(inst)]); break;
        case INST_FMT_S_T_BRANCH:
            snprintf(instBuf, sizeof(instBuf), "%-5s %s, %s, 0x%08lx", instOp.mnemonic, gprNames[RS(inst)],
                     gprNames[RT(inst)], BRANCH_TARGET(inst, pc));
            break;
        case INST_FMT_FT_OFF_S:
            snprintf(instBuf, sizeof(instBuf), "%-5s %s, %d(%s)", instOp.mnemonic, fprNames[FT(inst)], IMM(inst),
                     gprNames[RS(inst)]);
            break;
        case INST_FMT_CACHE:
            snprintf(instBuf, sizeof(instBuf), "%-5s 0x%02lx, %d(%s)", instOp.mnemonic, RT(inst), IMM(inst),
                     gprNames[RS(inst)]);
            break;
        case INST_FMT_S_BRANCH:
            snprintf(instBuf, sizeof(instBuf), "%-5s %s, 0x%08lx", instOp.mnemonic, gprNames[RS(inst)],
                     BRANCH_TARGET(inst, pc));
            break;
        case INST_FMT_INVALID:
        default: snprintf(instBuf, sizeof(instBuf), ".word 0x%08lx", inst); break;
    }

    return instBuf;
}
