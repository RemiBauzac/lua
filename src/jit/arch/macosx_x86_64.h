/*
** macosx_x86_64.h - Remi Bauzac (rbauzac@arkoon.net)
** Lua JiT MacOS specific functions for x86_64
** See Copyright Notice in lua.h
*/
#ifndef __macos_x86_64_h__
#define __macos_x86_64_h__

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/errno.h>
#include <unistd.h>
#include "../ljit.h"
#include "x86_64_generic.h"

static inline void jit_free(Proto *p)
{
	int tofree = p->sizejit;
	int pagesz = getpagesize();

	if (!p->jit) {
		return;
	}

	if (p->sizejit % pagesz) {
		tofree = p->sizejit + (pagesz - p->sizejit%pagesz);
	}

	munmap(p->jit, tofree);
	p->jit = NULL;
}

static inline int jit_alloc(Proto *p)
{
	int pagesz = getpagesize();
	int toalloc = p->sizejit;

	if (p->sizejit % pagesz) {
		toalloc = p->sizejit + (pagesz - p->sizejit%pagesz);
	}
	p->jit = mmap(0, toalloc, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_ANON, -1, 0);

	if (p->jit == MAP_FAILED) {
		perror("mmap");
		p->jit = NULL;
		return 1;
	}
	return 0;
}

#endif
