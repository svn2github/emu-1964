/*$T hardware.h GC 1.136 02/28/02 08:03:01 */


/*$6
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 The structures for the ultra processors and memory
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */


/*
 * 1964 Copyright (C) 1999-2004 Joel Middendorf, <schibo@emulation64.com> This
 * program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version. This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details. You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. To contact the
 * authors: email: schibo@emulation64.com, rice1964@yahoo.com
 */
#ifndef _HARDWARE_H__1964_
#define _HARDWARE_H__1964_

typedef		int			BOOL;

#include "globals.h"	/* loads the rom and handle endian stuff */

#define SHIFTER1_READ	2				/* Shifts off insignificant bits from memory_read_functions array size. The
										 * significant bits are 0xFFFC0000 (14 significant bits) because Check_LW needs
										 * to check SPREG_1, which is at 0xA4040000. So we only need 14bit instead of
										 * 16bit. */
#define SHIFTER1_WRITE	0				/* Shifts off insignificant bits from memory write functions array size. Set to
										 * zero because of protected memory in 0x1000 blocks. = All bits are
										 * significant. */

#define SHIFTER2_READ	(16 + SHIFTER1_READ)
#define SHIFTER2_WRITE	(12 + SHIFTER1_WRITE)

/*
 -----------------------------------------------------------------------------------------------------------------------
    includes for all Hardware parts
 -----------------------------------------------------------------------------------------------------------------------
 */

#define __pc		  32
#define __g_Lookup_HI 32
#define __LO		35
#define __HI		36
//if u change this, change NUM_CONSTS.

typedef struct	sHardwareState
{
	uint32	COP0Reg[32];			/* Coprocessor0 Registers */
	uint32	fpr32[64];				/* 32bit 64 items needed! */
	uint32	LLbit;					/* LoadLinked Bit */
	uint32	COP1Con[32];			/* FPControl Registers, only 0 and 31 is used */
	uint32	COP0Con[64];			
	uint32 *(*memory_read_functions[0x10000>>SHIFTER1_READ])();
    uint32 *(*memory_write_fun_eax_only[0x100000>>SHIFTER1_WRITE])();
	uint8  *sDYN_PC_LOOKUP[0x10000];
} HardwareState;


extern HardwareState	gHardwareState;

#define MAXTLB				32

typedef struct
{
	uint32	valid;
	uint32	EntryHi;
	uint32	EntryLo1;
	uint32	EntryLo0;
	uint64	PageMask;
	uint32	LoCompare;
	uint32	MyHiMask;
} tlb_struct;

typedef struct	sMemorySTATE
{
	uint32		*ramRegs0;
	uint32		*ramRegs4;
	uint32		*ramRegs8;
	uint32		*SP_MEM;
	uint32		*SP_REG_1;
	uint32		*SP_REG_2;
	uint32		*DPC;
	uint32		*DPS;
	uint32		*MI;
	uint32		*VI;
	uint32		*AI;
	uint32		*PI;
	uint32		*RI;
	uint32		*SI;
	uint8		*RDRAM;				/* Size = 4MB */
	uint32		*C2A1;
	uint32		*C1A1;
	uint32		*C1A3;
	uint32		*C2A2;
	uint8		*ROM_Image;
	uint32		*GIO_REG;
	uint8		*PIF;
	uint8		*ExRDRAM;			/* Size = 4MB */
	uint8		*dummyNoAccess;		/* handles crap pointers for now..band-aid'ish */
	uint8		*dummyReadWrite;	/* handles crap pointers for now..band-aid'ish */
	uint8		*dummyAllZero;		/* handles crap pointers for all zeros */
	tlb_struct	TLB[MAXTLB];
} MemoryState;

extern MemoryState	gMemoryState;
#define OP_PARAMS	HardwareState * reg
#define PASS_PARAMS reg

#define _r0			0
#define _at			1
#define _v0			2
#define _v1			3
#define _a0			4
#define _a1			5
#define _a2			6
#define _a3			7
#define _t0			8
#define _t1			9
#define _t2			10
#define _t3			11
#define _t4			12
#define _t5			13
#define _t6			14
#define _t7			15
#define _s0			16
#define _s1			17
#define _s2			18
#define _s3			19
#define _s4			20
#define _s5			21
#define _s6			22
#define _s7			23
#define _t8			24
#define _t9			25
#define _k0			26
#define _k1			27
#define _gp			28
#define _sp			29
#define __s8		30
#define _ra			31
#endif
