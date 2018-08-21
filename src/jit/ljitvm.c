/*
** ljitvm.c - Remi Bauzac (rbauzac@arkoon.net)
** Lua JiT function calls (named virtual machine)
** See Copyright Notice in lua.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define ljitvm_c
#define LUA_CORE


#include "../lua.h"
#include "../lobject.h"
#include "../lstate.h"
#include "../lopcodes.h"
#include "../lvm.h"
#include "../lgc.h"
#include "../lfunc.h"
#include "../ltable.h"
#include "../ldebug.h"
#include "ljit.h"

#if !defined luai_runtimecheck
#define luai_runtimecheck(L, c)		/* void */
#endif


void vm_hook(lua_State* L)
{
  if ((L->hookmask & (LUA_MASKLINE | LUA_MASKCOUNT)) &&
      (--L->hookcount == 0 || L->hookmask & LUA_MASKLINE)) {
    traceexec(L);
  }
}


void vm_not(TValue *ra, TValue *rb)
{
	int res = l_isfalse(rb);  /* next assignment may change this value */
	setbvalue(ra, res);
}
void vm_mod(lua_State* L, TValue *ra, TValue *rb, TValue *rc)
{
	if (ttisnumber(rb) && ttisnumber(rc)) {
		lua_Number nb = nvalue(rb), nc = nvalue(rc);
		setnvalue(ra, luai_nummod(L, nb, nc));
	}
	else {
		luaV_arith(L, ra, rb, rc, TM_MOD);
	}
}
void vm_pow(lua_State* L, TValue *ra, TValue *rb, TValue *rc)
{
	if (ttisnumber(rb) && ttisnumber(rc)) {
		lua_Number nb = nvalue(rb), nc = nvalue(rc);
		setnvalue(ra, luai_numpow(L, nb, nc));
	}
	else {
		luaV_arith(L, ra, rb, rc, TM_POW);
	}
}
void vm_unm(lua_State* L, TValue *ra, TValue *rb)
{
	if (ttisnumber(rb)) {
		lua_Number nb = nvalue(rb);
		setnvalue(ra, luai_numunm(L, nb));
	}
	else {
		luaV_arith(L, ra, rb, rb, TM_UNM);
	}

}

void vm_gettabup(lua_State* L, int b, TValue *rkc, TValue *ra)
{
	LClosure *cl = clLvalue(L->ci->func);
	luaV_gettable(L, cl->upvals[b]->v, rkc, ra);
}

void vm_settabup(lua_State* L, int a, TValue *rkb, TValue *rkc)
{
	LClosure *cl = clLvalue(L->ci->func);
	luaV_settable(L, cl->upvals[a]->v, rkb, rkc);
}


int vm_call(lua_State* L, TValue *ra, int b, int c, CallInfo *ci)
{
  int nresults = c - 1;

  if (b != 0) L->top = ra+b;  /* else previous instruction set top */
  if (luaD_precall(L, ra, nresults)) {  /* C function? */
    if (nresults >= 0) L->top = ci->top;  /* adjust results */
    return 0;
  }
  else {  /* Lua function */
    L->ci->callstatus |= CIST_REENTRY;
    return 1;
  }
}

void vm_closure(lua_State* L, TValue *ra, CallInfo *ci, int bx)
{
	LClosure *cl = clLvalue(ci->func);
  Proto *p = cl->p->p[bx];
  TValue *base = ci->u.l.base;
  Closure *ncl = getcached(p, cl->upvals, base);  /* cached closure */

  if (ncl == NULL)  /* no match? */
    pushclosure(L, p, cl->upvals, base, ra);  /* create a new one */
  else
    setclLvalue(L, ra, ncl);  /* push cashed closure */
	luaC_condGC(L,{L->top = ra+1;  /* limit of live values */ \
                   luaC_step(L); \
                   L->top = ci->top;})  /* restore top */ \
  luai_threadyield(L);
}

int vm_return(lua_State* L, TValue *ra, CallInfo *ci, int b)
{
	int nb = 0;
	LClosure *cl = clLvalue(ci->func);
  TValue *base = ci->u.l.base;

	if (b != 0) L->top = ra+b-1;
	if (cl->p->sizep > 0) luaF_close(L, base);
	nb = luaD_poscall(L, ra);
	if (!(ci->callstatus & CIST_REENTRY)) { /* 'ci' still the called one */
		return 0;  /* external invocation: return */
	}
	else {  /* invocation via reentry: continue execution */
		if (nb) L->top = L->ci->top;
		lua_assert(isLua(L->ci));
		lua_assert(GET_OPCODE(*((L->ci)->u.l.savedpc - 1)) == OP_CALL);
		return 1;
	}
}

void vm_jumpclose(lua_State* L, CallInfo *ci, int a)
{
	if (a > 0) luaF_close(L, ci->u.l.base + a - 1);
}

void vm_newtable(lua_State* L, CallInfo *ci, TValue *ra, int b, int c)
{
	Table *t = luaH_new(L);

	sethvalue(L, ra, t);
	if (b != 0 || c != 0)
		luaH_resize(L, t, luaO_fb2int(b), luaO_fb2int(c));
	luaC_condGC(L,{L->top = ra+1;  /* limit of live values */ \
                   luaC_step(L); \
                   L->top = ci->top;})  /* restore top */ \
    luai_threadyield(L);
}

void vm_setupval(lua_State* L, TValue *ra, int b)
{
	LClosure *cl = clLvalue(L->ci->func);
	UpVal *uv = cl->upvals[b];
	setobj(L, uv->v, ra);
	luaC_barrier(L, uv, ra);
}

void vm_getupval(lua_State* L, TValue *ra, int b)
{
	LClosure *cl = clLvalue(L->ci->func);
	setobj2s(L, ra, cl->upvals[b]->v);
}

void vm_forprep(lua_State* L, TValue *ra)
{
	const TValue *init = ra;
	const TValue *plimit = ra+1;
	const TValue *pstep = ra+2;

	if (!tonumber(init, ra))
		luaG_runerror(L, LUA_QL("for") " initial value must be a number");
	else if (!tonumber(plimit, ra+1))
		luaG_runerror(L, LUA_QL("for") " limit must be a number");
	else if (!tonumber(pstep, ra+2))
		luaG_runerror(L, LUA_QL("for") " step must be a number");
	setnvalue(ra, luai_numsub(L, nvalue(ra), nvalue(pstep)));
}

int vm_forloop(lua_State* L, TValue *ra)
{
	lua_Number step = nvalue(ra+2);
	lua_Number idx = luai_numadd(L, nvalue(ra), step); /* increment index */
	lua_Number limit = nvalue(ra+1);
	if (luai_numlt(L, 0, step) ? luai_numle(L, idx, limit)
			: luai_numle(L, limit, idx)) {
		setnvalue(ra, idx);  /* update internal index... */
		setnvalue(ra+3, idx);  /* ...and external index */
		return 1;
	}
	return 0;
}

int vm_eq(lua_State* L, TValue *rb, TValue *rc)
{
	return cast_int(equalobj(L, rb, rc));
}

void vm_self(lua_State* L, TValue *ra, TValue *rb, TValue *rc)
{
	setobjs2s(L, ra+1, rb);
	luaV_gettable(L, rb, rc, ra);
}

void vm_setlist(lua_State* L, CallInfo *ci, TValue *ra, int b, int c, int ax)
{
	int n = b;
	int nc = c;
	int last;
	Table *h;
	if (n == 0) n = cast_int(L->top - ra) - 1;
	if (nc == 0) {
		nc = ax;
	}

	luai_runtimecheck(L, ttistable(ra));
	h = hvalue(ra);
	last = ((nc-1)*LFIELDS_PER_FLUSH) + n;
	if (last > h->sizearray)  /* needs more space? */
	luaH_resizearray(L, h, last);  /* pre-allocate it at once */
	for (; n > 0; n--) {
		TValue *val = ra+n;
		luaH_setint(L, h, last--, val);
		luaC_barrierback(L, obj2gco(h), val);
	}
	L->top = ci->top;  /* correct top (in case of previous open call) */
}

void vm_tforcall(lua_State* L, CallInfo *ci, TValue *ra, int c)
{
	StkId cb = ra + 3;  /* call base */
	setobjs2s(L, cb+2, ra+2);
	setobjs2s(L, cb+1, ra+1);
	setobjs2s(L, cb, ra);
	L->top = cb + 3;  /* func. + 2 args (state and index) */
	luaD_call(L, cb, c, 1);
	L->top = ci->top;
}

int vm_tforloop(lua_State* L, TValue *ra)
{
	if (!ttisnil(ra + 1)) {  /* continue loop? */
		setobjs2s(L, ra, ra + 1);  /* save control variable */
		return 1;
	}
	return 0;
}

int vm_test(TValue *ra, int c)
{
	if (c ? l_isfalse(ra) : !l_isfalse(ra)) {
		return 1;
	}
	return 0;
}

int vm_testset(lua_State* L, TValue *ra, TValue *rb, int c)
{
	if (c ? l_isfalse(rb) : !l_isfalse(rb)) {
		return 1;
	}
	setobjs2s(L, ra, rb);
	return 0;
}

void vm_concat(lua_State* L, TValue *base, int b, int c)
{
	L->top = base + c + 1;
	luaV_concat(L, c - b + 1);
}

void vm_setconcat(lua_State* L, CallInfo *ci, TValue *ra, TValue *rb)
{
	setobjs2s(L, ra, rb);
	luaC_condGC(L,{L->top = (ra >= rb ? ra + 1 : rb);  /* limit of live values */ \
                   luaC_step(L); \
                   L->top = ci->top;})  /* restore top */ \
    luai_threadyield(L);
	L->top = ci->top;  /* restore top */
}

void vm_vararg(lua_State* L, CallInfo *ci, int a, int b)
{
	LClosure *cl = clLvalue(ci->func);
  TValue *base = ci->u.l.base;
	int nb = b - 1;
	int j;
	int n = cast_int(base - ci->func) - cl->p->numparams - 1;
	TValue *ra = base + a;

	if (nb < 0) {  /* B == 0? */
		nb = n;  /* get all var. arguments */
		luaD_checkstack(L, n);
		ra = ci->u.l.base + a;
		L->top = ra + n;
	}
	for (j = 0; j < nb; j++) {
		if (j < n) {
			setobjs2s(L, ra + j, ci->u.l.base - n + j);
		}
		else {
			setnilvalue(ra + j);
		}
	}
}

int vm_tailcall(lua_State* L, CallInfo *ci, int a, int b)
{
	LClosure *cl = clLvalue(ci->func);
	TValue *ra = (ci->u.l.base)+a;

	if (b != 0) L->top = ra+b;  /* else previous instruction set top */

	if (luaD_precall(L, ra, LUA_MULTRET)) { /* C function? */
		return 0;
	}
	else {
		int aux;
		/* tail call: put called frame (n) in place of caller one (o) */
		CallInfo *nci = L->ci;  /* called frame */
		CallInfo *oci = nci->previous;  /* caller frame */
		StkId nfunc = nci->func;  /* called function */
		StkId ofunc = oci->func;  /* caller function */
		/* last stack slot filled by 'precall' */
		StkId lim = nci->u.l.base + getproto(nfunc)->numparams;
		/* close all upvalues from previous call */
		if (cl->p->sizep > 0) luaF_close(L, oci->u.l.base);
		/* move new frame into old one */
		for (aux = 0; nfunc + aux < lim; aux++)
			setobjs2s(L, ofunc + aux, nfunc + aux);
		oci->u.l.base = ofunc + (nci->u.l.base - nfunc);  /* correct base */
		oci->top = L->top = ofunc + (L->top - nfunc);  /* correct top */
		oci->u.l.savedpc = nci->u.l.savedpc;
		oci->callstatus |= CIST_TAIL;  /* function was tail called */

		L->ci = oci;  /* remove new frame */
	}
	return 1;
}

