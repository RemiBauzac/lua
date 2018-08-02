/*
** linux_x86_64.h - Remi Bauzac (rbauzac@arkoon.net)
** Lua JiT Linux specific functions for x86_64
** See Copyright Notice in lua.h
*/
#ifndef __linux_x86_64_h__
#define __linux_x86_64_h__

#include <malloc.h>
#include <sys/mman.h>
#include <sys/user.h>
#include "../ljit.h"
#include "x86_64_generic.h"

static inline void jit_free(Proto *p)
{
	if (!p) return;
	free(p->jit);
	p->jit = NULL;
}

static inline int jit_alloc(Proto *p)
{
	p->jit = memalign(PAGE_SIZE, p->sizejit);
	if (!p->jit) return 1;

	if (mprotect(p->jit, p->sizejit, PROT_EXEC|PROT_READ|PROT_WRITE) == -1) {
		jit_free(p);
		return 1;
	}
	return 0;
}


#endif
