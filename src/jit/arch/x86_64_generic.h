/*
** x86_64_generic.h - Remi Bauzac (rbauzac@free.fr)
** Lua JiT implementation for x86_64 processors (Intel compat) with common ABI
** See Copyright Notice in lua.h
*/

#ifndef __x86_64_generic_h__
#define __x86_64_generic_h__

#define OP_SZ_MAX 200
#define _AU_ __attribute__((unused))

/* Jumps constants */
#define X86_JB  0x72
#define X86_JAE 0x73
#define X86_JE  0x74
#define X86_JNE 0x75
#define X86_JBE 0x76
#define X86_JA  0x77
#define X86_NJ  0xeb /* near jump */
#define X86_LJ  0xe9 /* long jump */
/*
 * Registers :
 *  %rbp : frame pointer
 *  %rax : scratch register
 *  %r10 : scratch register
 *  %rbx : L (Lua state) callee saved register
 *  %r12 : &savedpc callee saved register
 *  %r13 : ci (Call Info) callee saved register
 *
 * ABI: %rdi, %rsi, %rdx, %rcx, %r8, %r9
 *
 * Stack:
 *  -8(%rbp): saved %rbx callee saved register
 *  -16(%rbp): saved %r12 callee saved register
 *  -24(%rbp): saved %r13 callee saved register
 *
 * Registers on call:
 *  %rdi: Lua state
 *  %rsi: CallInfo struct
 *  %rdx: Closure struct
 *  %rcx: offset for prologue jump
 */

#define NOP  APPEND1(0x90);
#define NOP2 APPEND2(0x66, 0x90);
#define NOP3 APPEND3(0x0f, 0x1f, 0x00);
#define NOP4 APPEND4(0x0f, 0x1f, 0x40, 0x00);
#define NOP5 APPEND4(0x0f, 0x1f, 0x44, 0x00); APPEND1(0x00);
#define NOP6 APPEND4(0x66, 0x0f, 0x1f, 0x44); APPEND2(0x00, 0x00);
#define NOP7 APPEND4(0x0f, 0x1f, 0x80, 0x00); APPEND3(0x00, 0x00, 0x00);
#define NOP8 APPEND4(0x0f, 0x1f, 0x84, 0x00); APPEND4(0x00, 0x00, 0x00, 0x00);
#define NOP9 APPEND4(0x66, 0x0f, 0x1f, 0x84); APPEND4(0x00, 0x00, 0x00, 0x00); APPEND1(0x00);
#define NOP10 NOP6; NOP4;
#define NOP11 APPEND4(0x66, 0x66, 0x2e, 0x0f); APPEND4(0x1f, 0x84, 0x00 0x00); APPEND3(0x00, 0x00, 0x00);
#define NOP12 NOP6; NOP6;
#define NOP13 NOP6; NOP7;
#define NOP14 NOP7; NOP7;
#define NOP15 NOP7; NOP8;

#define RABC_RDI(arg) \
  /* mov offset8(%r13), %rdi*/ \
  APPEND4(0x49, 0x8b, 0x7d, offsetof(CallInfo, u.l.base)); \
  /* add (arg)*sizeof(TValue), %rdi */ \
  APPEND3(0x48, 0x81, 0xc7);\
  APPEND((arg)*sizeof(TValue), 4);

#define RK_RDI(k) APPEND2(0x48, 0xbf);APPEND((uint64_t)(k), 8);

#define RKBC_RDI(arg, p) \
  if (ISK(arg)) { \
    RK_RDI(&p->k[INDEXK(arg)]); \
  }\
  else { \
    RABC_RDI(arg); \
  }

#define RABC_RSI(arg) \
  /* mov offset8(%r13), %rsi*/ \
  APPEND4(0x49, 0x8b, 0x75, offsetof(CallInfo, u.l.base)); \
  if ((arg) > 0) { \
    /* add (arg)*sizeof(TValue), %rsi */ \
    APPEND3(0x48, 0x81, 0xc6);\
    APPEND((arg)*sizeof(TValue), 4); \
  }

#define RK_RSI(k) APPEND2(0x48, 0xbe);APPEND((uint64_t)(k), 8);

#define RKBC_RSI(arg, p) \
  if (ISK(arg)) { \
    RK_RSI(&p->k[INDEXK(arg)]); \
  }\
  else { \
    RABC_RSI(arg); \
  }

#define RABC_RDX(arg) \
  /* mov offset8(%r13), %rdx */ \
  APPEND4(0x49, 0x8b, 0x55, offsetof(CallInfo, u.l.base)); \
  if ((arg) > 0) { \
    /* add (arg)*sizeof(TValue), %rdx */ \
    APPEND3(0x48, 0x81, 0xc2);\
    APPEND((arg)*sizeof(TValue), 4); \
  }

#define RK_RDX(k) APPEND2(0x48, 0xba);APPEND((uint64_t)(k), 8);

#define RKBC_RDX(arg, p) \
  if (ISK(arg)) { \
    RK_RDX(&p->k[INDEXK(arg)]); \
  }\
  else { \
    RABC_RDX(arg); \
  }

#define RABC_RCX(arg) \
  /* mov offset8(%r13), %rcx */ \
  APPEND4(0x49, 0x8b, 0x4d, offsetof(CallInfo, u.l.base)); \
  if ((arg) > 0) { \
    /* add (arg)*sizeof(TValue), %rcx */ \
    APPEND3(0x48, 0x81, 0xc1);\
    APPEND((arg)*sizeof(TValue), 4); \
  }

#define RK_RCX(k) APPEND2(0x48, 0xb9);APPEND((uint64_t)(k), 8);

#define RKBC_RCX(arg, p) \
  if (ISK(arg)) { \
    RK_RCX(&p->k[INDEXK(arg)]); \
  }\
  else { \
    RABC_RCX(arg); \
  }

#define RABC_R8(arg) \
  /* mov offset8(%r13), %r8 */ \
  APPEND4(0x4d, 0x8b, 0x45, offsetof(CallInfo, u.l.base)); \
  if ((arg) > 0) { \
    /* add (arg)*sizeof(TValue), %r8 */ \
    APPEND3(0x49, 0x81, 0xc0);\
    APPEND((arg)*sizeof(TValue), 4); \
  }

#define RK_R8(k) APPEND2(0x49, 0xb8);APPEND((uint64_t)(k), 8);

#define RKBC_R8(arg, p) \
  if (ISK(arg)) { \
    RK_R8(&p->k[INDEXK(arg)]); \
  }\
  else { \
    RABC_R8(arg); \
  }

#define RABC_R9(arg) \
  /* mov offset8(%r13), %r9 */ \
  APPEND4(0x4d, 0x8b, 0x4d, offsetof(CallInfo, u.l.base)); \
  if ((arg) > 0) { \
    /* add (arg)*sizeof(TValue), %r9 */ \
    APPEND3(0x49, 0x81, 0xc1);\
    APPEND((arg)*sizeof(TValue), 4); \
  }

#define RK_R9(k) APPEND2(0x49, 0xb9);APPEND((uint64_t)(k), 8);

#define RKBC_R9(arg, p) \
  if (ISK(arg)) { \
    RK_R9(&p->k[INDEXK(arg)]); \
  }\
  else { \
    RABC_R9(arg); \
  }

#define JIT_RABC(arg) { \
  /* mov offset8(%r13), %rax */ \
  APPEND4(0x49, 0x8b, 0x45, offsetof(CallInfo, u.l.base)); \
  if ((arg) > 0) { \
    /* add arg*sizeof(TValue), %rax*/\
    APPEND2(0x48, 0x05);\
    APPEND((arg)*sizeof(TValue), 4);\
  } \
}

/**
 * Increment savedpc used in lvm.c
 */
#define LUA_ADD_SAVEDPC(arg) \
  /* addq arg*sizeof(Instrcution),(%r12) */ \
  APPEND4(0x49, 0x81, 0x04, 0x24); \
  APPEND((arg)*sizeof(Instruction), 4);


#define VM_CALL_SZ 5
#define VM_CALL(func) do { \
  int32_t calloff = (unsigned char *)(func) - (prog+VM_CALL_SZ); \
  APPEND1(0xe8); \
  APPENDOFF(calloff); \
} while(0);

#define SAVE_REGISTERS \
  /* mov %rbx, -8(%rbp) */ \
  APPEND4(0x48, 0x89, 0x5d, 0xf8); \
  /* mov %r12, -16(%rbp) */ \
  APPEND4(0x4c, 0x89, 0x65, 0xf0); \
  /* mov %r13, -24(%rbp) */ \
  APPEND4(0x4c, 0x89, 0x6d, 0xe8); \

#define RESTORE_REGISTERS \
  /* mov -8(%rbp), %rbx */ \
  APPEND4(0x48, 0x8b, 0x5d, 0xf8); \
  /* mov -16(%rbp), %r12 */ \
  APPEND4(0x4c, 0x8b, 0x65, 0xf0); \
  /* mov -24(%rbp), %r13 */ \
  APPEND4(0x4c, 0x8b, 0x6d, 0xe8); \

#define JIT_PROLOGUE \
  APPEND4(0x55, 0x48, 0x89, 0xe5); /* push %rbp; mov %rsp,%rbp */ \
  APPEND4(0x48, 0x83, 0xec, 32);    /* subq  $24,%rsp       */ \
  /* First, save registers */ \
  SAVE_REGISTERS; \
  /* put L in %rbx */ \
  APPEND3(0x48, 0x89, 0xfb); /* mov %rdi, %rbx */ \
  /* put ci in r13 */ \
  APPEND3(0x49, 0x89, 0xf5); \
  \
  /* put &ci->u.l.savedpc in %r12 */ \
  /* lea offset8(%r13),%rax */ \
  APPEND4(0x4d, 0x8d, 0x65, offsetof(CallInfo, u.l.savedpc)); \
  /* jmpq *%rcx */ \
  APPEND2(0xff, 0xe1);


/**
 * Generic
 */
static inline uint8_t *op_generic(uint8_t *bin, Proto *p _AU_, const Instruction *code _AU_,
    unsigned int *addrs _AU_, int pc _AU_)
{
  uint8_t *prog = bin;
  uint32_t mask = (LUA_MASKLINE | LUA_MASKCOUNT);

  LUA_ADD_SAVEDPC(1);
  /* testb $0xc, 0xc8(%rbx) */
  APPEND3(0xf6, 0x83, offsetof(lua_State, hookmask));
  APPEND(htonl(mask), 4);
  /* je +offset : move to the next instruction */
  APPEND2(X86_JE, 8);
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  VM_CALL(luaG_traceexec);
  return prog;
}

/**
 * OP_MOVE opcode
 */
static uint8_t *op_move_create(uint8_t *bin, Proto *p _AU_, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  RABC_RSI(GETARG_A(code[pc]));
  RABC_RDX(GETARG_B(code[pc]));
  /* mov (%rdx), %rax */
  APPEND3(0x48, 0x8b, 0x02);
  /* mov %rax, (%rsi) */
  APPEND3(0x48, 0x89, 0x06);
  /* mov 0x08(%rdx), %eax */
  APPEND3(0x8b, 0x42, 0x08);
  /* mov %eax, 0x08(%rsi) */
  APPEND3(0x89, 0x46, 0x08);
  return prog;
}

/**
 * OP_LOADK opcode
 */
static uint8_t *op_loadk_create(uint8_t *bin, Proto *p, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  TValue *rb = p->k + GETARG_Bx(code[pc]);
  /* Get RA in rax */
  JIT_RABC(GETARG_A(code[pc]));
  /* mov rb->_value, %r10 */
  APPEND2(0x49, 0xba);
  APPEND((uint64_t)val_(rb).p, 8);
  /* mov %r10, (%rax) */
  APPEND3(0x4c, 0x89, 0x10);
  /* movl rb->_tt, 0x8(%rax) */
  APPEND3(0xc7, 0x40, 0x8);
  APPEND(rttype(rb), 4);
  return prog;
}

/**
 * OP_LOADKX opcode
 */
static uint8_t *op_loadkx_create(uint8_t *bin, Proto *p, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  TValue *rax = p->k + GETARG_Ax(code[pc+1]);

  /* Get RA in rax */
  JIT_RABC(GETARG_A(code[pc]));
  /* mov rb->_value, %r10 */
  APPEND2(0x49, 0xba);
  APPEND((uint64_t)val_(rax).p, 8);
  /* mov %r10, (%rax) */
  APPEND3(0x4c, 0x89, 0x10);
  /* movl rax->_tt, 0x8(%rax) */
  APPEND3(0xc7, 0x40, 0x8);
  APPEND(rttype(rax), 4);
  return prog;
}

/**
 * OP_LOADBOOL opcode
 */
static inline uint8_t *op_loadbool_create(uint8_t *bin, Proto *p _AU_, const Instruction *code,
    unsigned int *addrs, int pc)
{
  uint8_t *prog = bin;
  RABC_RDI(GETARG_A(code[pc]));
  /* mov GETARG_B(i), %esi */
  APPEND1(0xbe);
  APPEND(GETARG_B(code[pc]), 4);
  /* mov %esi, (%rdi) */
  APPEND2(0x89, 0x37);
  /* movl   LUA_TBOOLEAN,0x8(%rdi) */
  APPEND3(0xc7, 0x47, 0x08);
  APPEND(LUA_TBOOLEAN, 4);
  if (GETARG_C(code[pc])) {
    LUA_ADD_SAVEDPC(1);
    APPEND2(X86_NJ, addrs[pc+2] - addrs[pc+1]);
  }
  return prog;
}

/**
 * OP_LOADNIL opcode
 */
static uint8_t *op_loadnil_create(uint8_t *bin, Proto *p _AU_, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  int b = GETARG_B(code[pc]);

  RABC_RDI(GETARG_A(code[pc]));
  do {
    /* movl $0x0, offset(%rdi) */
    APPEND3(0xc7, 0x47, offsetof(TValue, tt_));
    APPEND(0x0, 4);
    /* add sizeof(TValue), %rdi */
    APPEND4(0x48, 0x83, 0xc7, sizeof(TValue));
  } while(b--);
  return prog;
}

/**
 * OP_GETUPVAL opcode
 */
static uint8_t *op_getupval_create(uint8_t *bin, Proto *p _AU_, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  RABC_RSI(GETARG_A(code[pc]));
  /* mov GETARG_B(i), %edx */
  APPEND1(0xba);
  APPEND(GETARG_B(code[pc]), 4);
  VM_CALL(vm_getupval);
  return prog;
}

/**
 * OP_GETTABUP opcode
 */
static uint8_t *op_gettabup_create(uint8_t *bin, Proto *p, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  /* mov GETARG_B(i), %esi */
  APPEND1(0xbe);
  APPEND(GETARG_B(code[pc]), 4);
  RKBC_RDX(GETARG_C(code[pc]), p);
  RABC_RCX(GETARG_A(code[pc]));
  VM_CALL(vm_gettabup);
  /* vm_gettabup can realloc base, reset it */
  return prog;
}

/**
 * OP_GETTABLE opcode
 */
static uint8_t *op_gettable_create(uint8_t *bin, Proto *p, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  RABC_RSI(GETARG_B(code[pc]));
  RKBC_RDX(GETARG_C(code[pc]), p);
  RABC_RCX(GETARG_A(code[pc]));
  VM_CALL(vm_gettable);
  return prog;
}

/**
 * OP_SETTABUP opcode
 */
static uint8_t *op_settabup_create(uint8_t *bin, Proto *p, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  /* mov GETARG_A(i), %esi */
  APPEND1(0xbe);
  APPEND(GETARG_A(code[pc]), 4);
  RKBC_RDX(GETARG_B(code[pc]), p);
  RKBC_RCX(GETARG_C(code[pc]), p);
  VM_CALL(vm_settabup);
  /* vm_settabup can realloc base, reset it */
  return prog;
}

/**
 * OP_SETUPVAL opcode
 */
static uint8_t *op_setupval_create(uint8_t *bin, Proto *p _AU_, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  RABC_RSI(GETARG_A(code[pc]));
  /* mov GETARG_B(i), %edx */
  APPEND1(0xba);
  APPEND(GETARG_B(code[pc]), 4);
  VM_CALL(vm_setupval);
  return prog;
}

/**
 * OP_SETTABLE opcode
 */
static uint8_t *op_settable_create(uint8_t *bin, Proto *p, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  RABC_RSI(GETARG_A(code[pc]));
  RKBC_RDX(GETARG_B(code[pc]), p);
  RKBC_RCX(GETARG_C(code[pc]), p);
  VM_CALL(vm_settable);
  return prog;
}

/**
 * OP_NEWTABLE opcode
 */
static uint8_t *op_newtable_create(uint8_t *bin, Proto *p _AU_, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  /* mov %r13, %rsi */
  APPEND3(0x4c, 0x89, 0xee);
  RABC_RDX(GETARG_A(code[pc]));
  /* mov GETARG_B(i), %ecx */
  APPEND1(0xb9);
  APPEND(GETARG_B(code[pc]), 4);
  /* mov GETARG_C(i), %r8d */
  APPEND2(0x41, 0xb8);
  APPEND(GETARG_C(code[pc]), 4);
  VM_CALL(vm_newtable);
  return prog;
}

/**
 * OP_SELF
 */
static uint8_t *op_self_create(uint8_t *bin, Proto *p, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  RABC_RSI(GETARG_A(code[pc]));
  RKBC_RDX(GETARG_B(code[pc]), p);
  RKBC_RCX(GETARG_C(code[pc]), p);
  VM_CALL(vm_self);
  return prog;
}

/**
 * OP_ADD opcode
 */
static uint8_t *op_add_create(uint8_t *bin, Proto *p, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  RABC_RSI(GETARG_A(code[pc]));
  /* Get RKB */
  RKBC_RDX(GETARG_B(code[pc]), p);
  /* Get RKC */
  RKBC_RCX(GETARG_C(code[pc]), p);
  VM_CALL(vm_add);
  return prog;
}

/**
 * OP_SUB opcode
 */
static uint8_t *op_sub_create(uint8_t *bin, Proto *p, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  RABC_RSI(GETARG_A(code[pc]));
  /* Get RKB */
  RKBC_RDX(GETARG_B(code[pc]), p);
  /* Get RKC */
  RKBC_RCX(GETARG_C(code[pc]), p);
  VM_CALL(vm_sub);
  return prog;
}

/**
 * OP_MUL opcode
 */
static uint8_t *op_mul_create(uint8_t *bin, Proto *p, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  RABC_RSI(GETARG_A(code[pc]));
  /* Get RKB */
  RKBC_RDX(GETARG_B(code[pc]), p);
  /* Get RKC */
  RKBC_RCX(GETARG_C(code[pc]), p);
  VM_CALL(vm_mul);
  return prog;
}

/**
 * OP_DIV opcode
 */
static uint8_t *op_div_create(uint8_t *bin, Proto *p, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  RABC_RSI(GETARG_A(code[pc]));
  /* Get RKB */
  RKBC_RDX(GETARG_B(code[pc]), p);
  /* Get RKC */
  RKBC_RCX(GETARG_C(code[pc]), p);
  VM_CALL(vm_div);
  return prog;
}

/**
 * OP_IDIV opcode
 */
static uint8_t *op_idiv_create(uint8_t *bin, Proto *p, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  RABC_RSI(GETARG_A(code[pc]));
  /* Get RKB */
  RKBC_RDX(GETARG_B(code[pc]), p);
  /* Get RKC */
  RKBC_RCX(GETARG_C(code[pc]), p);
  VM_CALL(vm_idiv);
  return prog;
}

/**
 * OP_BAND opcode
 */
static uint8_t *op_band_create(uint8_t *bin, Proto *p, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  RABC_RSI(GETARG_A(code[pc]));
  /* Get RKB */
  RKBC_RDX(GETARG_B(code[pc]), p);
  /* Get RKC */
  RKBC_RCX(GETARG_C(code[pc]), p);
  VM_CALL(vm_band);
  return prog;
}

/**
 * OP_BOR opcode
 */
static uint8_t *op_bor_create(uint8_t *bin, Proto *p, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  RABC_RSI(GETARG_A(code[pc]));
  /* Get RKB */
  RKBC_RDX(GETARG_B(code[pc]), p);
  /* Get RKC */
  RKBC_RCX(GETARG_C(code[pc]), p);
  VM_CALL(vm_bor);
  return prog;
}

/**
 * OP_BXOR opcode
 */
static uint8_t *op_bxor_create(uint8_t *bin, Proto *p, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  RABC_RSI(GETARG_A(code[pc]));
  /* Get RKB */
  RKBC_RDX(GETARG_B(code[pc]), p);
  /* Get RKC */
  RKBC_RCX(GETARG_C(code[pc]), p);
  VM_CALL(vm_bxor);
  return prog;
}

/**
 * OP_SHL opcode
 */
static uint8_t *op_shl_create(uint8_t *bin, Proto *p, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  RABC_RSI(GETARG_A(code[pc]));
  /* Get RKB */
  RKBC_RDX(GETARG_B(code[pc]), p);
  /* Get RKC */
  RKBC_RCX(GETARG_C(code[pc]), p);
  VM_CALL(vm_shl);
  return prog;
}

/**
 * OP_SHR opcode
 */
static uint8_t *op_shr_create(uint8_t *bin, Proto *p, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  RABC_RSI(GETARG_A(code[pc]));
  /* Get RKB */
  RKBC_RDX(GETARG_B(code[pc]), p);
  /* Get RKC */
  RKBC_RCX(GETARG_C(code[pc]), p);
  VM_CALL(vm_shr);
  return prog;
}

/**
 * OP_BNOT opcode
 */
static uint8_t *op_bnot_create(uint8_t *bin, Proto *p, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */ \
  APPEND3(0x48, 0x89, 0xdf); \
  RABC_RSI(GETARG_A(code[pc])); \
  /* Get RKB */ \
  RKBC_RDX(GETARG_B(code[pc]), p); \
  VM_CALL(vm_bnot); \
  return prog;
}

/**
 * OP_MOD opcode
 */
static uint8_t *op_mod_create(uint8_t *bin, Proto *p, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  RABC_RSI(GETARG_A(code[pc]));
  /* Get RKB */
  RKBC_RDX(GETARG_B(code[pc]), p);
  /* Get RKC */
  RKBC_RCX(GETARG_C(code[pc]), p);
  VM_CALL(vm_mod);
  /* vm_mod can realloc base, reset it */
  return prog;
}

/**
 * OP_POW opcode
 */
static uint8_t *op_pow_create(uint8_t *bin, Proto *p, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  RABC_RSI(GETARG_A(code[pc]));
  /* Get RKB */
  RKBC_RDX(GETARG_B(code[pc]), p);
  /* Get RKC */
  RKBC_RCX(GETARG_C(code[pc]), p);
  VM_CALL(vm_pow);
  /* vm_mod can realloc base, reset it */
  return prog;
}

/**
 * OP_UNM
 */
static uint8_t *op_unm_create(uint8_t *bin, Proto *p, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */ \
  APPEND3(0x48, 0x89, 0xdf); \
  RABC_RSI(GETARG_A(code[pc])); \
  /* Get RKB */ \
  RKBC_RDX(GETARG_B(code[pc]), p); \
  VM_CALL(vm_unm); \
  /* vm_unm can realloc base, reset it */ \
  return prog;
}

/**
 * OP_NOT opcode
 */
static uint8_t *op_not_create(uint8_t *bin, Proto *p _AU_, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  RABC_RDI(GETARG_A(code[pc])); \
  RABC_RSI(GETARG_B(code[pc])); \
  VM_CALL(vm_not);
  return prog;
}

/**
 * OP_LEN opcode
 */
static uint8_t *op_len_create(uint8_t *bin, Proto *p _AU_, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  RABC_RSI(GETARG_A(code[pc]));
  RABC_RDX(GETARG_B(code[pc]));
  VM_CALL(luaV_objlen);
  /* luaV_objlen can realloc base, reset it */
  return prog;
}

/**
 * OP_CONCAT opcode
 */
static uint8_t *op_concat_create(uint8_t *bin, Proto *p _AU_, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  /* mov offset8(%r13), %rsi */
  APPEND4(0x49, 0x8b, 0x75, offsetof(CallInfo, u.l.base));
  /* mov GETARG_B(i), %edx */
  APPEND1(0xba);
  APPEND(GETARG_B(code[pc]), 4);
  /* mov GETARG_C(i), %ecx */
  APPEND1(0xb9);
  APPEND(GETARG_C(code[pc]), 4);
  VM_CALL(vm_concat);
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  /* mov %r13, %rsi */
  APPEND3(0x4c, 0x89, 0xee);
  RABC_RDX(GETARG_A(code[pc]));
  RABC_RCX(GETARG_B(code[pc]));
  VM_CALL(vm_setconcat);
  return prog;
}

/**
 * OP_JMP opcode
 */
static uint8_t *op_jmp_create(uint8_t *bin, Proto *p _AU_, const Instruction *code,
    unsigned int *addrs, int pc)
{
  uint8_t *prog = bin;
  int a = GETARG_A(code[pc]);
  if (a != 0) {
    /* mov %rbx, %rdi */
    APPEND3(0x48, 0x89, 0xdf);
    RABC_RSI(a - 1);
    VM_CALL(luaF_close);
  }
  LUA_ADD_SAVEDPC(GETARG_sBx(code[pc]));
  APPEND1(X86_LJ);
  APPEND(addrs[pc+1+GETARG_sBx(code[pc])] - addrs[pc+1], 4);
  return prog;
}

/**
 * OP_EQ opcode
 */
static uint8_t *op_eq_create(uint8_t *bin, Proto *p, const Instruction *code,
    unsigned int *addrs, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  RKBC_RSI(GETARG_B(code[pc]), p);
  RKBC_RDX(GETARG_C(code[pc]), p);
  VM_CALL(vm_eq);
  /* cmp GETARG_A(i), %eax */
  APPEND1(0x3d);
  APPEND(GETARG_A(code[pc]), 4);
  APPEND2(X86_JE, 10);
  LUA_ADD_SAVEDPC(1);
  APPEND2(X86_NJ, addrs[pc+2] - addrs[pc+1]);
  return prog;
}

/**
 * OP_LT opcode
 */
static uint8_t *op_lt_create(uint8_t *bin, Proto *p, const Instruction *code,
    unsigned int *addrs, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  RKBC_RSI(GETARG_B(code[pc]), p);
  RKBC_RDX(GETARG_C(code[pc]), p);
  VM_CALL(luaV_lessthan);
  /* cmp GETARG_A(i), %eax */
  APPEND1(0x3d);
  APPEND(GETARG_A(code[pc]), 4);
  APPEND2(X86_JE, 10);
  LUA_ADD_SAVEDPC(1);
  APPEND2(X86_NJ, addrs[pc+2] - addrs[pc+1]);
  return prog;
}

/**
 * OP_LE opcode
 */
static uint8_t *op_le_create(uint8_t *bin, Proto *p, const Instruction *code,
    unsigned int *addrs, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  RKBC_RSI(GETARG_B(code[pc]), p);
  RKBC_RDX(GETARG_C(code[pc]), p);
  VM_CALL(luaV_lessequal);
  /* cmp GETARG_A(i), %eax */
  APPEND1(0x3d);
  APPEND(GETARG_A(code[pc]), 4);
  APPEND2(X86_JE, 10);
  LUA_ADD_SAVEDPC(1);
  APPEND2(X86_NJ, addrs[pc+2] - addrs[pc+1]);
  return prog;
}

/**
 * OP_TEST opcode
 */
static uint8_t *op_test_create(uint8_t *bin, Proto *p _AU_, const Instruction *code,
    unsigned int *addrs, int pc)
{
  uint8_t *prog = bin;
  RABC_RDI(GETARG_A(code[pc]));
  /* mov GETARG_C(i), %esi */
  APPEND1(0xbe);
  APPEND(GETARG_C(code[pc]), 4);
  VM_CALL(vm_test);
  /* cmpl %$0x1, %eax */
  APPEND3(0x83, 0xf8, 0X01);
  APPEND2(X86_JNE, 10);
  LUA_ADD_SAVEDPC(1);
  APPEND2(X86_NJ, addrs[pc+2] - addrs[pc+1]);
  return prog;
}

/**
 * OP_TESTSET opcode
 */
static uint8_t *op_testset_create(uint8_t *bin, Proto *p _AU_, const Instruction *code,
    unsigned int *addrs, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  RABC_RSI(GETARG_A(code[pc]));
  RABC_RDX(GETARG_B(code[pc]));
  /* movl GETARG_C(i), %ecx */
  APPEND1(0xb9);
  APPEND(GETARG_C(code[pc]), 4);
  VM_CALL(vm_testset);
  /* cmpl %$0x1, %eax */
  APPEND3(0x83, 0xf8, 0x01);
  /* jne +offset */
  APPEND2(X86_JNE, 10); /* jump to next OP (OP_JMP) */
  /* else jump over the next OP_JUMP */
  LUA_ADD_SAVEDPC(1);
  APPEND2(X86_NJ, addrs[pc+2] - addrs[pc+1]);
  return prog;
}

/**
 * OP_CALL opcode
 */
static uint8_t *op_call_create(uint8_t *bin, Proto *p _AU_, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  int b = GETARG_B(code[pc]);
  int nresults = GETARG_C(code[pc]) - 1;

  if (b != 0) {
    /* L->top = ra+b */
    RABC_RSI(GETARG_A(code[pc])+b);
    /* mov %rsi, offset(%rbx) */
    APPEND4(0x48, 0x89, 0x73, offsetof(lua_State, top));
  }

  /* call luaD_precall */
  /* mov %rbx, %rdi */ \
  APPEND3(0x48, 0x89, 0xdf);
  /* mov ra, %rsi */
  RABC_RSI(GETARG_A(code[pc]));
  /* mov nresult, %edx */
  APPEND1(0xba);
  APPEND(nresults, 4);
  VM_CALL(luaD_precall);

  /* testl %eax, %eax */
  APPEND2(0x85, 0xc0);
  /* je +offset (to movl instruction) */
  APPEND2(X86_JE, (nresults >= 0)?10:2);
  /* C function called */
  if (nresults >= 0) {
    /* L->top = ci->top */
    /* mov offset(%r13), %r10 */
    APPEND4(0x4d, 0x8b, 0x55, offsetof(CallInfo, top));
    /* mov %r10, offset(%rbx) */
    APPEND4(0x4c, 0x89, 0x53, offsetof(lua_State, top));
  }
  /* jmp +offset (to next OP) */
  APPEND2(X86_NJ, 16);
  /* incl %eax */
  APPEND2(0xff, 0xc0);
  /* on call, just return to luaV_execute in new frame */
  RESTORE_REGISTERS;
  /* leaveq; retq */
  APPEND2(0xc9, 0xc3);
  return prog;
}

/**
 * OP_TAILCALL opcaode
 */
static uint8_t *op_tailcall_create(uint8_t *bin, Proto *p _AU_, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  /* mov %r13, %rsi */
  APPEND3(0x4c, 0x89, 0xee);
  /* movl GETARG_A(i), %edx */
  APPEND1(0xba);
  APPEND(GETARG_A(code[pc]), 4);
  /* movl GETARG_B(i), %ecx */
  APPEND1(0xb9);
  APPEND(GETARG_B(code[pc]), 4);
  VM_CALL(vm_tailcall);

  /* if C function we have to Jump to next Lua return */
  /* cmpl $0x0, %eax */
  APPEND3(0x83, 0xf8, 0x0);
  /* jeq +offset8 */
  APPEND2(X86_JE, 14);

  RESTORE_REGISTERS;
  /* %eax is 0x01, used in luaV_execute */
  /* leaveq; retq */
  APPEND2(0xc9, 0xc3);
  return prog;
}

/**
 * OP_RETURN opcode
 */
static uint8_t *op_return_create(uint8_t *bin, Proto *p _AU_, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  RABC_RSI(GETARG_A(code[pc]));
  /* mov %r13, %rdx */
  APPEND3(0x4c, 0x89, 0xea);
  /* mov GETARG_B(i), %ecx */
  APPEND1(0xb9);
  APPEND(GETARG_B(code[pc]), 4);
  VM_CALL(vm_return);
  RESTORE_REGISTERS;
  /* leaveq; retq */
  APPEND2(0xc9, 0xc3);
  return prog;
}

/**
 * OP_FORLOOP opcode
 */
static uint8_t *op_forloop_create(uint8_t *bin, Proto *p _AU_, const Instruction *code,
    unsigned int *addrs, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  RABC_RSI(GETARG_A(code[pc]));
  VM_CALL(vm_forloop);
  /* cmpl %$0x1, %eax */
  APPEND3(0x83, 0xf8, 0x01);
  /* jne +offset to next instruction */
  APPEND2(X86_JNE, 13);
  /* else */
  LUA_ADD_SAVEDPC(GETARG_sBx(code[pc]));
  /* jmp +offset */
  APPEND1(X86_LJ);
  APPEND(addrs[pc+1+GETARG_sBx(code[pc])] - addrs[pc+1], 4);
  return prog;
}

/**
 * OP_FORPREP opcode
 */
static uint8_t *op_forprep_create(uint8_t *bin, Proto *p _AU_, const Instruction *code,
    unsigned int *addrs, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  RABC_RSI(GETARG_A(code[pc]));
  VM_CALL(vm_forprep);
  LUA_ADD_SAVEDPC(GETARG_sBx(code[pc]));
  APPEND1(X86_LJ);
  APPEND(addrs[pc+1+GETARG_sBx(code[pc])] - addrs[pc+1], 4);
  return prog;
}

/**
 * OP_TFORCALL opcode
 */
static uint8_t *op_tforcall_create(uint8_t *bin, Proto *p _AU_, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  /* mov %r13, %rsi */
  APPEND3(0x4c, 0x89, 0xee);
  RABC_RDX(GETARG_A(code[pc]));
  /* mov GETARG_C(i), %ecx */
  APPEND1(0xb9);
  APPEND(GETARG_C(code[pc]), 4);
  VM_CALL(vm_tforcall);
  return prog;
}

/**
 * OP_TFORLOOP opcode
 */
static uint8_t *op_tforloop_create(uint8_t *bin, Proto *p _AU_, const Instruction *code,
    unsigned int *addrs, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  RABC_RSI(GETARG_A(code[pc]));
  VM_CALL(vm_tforloop);
  /* cmpl %$0x1, %eax */
  APPEND3(0x83, 0xf8, 0x01);
  APPEND2(X86_JNE, 13); /* jump to next op */
  /* else */
  LUA_ADD_SAVEDPC(GETARG_sBx(code[pc]));
  APPEND1(X86_LJ);
  APPEND(addrs[pc+1+GETARG_sBx(code[pc])] - addrs[pc+1], 4);
  return prog;
}

/**
 * OP_SETLIST opcode
 */
static uint8_t *op_setlist_create(uint8_t *bin, Proto *p _AU_, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  /* mov %r13, %rsi */
  APPEND3(0x4c, 0x89, 0xee);
  RABC_RDX(GETARG_A(code[pc]));
  /* mov GETARG_B(i), %ecx */
  APPEND1(0xb9);
  APPEND(GETARG_B(code[pc]), 4);
  /* mov GETARG_C(i), %r8d */
  APPEND2(0x41, 0xb8);
  APPEND(GETARG_C(code[pc]), 4);
  /* mov GETARG_Ax(i+1), %r9d */
  APPEND2(0x41, 0xb9);
  APPEND(GETARG_Ax(code[pc+1]), 4);
  VM_CALL(vm_setlist);
  return prog;
}

/**
 * OP_CLOSURE opcode
 */
static uint8_t *op_closure_create(uint8_t *bin, Proto *p _AU_, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  RABC_RSI(GETARG_A(code[pc]));
  /* mov %r13, %rdx */
  APPEND3(0x4c, 0x89, 0xea);
  /* movl GETARG_Bx(i), %ecx */
  APPEND1(0xb9);
  APPEND(GETARG_Bx(code[pc]), 4);
  VM_CALL(vm_closure);
  return prog;
}

/**
 * OP_VARARG opcode
 */
static uint8_t *op_vararg_create(uint8_t *bin, Proto *p _AU_, const Instruction *code,
    unsigned int *addrs _AU_, int pc)
{
  uint8_t *prog = bin;
  /* mov %rbx, %rdi */
  APPEND3(0x48, 0x89, 0xdf);
  /* mov %r13, %rsi */
  APPEND3(0x4c, 0x89, 0xee);
  /* movl GETARG_A(i), %edx */
  APPEND1(0xba);
  APPEND(GETARG_A(code[pc]), 4);
  /* movl GETARG_B(i), %ecx */
  APPEND1(0xb9);
  APPEND(GETARG_B(code[pc]), 4);
  VM_CALL(vm_vararg);
  return prog;
}

/**
 * OP_EXTRAARG opcode
 */
static uint8_t *op_extraarg_create(uint8_t *bin, Proto *p _AU_, const Instruction *code _AU_,
    unsigned int *addrs _AU_, int pc _AU_)
{
  uint8_t *prog = bin;
  NOP;
  return prog;
}

static uint8_t *(*jit_create_funcs[NUM_OPCODES]) (uint8_t *bin, Proto *p, const Instruction *code, unsigned int *addrs, int pc) =
{
  op_move_create,/* OP_MOVE,  A B  R(A) := R(B)          */
  op_loadk_create,/* OP_LOADK,  A Bx  R(A) := Kst(Bx)          */
  op_loadkx_create,/* OP_LOADKX,  A  R(A) := Kst(extra arg)        */
  op_loadbool_create,/* OP_LOADBOOL,  A B C  R(A) := (Bool)B; if (C) pc++      */
  op_loadnil_create,/* OP_LOADNIL,  A B  R(A), R(A+1), ..., R(A+B) := nil    */
  op_getupval_create,/* OP_GETUPVAL,  A B  R(A) := UpValue[B]        */
  op_gettabup_create,/* OP_GETTABUP,  A B C  R(A) := UpValue[B][RK(C)]      */
  op_gettable_create,/* OP_GETTABLE,  A B C  R(A) := R(B)[RK(C)]        */
  op_settabup_create,/* OP_SETTABUP,A B C  UpValue[A][RK(B)] := RK(C)      */
  op_setupval_create,/* OP_SETUPVAL,  A B  UpValue[B] := R(A)        */
  op_settable_create,/* OP_SETTABLE,  A B C  R(A)[RK(B)] := RK(C)        */
  op_newtable_create,/* OP_NEWTABLE,  A B C  R(A) := {} (size = B,C)        */
  op_self_create,/* OP_SELF,  A B C  R(A+1) := R(B); R(A) := R(B)[RK(C)]    */
  op_add_create,/* OP_ADD,  A B C  R(A) := RK(B) + RK(C)        */
  op_sub_create,/* OP_SUB,  A B C  R(A) := RK(B) - RK(C)        */
  op_mul_create,/* OP_MUL,  A B C  R(A) := RK(B) * RK(C)        */
  op_mod_create,/* OP_MOD,  A B C  R(A) := RK(B) % RK(C)        */
  op_pow_create,/* OP_POW,  A B C  R(A) := RK(B) ^ RK(C)        */
  op_div_create,/* OP_DIV, A B C R(A) := RK(B) / RK(C)       */
  op_idiv_create, /* OP_IDIV, A B C R(A) := RK(B) // RK(C)        */
  op_band_create, /* OP_BAND, A B C R(A) := RK(B) & RK(C)       */
  op_bor_create, /* OP_BOR, A B C R(A) := RK(B) | RK(C)       */
  op_bxor_create, /* OP_BXOR, A B C R(A) := RK(B) ~ RK(C)       */
  op_shl_create, /* OP_SHL, A B C R(A) := RK(B) << RK(C)        */
  op_shr_create, /* OP_SHR, A B C R(A) := RK(B) >> RK(C)        */
  op_unm_create,/* OP_UNM,  A B  R(A) := -R(B)          */
  op_bnot_create, /* OP_BNOT, A B R(A) := ~R(B)         */
  op_not_create,/* OP_NOT,  A B  R(A) := not R(B)        */
  op_len_create,/* OP_LEN,  A B  R(A) := length of R(B)        */
  op_concat_create,/* OP_CONCAT,  A B C  R(A) := R(B).. ... ..R(C)      */
  op_jmp_create,/* OP_JMP,  A sBx  pc+=sBx; if (A) close all upvalues >= R(A) + 1  */
  op_eq_create,/* OP_EQ,  A B C  if ((RK(B) == RK(C)) ~= A) then pc++    */
  op_lt_create,/* OP_LT,  A B C  if ((RK(B) <  RK(C)) ~= A) then pc++    */
  op_le_create,/* OP_LE,  A B C  if ((RK(B) <= RK(C)) ~= A) then pc++    */
  op_test_create,/* OP_TEST,  A C  if not (R(A) <=> C) then pc++      */
  op_testset_create,/* OP_TESTSET,  A B C  if (R(B) <=> C) then R(A) := R(B) else pc++  */
  op_call_create,/* OP_CALL,  A B C  R(A), ... ,R(A+C-2) := R(A)(R(A+1), ... ,R(A+B-1)) */
  op_tailcall_create,/* OP_TAILCALL,  A B C  return R(A)(R(A+1), ... ,R(A+B-1))    */
  op_return_create,/* OP_RETURN,  A B  return R(A), ... ,R(A+B-2)  (see note)  */
  op_forloop_create,/* OP_FORLOOP,  A sBx  R(A)+=R(A+2);
                       if R(A) <?= R(A+1) then { pc+=sBx; R(A+3)=R(A) }*/
  op_forprep_create,/* OP_FORPREP,  A sBx  R(A)-=R(A+2); pc+=sBx        */
  op_tforcall_create,/* OP_TFORCALL,  A C  R(A+3), ... ,R(A+2+C) := R(A)(R(A+1), R(A+2));  */
  op_tforloop_create,/* OP_TFORLOOP,  A sBx  if R(A+1) ~= nil then { R(A)=R(A+1); pc += sBx }*/
  op_setlist_create,/* OP_SETLIST,  A B C  R(A)[(C-1)*FPF+i] := R(A+i), 1 <= i <= B  */
  op_closure_create,/* OP_CLOSURE,  A Bx  R(A) := closure(KPROTO[Bx])      */
  op_vararg_create,/* OP_VARARG,  A B  R(A), R(A+1), ..., R(A+B-2) = vararg    */
  op_extraarg_create/* OP_EXTRAARG  Ax  extra (larger) argument for previous opcode  */
};

#endif
