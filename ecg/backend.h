//
// backend.h: Common generator/backend code.
//
// ECG: Emulator Code Generator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __ecg_backend_h__
#define __ecg_backend_h__
#include "ecg/common.h"

#define ECG_32BIT 0
#define ECG_64BIT 1

#if UINTPTR_MAX == 0xFFFFFFFF
#define ECG_PTR 0
#else
#define ECG_PTR 1
#endif

extern const unsigned num_target_regs;
extern const unsigned num_target_vregs;
extern const unsigned num_callee_save_regs;
extern const unsigned num_caller_save_regs;

#ifndef NDEBUG
extern const char *target_reg_names[];
extern const char *target_vreg_names[];
#endif

enum ecg_opcode {
  ECG_OPCODE_ADC,
  ECG_OPCODE_ADD,
  ECG_OPCODE_AND,
  ECG_OPCODE_ASR,
  ECG_OPCODE_BSWAP16,
  ECG_OPCODE_BSWAP32,
  ECG_OPCODE_BSWAP64,
  ECG_OPCODE_CALL,
  ECG_OPCODE_CMP,
  ECG_OPCODE_DIVS,
  ECG_OPCODE_DIVU,
  ECG_OPCODE_FUNC,
  ECG_OPCODE_JMP,
  ECG_OPCODE_LBS,
  ECG_OPCODE_LBU,
  ECG_OPCODE_LD,
  ECG_OPCODE_LHS,
  ECG_OPCODE_LHU,
  ECG_OPCODE_LSL,
  ECG_OPCODE_LSR,
  ECG_OPCODE_LWS,
  ECG_OPCODE_LWU,
  ECG_OPCODE_MOV,
  ECG_OPCODE_MULS,
  ECG_OPCODE_MULU,
  ECG_OPCODE_NEG,
  ECG_OPCODE_NOT,
  ECG_OPCODE_ORR,
  ECG_OPCODE_POP,
  ECG_OPCODE_PUSH,
  ECG_OPCODE_RET,
  ECG_OPCODE_SB,
  ECG_OPCODE_SBC,
  ECG_OPCODE_SD,
  ECG_OPCODE_SH,
  ECG_OPCODE_SUB,
  ECG_OPCODE_SW,
  ECG_OPCODE_TST,
  ECG_OPCODE_XOR,
  NUM_ECG_OPCODES,
};

enum ecg_brcond {
  ECG_COND_AL,
  ECG_COND_EQ,
  ECG_COND_NE,
  ECG_COND_LT,
  ECG_COND_LE,
  ECG_COND_GT,
  ECG_COND_GE,
  ECG_COND_LTU,
  ECG_COND_LEU,
  ECG_COND_GTU,
  ECG_COND_GEU,
};

// Functions to append code to the end of a context block.
// Powers of two are used to enforce efficient code generation.
static inline void ecg_ctx_append8(struct ecg_ctx *ctx, uint8_t d) {
  assert((ctx->buffer + sizeof(d)) <= (ctx->end + ctx->size));
  *ctx->end++ = d;
}

static inline void ecg_ctx_append16(struct ecg_ctx *ctx, uint16_t d) {
  assert((ctx->buffer + sizeof(d)) <= (ctx->end + ctx->size));
  memcpy(ctx->end, &d, sizeof(d));
  ctx->end += sizeof(d);
}

static inline void ecg_ctx_append32(struct ecg_ctx *ctx, uint32_t d) {
  assert((ctx->buffer + sizeof(d)) <= (ctx->end + ctx->size));
  memcpy(ctx->end, &d, sizeof(d));
  ctx->end += sizeof(d);
}

static inline void ecg_ctx_append64(struct ecg_ctx *ctx, uint64_t d) {
  assert((ctx->buffer + sizeof(d)) <= (ctx->end + ctx->size));
  memcpy(ctx->end, &d, sizeof(d));
  ctx->end += sizeof(d);
}

// Initializes a context for code generation.
static inline void ecg_ctx_init(struct ecg_ctx *ctx,
  uint8_t *buffer, size_t size) {
  ctx->buffer = ctx->end = buffer;
  ctx->size = size;

  ctx->used_regs = 0;
}

size_t ecg_alloc_buf(uint8_t **buf, size_t size);
int ecg_free_buf(uint8_t *buffer, size_t size);

// Immediate move operations.
void ecg_gen_movi32(struct ecg_ctx *ctx, unsigned rd, uint32_t imm);
void ecg_gen_movi64(struct ecg_ctx *ctx, unsigned rd, uint64_t imm);

static inline void ecg_gen_movptr(struct ecg_ctx *ctx, unsigned rd, void *ptr) {
#if ECG_PTR == 0
  ecg_gen_movi32(ctx, rd, (uintptr_t) ptr);
#else
  ecg_gen_movi64(ctx, rd, (uintptr_t) ptr);
#endif
}

// Instruction generators:
//  W == 0 => 32-bit
//  W == 1 => 64-bit

// ECG_OPCODE_{ADC,ADD,AND,ASR,CMP,LSL,LSR,ORR,SBC,SUB,TST,XOR}
//   CMP/TST: flags = rd op imm
//   Others: rd = rd op imm
void ecg_gen_alu_imm(struct ecg_ctx *ctx,  enum ecg_opcode eop,
  unsigned rd, int32_t imm, unsigned w);

// ECG_OPCODE_{ADC,ADD,AND,ASR,CMP,LSL,LSR,NEG,NOT,ORR,SBC,SUB,TST,XOR}
//   CMP/TST: flags = rd op rn
//   NEG/NOT: rd = op rd
//   Others: rd = rd op rn
void ecg_gen_alu_reg(struct ecg_ctx *ctx, enum ecg_opcode eop,
  unsigned rd, unsigned rn, unsigned w);

// ECG_OPCODE_{CALL,JMP}
//  CALL: pc <= imm, PUSH pc
//  JMP.cond: cond ? pc <= imm : next_pc;
void ecg_gen_branch(struct ecg_ctx *ctx, enum ecg_opcode eop,
  unsigned cond, uintptr_t ptr);

// ECG_OPCODE_BSWAP_{16,32,64}
//  rd = bswap.size rd
void ecg_gen_bswap(struct ecg_ctx *ctx, enum ecg_opcode eop,
  unsigned rd, unsigned w);

// ECG_OPCODE_FUNC
//  rd = pc <= rd, PUSH pc
void ecg_gen_func(struct ecg_ctx *ctx, unsigned rd);

// ECG_OPCODE_{LBU,LBS,LHU,LHS,LWU,LWS,LD}
//  rd = mem[rn + off ... rn + off + op_size]
void ecg_gen_ld(struct ecg_ctx *ctx, enum ecg_opcode eop,
  unsigned rd, unsigned rn, int32_t off, unsigned w);

// ECG_OPCODE_{DIVS,DIVU,MULS,MULU}
//  DIVS/DIVU: r3 = r1:r2 / r3
//  MULS/MULU: r1:r2 = r2 * r3
void ecg_gen_muldiv(struct ecg_ctx *ctx, enum ecg_opcode,
  unsigned r1, unsigned r2, unsigned r3, unsigned w);

// ECG_OPCODE_{POP,PUSH}
//  PUSH: PUSH rd, ADD sp, reg_size
//  POP: POP rd, SUB sp, reg_size
void ecg_gen_pushpop(struct ecg_ctx *ctx, enum ecg_opcode,
  unsigned rd);

// ECG_OPCODE_{SB,SH,SW,SD}
//  mem[rn + off ... rn + off + op_size] = rd
void ecg_gen_st(struct ecg_ctx *ctx, enum ecg_opcode eop,
  unsigned rn, unsigned rm, int32_t off);

// ECG_OPCODE_{RET}
//  POP pc
void ecg_gen_ret(struct ecg_ctx *ctx);

// Allocates a free register.
//
// Registers are allocated in the order that they should be
// used to pass arguments according to the target's calling
// convention.
//
// The function returns a value to indicate register type:
//  * 2: Indicates a return value register.
//  * 1: Indicates a caller save register.
//  * O: Indicates a callee save register.
unsigned ecg_alloc_reg(struct ecg_ctx *ctx, unsigned *reg);

// Allocates a free caller save register
void ecg_alloc_cs_reg(struct ecg_ctx *ctx, unsigned *reg);

// Marks a previously allocated register as free.
void ecg_free_reg(struct ecg_ctx *ctx, unsigned reg);

// Returns the register used for return values.
unsigned ecg_get_rv_reg(void);

#endif

