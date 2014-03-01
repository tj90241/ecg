//
// x86_64.h: x86_64 generator backend.
//
// ECG: Emulator Code Generator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __ecg_x86_64_h__
#define __ecg_x86_64_h__
#include "ecg/common.h"
#include "ecg/backend.h"

#define X86_64_OPCODE_LSL  0x00
#define X86_64_OPCODE_LSR  0x01
#define X86_64_OPCODE_ASR  0x03
#define X86_64_OPCODE_TST  0x04
#define X86_64_OPCODE_ADD  0x05
#define X86_64_OPCODE_ORR  0x06
#define X86_64_OPCODE_ADC  0x07
#define X86_64_OPCODE_SBC  0x08
#define X86_64_OPCODE_AND  0x09
#define X86_64_OPCODE_SUB  0x0A
#define X86_64_OPCODE_XOR  0x0B
#define X86_64_OPCODE_CMP  0x0C
#define X86_64_OPCODE_MOV  0x0D
#define X86_64_OPCODE_NOT  0x12
#define X86_64_OPCODE_NEG  0x13
#define X86_64_OPCODE_MULU 0x14
#define X86_64_OPCODE_MULS 0x15
#define X86_64_OPCODE_DIVU 0x16
#define X86_64_OPCODE_DIVS 0x17
#define X86_64_OPCODE_LWU  0x20
#define X86_64_OPCODE_LD   0x21
#define X86_64_OPCODE_LBU  0x22
#define X86_64_OPCODE_LHU  0x23
#define X86_64_OPCODE_LWS  0x28
#define X86_64_OPCODE_LBS  0x2A
#define X86_64_OPCODE_LHS  0x2B
#define X86_64_OPCODE_SB   0x30
#define X86_64_OPCODE_SH   0x38
#define X86_64_OPCODE_SW   0x39
#define X86_64_OPCODE_SD   0x3A
#define X86_64_OPCODE_CALL 0x40
#define X86_64_OPCODE_JMP  0x41
#define X86_64_OPCODE_FUNC 0x42
#define X86_64_OPCODE_PUSH 0x50
#define X86_64_OPCODE_POP  0x58

#define X86_64_COND_AL     0x0
#define X86_64_COND_EQ     0x4
#define X86_64_COND_NE     0x5
#define X86_64_COND_LT     0xC
#define X86_64_COND_LE     0xE
#define X86_64_COND_GT     0xF
#define X86_64_COND_GE     0xD
#define X86_64_COND_LTU    0x2
#define X86_64_COND_LEU    0x6
#define X86_64_COND_GTU    0x7
#define X86_64_COND_GEU    0x3

enum target_regs {
  TARGET_REG_RAX,
  TARGET_REG_RCX,
  TARGET_REG_RDX,
  TARGET_REG_RBX,
  TARGET_REG_RSP,
  TARGET_REG_RBP,
  TARGET_REG_RSI,
  TARGET_REG_RDI,
  TARGET_REG_R8,
  TARGET_REG_R9,
  TARGET_REG_R10,
  TARGET_REG_R11,
  TARGET_REG_R12,
  TARGET_REG_R13,
  TARGET_REG_R14,
  TARGET_REG_R15,
  NUM_TARGET_REGS,
};

enum target_vregs {
  TARGET_REG_XMM0,
  TARGET_REG_XMM1,
  TARGET_REG_XMM2,
  TARGET_REG_XMM3,
  TARGET_REG_XMM4,
  TARGET_REG_XMM5,
  TARGET_REG_XMM6,
  TARGET_REG_XMM7,
  TARGET_REG_XMM8,
  TARGET_REG_XMM9,
  TARGET_REG_XMM10,
  TARGET_REG_XMM11,
  TARGET_REG_XMM12,
  TARGET_REG_XMM13,
  TARGET_REG_XMM14,
  TARGET_REG_XMM15,
  NUM_TARGET_VREGS,
};

#endif

