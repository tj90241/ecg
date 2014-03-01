//
// x86_64.c: x86_64 generator backend.
//
// ECG: Emulator Code Generator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "ecg/backend.h"
#include "backends/x86_64.h"

#ifndef _MSC_VER
#define NUM_CALLEE_SAVE_REGISTERS 6
#else
#define NUM_CALLEE_SAVE_REGISTERS 4
#endif

const unsigned num_target_regs = NUM_TARGET_REGS - 1;
const unsigned num_target_vregs = NUM_TARGET_VREGS;
const unsigned num_callee_save_regs = NUM_CALLEE_SAVE_REGISTERS;
const unsigned num_caller_save_regs = (NUM_TARGET_REGS - 1) -
  NUM_CALLEE_SAVE_REGISTERS;

#ifndef NDEBUG
const char *target_reg_names[NUM_TARGET_REGS] = {
  "%rax", "%rcx", "%rdx", "%rbx", "%rsp", "%rbp", "%rsi", "%rdi",
  "%r8",  "%r9",  "%r10", "%r11", "%r12", "%r13", "%r14", "%r15",
};

const char *target_vreg_names[NUM_TARGET_VREGS] = {
  "%xmm0", "%xmm1", "%xmm2",  "%xmm3",  "%xmm4",  "%xmm5",  "%xmm6",  "%xmm7", 
  "%xmm8", "%xmm9", "%xmm10", "%xmm11", "%xmm12", "%xmm13", "%xmm14", "%xmm15", 
};
#endif

static void ecg_gen_rex(struct ecg_ctx *ctx,
  unsigned r, unsigned rm, unsigned w) {
  int rm_hi = rm; // & 0x8;
  int r_hi = r & 0x8;
  int rex;

  assert(rm < 0x10);
  assert(r < 0x10);

  if ((rex = (r_hi >> 1) | (rm_hi >> 3) | (w << 3)) != 0)
    ecg_ctx_append8(ctx, 0x40 | rex);
}

static void ecg_gen_rex_1op(struct ecg_ctx *ctx,
  unsigned op, unsigned r, unsigned rm, unsigned w) {
  ecg_gen_rex(ctx, r, rm, w);
  ecg_ctx_append8(ctx, op);
}

static void ecg_gen_ldst_displacement(struct ecg_ctx *ctx, int32_t off) {
  if (off == 0)
    return;

  if ((int8_t) off == off) {
    ctx->end[-1] |= 0x40;
    ecg_ctx_append8(ctx, off);
  }

  else {
    ctx->end[-1] |= 0x80;
    ecg_ctx_append32(ctx, off);
  }
}

static void ecg_gen_modrm(struct ecg_ctx *ctx,
  unsigned mod, unsigned r, unsigned rm) {
  rm &= 0x7;
  r &= 0x7;

  ecg_ctx_append8(ctx, (mod << 6) | (r << 3) | rm);
}

static void ecg_gen_modrm_1op(struct ecg_ctx *ctx, unsigned op,
  unsigned mod, unsigned r, unsigned rm, unsigned w) {
  ecg_gen_rex_1op(ctx, op, r, rm, w);
  ecg_gen_modrm(ctx, mod, r, rm);
}

ecg_align(static const uint8_t ecg_to_x86_64_lut[], 64) = {
  X86_64_OPCODE_ADC,
  X86_64_OPCODE_ADD,
  X86_64_OPCODE_AND,
  X86_64_OPCODE_ASR,
  0, // X86_64_OPCODE_BSWAP16,
  0, // X86_64_OPCODE_BSWAP32,
  0, // X86_64_OPCODE_BSWAP64,
  X86_64_OPCODE_CALL,
  X86_64_OPCODE_CMP,
  X86_64_OPCODE_DIVS,
  X86_64_OPCODE_DIVU,
  X86_64_OPCODE_FUNC,
  X86_64_OPCODE_JMP,
  X86_64_OPCODE_LBS,
  X86_64_OPCODE_LBU,
  X86_64_OPCODE_LD,
  X86_64_OPCODE_LHS,
  X86_64_OPCODE_LHU,
  X86_64_OPCODE_LSL,
  X86_64_OPCODE_LSR,
  X86_64_OPCODE_LWS,
  X86_64_OPCODE_LWU,
  X86_64_OPCODE_MOV,
  X86_64_OPCODE_MULS,
  X86_64_OPCODE_MULU,
  X86_64_OPCODE_NEG,
  X86_64_OPCODE_NOT,
  X86_64_OPCODE_ORR,
  X86_64_OPCODE_POP,
  X86_64_OPCODE_PUSH,
  0, // X86_64_OPCODE_RET,
  X86_64_OPCODE_SB,
  X86_64_OPCODE_SBC,
  X86_64_OPCODE_SD,
  X86_64_OPCODE_SH,
  X86_64_OPCODE_SUB,
  X86_64_OPCODE_SW,
  X86_64_OPCODE_TST,
  X86_64_OPCODE_XOR,
};

static unsigned ecg_to_x86_64(enum ecg_opcode eop) {
  return ecg_to_x86_64_lut[eop];
}

// Generates register/immediate operations.
void ecg_gen_alu_imm(struct ecg_ctx *ctx, enum ecg_opcode eop,
  unsigned rd, int32_t imm, unsigned w) {
  unsigned op = ecg_to_x86_64(eop);
  int opc, rm = (op - X86_64_OPCODE_ADD) & 0x7;

  // Shifts and 8-bit immediate operations.
  if ((int8_t) imm == imm && op != X86_64_OPCODE_TST) {
    opc = 0x83;

    if (op < 0x4) {
      assert((w && imm < 64) || (!w && imm < 32));

      opc = 0xC0;
      rm = 0x4 | (op & 3);
    }

    ecg_gen_modrm_1op(ctx, opc, 0x3, rm, rd, w);
    ecg_ctx_append8(ctx, imm);
    return;
  }

  // 32-bit immediate operations.
  opc = 0x81;

  if (op == X86_64_OPCODE_TST) {
    opc = 0xF7;
    rm = 0x0;
  }

  ecg_gen_modrm_1op(ctx, opc, 0x3, rm, rd, w);
  ecg_ctx_append32(ctx, imm);
}

ecg_align(static const uint8_t ecg_gen_alu_reg_lut[], 16) = {
  0xD3, // X86_64_OPCODE_LSL
  0xD3, // X86_64_OPCODE_LSR
  0xD3, // PADDING
  0xD3, // X86_64_OPCODE_ASR
  0x85, // X86_64_OPCODE_TST
  0x03, // X86_64_OPCODE_ADD
  0x0B, // X86_64_OPCODE_ORR
  0x13, // X86_64_OPCODE_ADC
  0x1B, // X86_64_OPCODE_SBB
  0x23, // X86_64_OPCODE_AND
  0x2B, // X86_64_OPCODE_SUB
  0x33, // X86_64_OPCODE_XOR
  0x3B, // X86_64_OPCODE_CMP
  0x8B, // X86_64_OPCODE_MOV
};

// Generates register/register operations.
void ecg_gen_alu_reg(struct ecg_ctx *ctx,
  enum ecg_opcode eop, unsigned rd, unsigned rn, unsigned w) {
  unsigned op = ecg_to_x86_64(eop);

  // Oddly enough, not/neg are encoded like *mul/*div.
  if (eop == ECG_OPCODE_NEG || eop == ECG_OPCODE_NOT) {
    ecg_gen_muldiv(ctx, eop, 0x0, 0x0, rd, w);
    return;
  }

  if (op < 0x5) {
    int temp = rd;
    rd = rn;
    rn = temp;

    // Shifts:
    if (op < 4) {
      assert(rn == TARGET_REG_RCX);
      rd = 0x4 | (op & 3);
    }
  }

  ecg_gen_modrm_1op(ctx, ecg_gen_alu_reg_lut[op], 0x3, rd, rn, w);
}

ecg_align(static uint8_t ecg_to_x86_64_brcond_lut[], 16) = {
  X86_64_COND_AL,
  X86_64_COND_EQ,
  X86_64_COND_NE,
  X86_64_COND_LT,
  X86_64_COND_LE,
  X86_64_COND_GT,
  X86_64_COND_GE,
  X86_64_COND_LTU,
  X86_64_COND_LEU,
  X86_64_COND_GTU,
  X86_64_COND_GEU,
};

// Generates call and jump operations.
void ecg_gen_branch(struct ecg_ctx *ctx,
  enum ecg_opcode eop, enum ecg_brcond brcond, uintptr_t ptr) {
  unsigned cond = ecg_to_x86_64_brcond_lut[brcond];
  unsigned op = ecg_to_x86_64(eop);
  intptr_t d;

  // Relative 8-bit addressing.
  d = ptr - (intptr_t) ctx->end - 2;

  if (op != X86_64_OPCODE_CALL && (d == (int8_t) d)) {
    ecg_ctx_append16(ctx, (0x70 + cond) | (d << 8));
    return;
  }

  // Relative 32-bit addressing.
  d = ptr - (intptr_t) ctx->end - 5;

  if (d == (int32_t) d) {
    if (op == X86_64_OPCODE_CALL)
      ecg_ctx_append8(ctx, 0xE8);
    else
      ecg_ctx_append16(ctx, ((0x80 + cond) << 8) | 0x0F);

    ecg_ctx_append32(ctx, d);
    return;
  }

  assert(0 && "x86_64 backend doesn't support far branches yet.");
}

// Generates a byte order swap operation.
void ecg_gen_bswap(struct ecg_ctx *ctx,
  enum ecg_opcode eop, unsigned rd, unsigned w) {
  if (eop == ECG_OPCODE_BSWAP16) {
    ecg_ctx_append8(ctx, 0x66); // rorh $8, reg
    ecg_gen_modrm_1op(ctx, 0xC1, 0x3, 0x1, rd, 0x0);
    ecg_ctx_append8(ctx, 0x08);
  }

  else
    ecg_gen_modrm_1op(ctx, 0x0F, 0x3, 0x1, rd, w);
}

// Generates a far call operation.
void ecg_gen_func(struct ecg_ctx *ctx, unsigned rd) {
  ecg_gen_modrm_1op(ctx, 0xFF, 0x3, 0x2, rd, 0x0);
}

// Generates a 32-bit immediate move operation.
void ecg_gen_movi32(struct ecg_ctx *ctx, unsigned rd, uint32_t imm) {
  if (rd >= 0x8)
    ecg_ctx_append8(ctx, 0x41);

  ecg_ctx_append8(ctx, 0xB8 + (rd & 0x7));
  ecg_ctx_append32(ctx, imm);
}

// Generates a 64-bit immediate move operation.
void ecg_gen_movi64(struct ecg_ctx *ctx, unsigned rd, uint64_t imm) {
  int64_t d = imm - ((uintptr_t) (ctx->end) + 7);
  int rd_hi = rd & 0x8;

  // 32-bit mov will zero-extend.
  // Can we just use that instead?
  if (imm == (uint32_t) imm) {
    ecg_gen_movi32(ctx, rd, imm);
    return;
  }

  rd &= 0x7;

  // Can we sign-extend a word?
  if ((int64_t) imm == (int32_t) imm) {
    ecg_ctx_append16(ctx, 0xC748 | (rd_hi >> 3));
    ecg_gen_modrm(ctx, 0x3, 0x0, rd);
    ecg_ctx_append32(ctx, imm);
    return;
  }

  // 7-byte RIP relative?
  if (d == (int32_t) d) {
    ecg_ctx_append16(ctx, 0x8D48 | (rd_hi >> 1));
    ecg_gen_modrm(ctx, 0x0, rd, 0x5);
    ecg_ctx_append32(ctx, d);
    return;
  }

  ecg_ctx_append16(ctx, 0x48 | ((0xB8 + rd) << 8) | (rd_hi >> 3));
  ecg_ctx_append64(ctx, imm);
}

// Generates a load operation.
void ecg_gen_ld(struct ecg_ctx *ctx, enum ecg_opcode eop,
  unsigned rd, unsigned rn, int32_t off, unsigned w) {
  unsigned op = ecg_to_x86_64(eop);
  int opc = 0xB6 | (op & 0x9);

  // Don't use SIB, check nonsense loads.
  assert((rn & 7) != TARGET_REG_RSP);

  if (op == X86_64_OPCODE_LD)
    assert(w);

  ecg_gen_rex(ctx, rd, rn, w);

  if ((op >> 1 & 0x1) == 0) {
    opc = op == X86_64_OPCODE_LWS ? 0x63 : 0x8B;
    ecg_ctx_append8(ctx, opc);
  }

  else
    ecg_ctx_append16(ctx, (opc << 8) | 0x0F);

  ecg_gen_modrm(ctx, 0x0, rd, rn);
  ecg_gen_ldst_displacement(ctx, off);
}

// Generates multiply and divide operations.
void ecg_gen_muldiv(struct ecg_ctx *ctx, enum ecg_opcode eop,
  unsigned r1, unsigned r2, unsigned r3, unsigned w) {
  unsigned op = ecg_to_x86_64(eop);

  if (op != X86_64_OPCODE_NOT && op != X86_64_OPCODE_NEG) {
    assert(r1 == TARGET_REG_RDX);
    assert(r2 == TARGET_REG_RAX);
  }

  ecg_gen_modrm_1op(ctx, 0xF7, 0x3, op & 0x7, r3, w);
}

// Generates stack push and pop operations.
void ecg_gen_pushpop(struct ecg_ctx *ctx, enum ecg_opcode eop, unsigned rd) {
  unsigned op = ecg_to_x86_64(eop);
  ecg_gen_rex_1op(ctx, op + (rd & 0x7), 0x0, rd, 0x0);
}


// Generates a return from translated block.
void ecg_gen_ret(struct ecg_ctx *ctx) {
  ecg_ctx_append8(ctx, 0xC3);
}

// Generates a store operation.
void ecg_gen_st(struct ecg_ctx *ctx, enum ecg_opcode eop,
  unsigned rn, unsigned rm, int32_t off) {
  unsigned op = ecg_to_x86_64(eop);
  int opc = 0x89;

  // Don't use SIB.
  assert((rn & 7) != TARGET_REG_RSP);

  // movb opcode adjustment.
  if (op == X86_64_OPCODE_SB)
    opc -= 0x1;

  if (op == X86_64_OPCODE_SH)
    ecg_ctx_append8(ctx, 0x66);

  ecg_gen_modrm_1op(ctx, opc, 0x0, rn, rm, 0x0);
  ecg_gen_ldst_displacement(ctx, off);
}

// LUT returns the lowest bit set in a 8-bit value.
ecg_align(static const uint8_t ecg_regalloc_bitmap[256], 64) = {
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x03,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x04,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x03,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x05,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x03,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x04,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x03,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x06,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x03,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x04,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x03,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x05,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x03,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x04,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x03,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x07,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x03,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x04,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x03,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x05,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x03,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x04,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x03,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x06,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x03,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x04,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x03,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x05,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x03,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x04,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x03,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x08,
};

// We allocate registers in different orderings
// depending on the host calling convention.
ecg_align(static const uint8_t reg_alloc_map[16], 16) = {
#ifndef _MSC_VER
  TARGET_REG_RDI, TARGET_REG_RSI, TARGET_REG_RDX, TARGET_REG_RCX,
  TARGET_REG_R8,  TARGET_REG_R9,  TARGET_REG_RAX, TARGET_REG_RBX,
  TARGET_REG_RBP, TARGET_REG_R10, TARGET_REG_R11, TARGET_REG_R12,
  TARGET_REG_R13, TARGET_REG_R14, TARGET_REG_R15, TARGET_REG_RSP,
#else
  TARGET_REG_RCX, TARGET_REG_RDX, TARGET_REG_R8,  TARGET_REG_R9,
  TARGET_REG_RAX, TARGET_REG_RBX, TARGET_REG_RDI, TARGET_REG_RSI,
  TARGET_REG_RBP, TARGET_REG_R10, TARGET_REG_R11, TARGET_REG_R12,
  TARGET_REG_R13, TARGET_REG_R14, TARGET_REG_R15, TARGET_REG_RSP,
#endif
};

ecg_align(static const uint8_t reg_free_map[16], 16) = {
#ifndef _MSC_VER
  6  /*REG_RAX*/, 3  /*REG_RCX*/, 2  /*REG_RDX*/, 7  /*REG_RBX*/,
  15 /*REG_RSP*/, 8  /*REG_RBP*/, 1  /*REG_RSI*/, 0  /*REG_RDI*/,
  4  /*REG_R8*/,  5  /*REG_R9*/,  9  /*REG_R10*/, 10 /*REG_R11*/,
  11 /*REG_R12*/, 12 /*REG_R13*/, 13 /*REG_R14*/, 14 /*REG_R15*/,
#else
  4  /*REG_RAX*/, 0  /*REG_RCX*/, 1  /*REG_RDX*/, 5  /*REG_RBX*/,
  15 /*REG_RSP*/, 8  /*REG_RBP*/, 7  /*REG_RSI*/, 6  /*REG_RDI*/,
  2  /*REG_R8*/,  3  /*REG_R9*/,  9  /*REG_R10*/, 10 /*REG_R11*/,
  11 /*REG_R12*/, 12 /*REG_R13*/, 13 /*REG_R14*/, 14 /*REG_R15*/,
#endif
};

ecg_align(static const uint8_t reg_type_map[16], 16) = {
#ifndef _MSC_VER
  2  /*REG_RAX*/, 0  /*REG_RCX*/, 0  /*REG_RDX*/, 1  /*REG_RBX*/,
  ~0 /*REG_RSP*/, 1  /*REG_RBP*/, 0  /*REG_RSI*/, 0  /*REG_RDI*/,
  0  /*REG_R8*/,  0  /*REG_R9*/,  1  /*REG_R10*/, 1  /*REG_R11*/,
  1  /*REG_R12*/, 1  /*REG_R13*/, 1  /*REG_R14*/, 1  /*REG_R15*/,
#else
  2  /*REG_RAX*/, 0  /*REG_RCX*/, 0  /*REG_RDX*/, 0  /*REG_RBX*/,
  ~0 /*REG_RSP*/, 1  /*REG_RBP*/, 1  /*REG_RSI*/, 1  /*REG_RDI*/,
  0  /*REG_R8*/,  0  /*REG_R9*/,  1  /*REG_R10*/, 1  /*REG_R11*/,
  1  /*REG_R12*/, 1  /*REG_R13*/, 1  /*REG_R14*/, 1  /*REG_R15*/,
#endif
};

// Allocates a free register and returns it.
unsigned ecg_alloc_reg(struct ecg_ctx *ctx, unsigned *reg) {
  unsigned r, rnum;

  assert(ctx->used_regs < 0xFFFF);
  if ((r = ecg_regalloc_bitmap[ctx->used_regs & 0xFF]) == 0x8)
    r = 8 + ecg_regalloc_bitmap[ctx->used_regs >> 8];

  ctx->used_regs |= 1 << r;
  rnum = reg_alloc_map[r];
  *reg = rnum;

  return reg_type_map[rnum];
}

// Allocates a free caller-save register and returns it.
void ecg_alloc_cs_reg(struct ecg_ctx *ctx, unsigned *reg) {
  unsigned mask = (1 << (NUM_CALLEE_SAVE_REGISTERS + 1)) - 1;
  uint32_t used_regs = ctx->used_regs | mask;
  unsigned r;

  assert(used_regs < 0xFFFF);

  if ((r = ecg_regalloc_bitmap[used_regs & 0xFF]) == 0x8)
    r = 8 + ecg_regalloc_bitmap[used_regs >> 8];

  ctx->used_regs |= 1 << r;
  *reg = reg_alloc_map[r];
}

// Returns the register used for return values.
unsigned ecg_get_rv_reg(void) {
  return TARGET_REG_RAX;
}

// Marks a previously allocated register as free.
void ecg_free_reg(struct ecg_ctx *ctx, unsigned reg) {
  reg = reg_free_map[reg];
  assert(ctx->used_regs & (1 << reg));
  ctx->used_regs &= ~(1 << reg);
}

