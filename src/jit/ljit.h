/*
** ljit.h - Remi Bauzac (rbauzac@arkoon.net)
** Lua JiT skeleton
** See Copyright Notice in lua.h
*/
#ifndef __ljit_h__
#define __ljit_h__

#include "../lobject.h"
#include <stdint.h>

static inline uint8_t *append(uint8_t *ptr, uint64_t bytes, unsigned int len)
{
  if (ptr) {
    if (len == 1) {
      *ptr = bytes;
    }
    else if (len == 2) {
      *(uint16_t *)ptr = bytes;
    }
    else if (len == 4) {
      *(uint32_t *)ptr = bytes;
    }
    else {
      *(uint64_t *)ptr = bytes;
    }
  }
  return ((uint8_t *)ptr) + len;
}

static inline uint8_t *appendoff(uint8_t *ptr, uint32_t offset) {
    *(uint32_t *)ptr = offset;
  return ptr + sizeof(uint32_t);
}

static inline uint8_t is_imm8(int value)
{
  return value <= 127 && value >= -128;
}

#define APPEND(bytes, len)        do { prog = append(prog, bytes, len); } while (0)
#define APPENDOFF(offset)         do { prog = appendoff(prog, offset); } while (0)

#define APPEND1(b1)               APPEND(b1, 1)
#define APPEND2(b1, b2)           APPEND((b1) + ((b2) << 8), 2)
#define APPEND3(b1, b2, b3)       APPEND2(b1,b2);APPEND1(b3)
#define APPEND4(b1, b2, b3, b4)   APPEND((b1) + ((b2) << 8) + ((b3) << 16) + ((b4) << 24), 4)

#define JITADDR_NONE -1U

int luaJ_need(lua_State* L, Proto *p);
int luaJ_create(lua_State* L, Proto *p);
void luaJ_free(lua_State* L, Proto *p);
/* VM functions */
void vm_add(lua_State* L, TValue *ra, TValue *rb, TValue *rc);
void vm_sub(lua_State* L, TValue *ra, TValue *rb, TValue *rc);
void vm_mul(lua_State* L, TValue *ra, TValue *rb, TValue *rc);
void vm_div(lua_State* L, TValue *ra, TValue *rb, TValue *rc);
void vm_idiv(lua_State* L, TValue *ra, TValue *rb, TValue *rc);
void vm_band(lua_State* L, TValue *ra, TValue *rb, TValue *rc);
void vm_bor(lua_State* L, TValue *ra, TValue *rb, TValue *rc);
void vm_bxor(lua_State* L, TValue *ra, TValue *rb, TValue *rc);
void vm_shl(lua_State* L, TValue *ra, TValue *rb, TValue *rc);
void vm_shr(lua_State* L, TValue *ra, TValue *rb, TValue *rc);
void vm_bnot(lua_State* L, TValue *ra, TValue *rb);
void vm_mod(lua_State* L, TValue *ra, TValue *rb, TValue *rc);
void vm_pow(lua_State* L, TValue *ra, TValue *rb, TValue *rc);
void vm_unm(lua_State* L, TValue *ra, TValue *rb);
void vm_gettabup(lua_State* L, int b, TValue *rkc, TValue *ra);
void vm_closure(lua_State* L, TValue *ra, CallInfo *ci, int bx);
void vm_settabup(lua_State* L, int a, TValue *rkb, TValue *rkc);
void vm_settable(lua_State* L, const TValue *t, TValue *key, StkId val);
void vm_gettable(lua_State *L, const TValue *t, TValue *key, StkId val);
int vm_return(lua_State* L, TValue *ra, CallInfo *ci, int b);
void vm_newtable(lua_State* L, CallInfo *ci, TValue *ra, int b, int c);
void vm_setupval(lua_State* L, TValue *ra, int b);
void vm_forprep(lua_State* L, TValue *ra);
int vm_forloop(lua_State* L, TValue *ra);
void vm_self(lua_State* L, TValue *ra, TValue *rb, TValue *rc);
void vm_setlist(lua_State* L, CallInfo *ci, TValue *ra, int b, int c, int ax);
void vm_tforcall(lua_State* L, CallInfo *ci, TValue *ra, int c);
int vm_test(TValue *ra, int c);
void vm_setbool(TValue *a, int b);
void vm_setnil(TValue *a, int b);
void vm_not(TValue *ra, TValue *rb);
void vm_setconcat(lua_State* L, CallInfo *ci, TValue *ra, TValue *rb);
void vm_vararg(lua_State* L, CallInfo *ci, int a, int b);
int vm_tailcall(lua_State* L, CallInfo *ci, int a, int b);
int vm_testset(lua_State* L, TValue *ra, TValue *rb, int c);
#endif
