/*
 * Copyright (C) Jonathan D. Belanger 2017.
 * All Rights Reserved.
 *
 * This software is furnished under a license and may be used and copied only
 * in accordance with the terms of such license and with the inclusion of the
 * above copyright notice.  This software or any other copies thereof may not
 * be provided or otherwise made available to any other person.  No title to
 * and ownership of the software is hereby transferred.
 *
 * The information in this software is subject to change without notice and
 * should not be construed as a commitment by the author or co-authors.
 *
 * The author and any co-authors assume no responsibility for the use or
 * reliability of this software.
 *
 * Description:
 *
 *	This header file contains the function definitions implemented in the
 *	AXP_21264_Fbox.c module.
 *
 * Revision History:
 *
 *	V01.000		19-Jun-2017	Jonathan D. Belanger
 *	Initially written.
 */
#ifndef _AXP_21264_FBOX_DEFS_
#define _AXP_21264_FBOX_DEFS_

#include "AXP_Utility.h"
#include "AXP_Base_CPU.h"
#include "AXP_21264_CPU.h"

/*
 * Ebox Instruction Prototypes
 *
 * Floating-Point Control
 */
AXP_EXCEPTIONS AXP_FBEQ(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_FBGE(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_FBGT(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_FBLE(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_FBLT(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_FBNE(AXP_21264_CPU *, AXP_INSTRUCTION *);

/*
 * Floating-Point Load/Store
 */
AXP_EXCEPTIONS AXP_LDF(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_LDG(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_LDS(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_LDT(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_STF(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_STG(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_STS(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_STT(AXP_21264_CPU *, AXP_INSTRUCTION *);

/*
 * Floating-Point Operate
 */
AXP_EXCEPTIONS AXP_CPYS(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_CPYSE(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_CPYSN(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_CVTLQ(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_CVTQL(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_CVTQL_V(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_CVTQL_SV(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_FCMOVEQ(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_FCMOVGE(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_FCMOVGT(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_FCMOVLE(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_FCMOVLT(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_FCMOVNE(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_MF_FPCR(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_MT_FPCR(AXP_21264_CPU *, AXP_INSTRUCTION *);

/*
 * FP Operate Function Field Format
 * 	The formatted function field for FP operations is only done for Opcodes
 * 	0x14 and 0x16.
 */
typedef struct
{
	 u32	fnc : 4;
	 u32	src : 2;
	 u32	rnd : 2;
	 u32	trp : 3;
	 u32	res : 21;
} AXP_FP_FUNC;

#define AXP_FP_ADD		0x0
#define AXP_FP_SUB		0x1
#define AXP_FP_MUL		0x2
#define AXP_FP_DIV		0x3
#define AXP_FP_ITOFST	0x4
#define AXP_FP_CMPEQ	0x5
#define AXP_FP_CMPLT	0x6
#define AXP_FP_CMPLE	0x7
#define AXP_FP_SQRTST	0xb
#define AXP_FP_CTVS		0xc
#define AXP_FP_CVTT		0xe
#define AXP_FP_CVTQ		0xf

#define AXP_FP_S		0x0
#define AXP_FP_T		0x2
#define AXP_FP_Q		0x3

#define AXP_FP_CHOPPED	0x0
#define AXP_FP_MINUS_INF 0x1
#define AXP_FP_NORMAL	0x2
#define AXP_FP_DYNAMIC	0x3

#define AXP_FP_IMPRECISE 0x0
#define AXP_FP_UNDERFLOW 0x1

#define AXP_R_SIGN				0x8000000000000000ll
#define AXP_R_NM 				0x8000000000000000ll	/* normalized */
#define AXP_R_HB				0x0010000000000000ll
#define AXP_R_NAN				0x7ff
#define AXP_R_GUARD				(63 - 52)
#define AXP_R_LONG_SMALL		0xffffffff00000000ll
#define AXP_R_LONG_LARGE		0x000000007fffffffll
#define AXP_R_CQ_NAN			0xfff8000000000000ll
#define AXP_F_BIAS				0x080
#define AXP_G_BIAS				0x400
#define AXP_S_BIAS				0x7f
#define AXP_S_NAN				0xff
#define AXP_T_BIAS				0x3ff
#define AXP_D_BIAS				0x80
#define AXP_D_GUARD				(63 - 55)

#define AXP_R_Q2L_OVERFLOW(val)	(((val) & AXP_R_SIGN) ? 					\
								 ((val) < AXP_R_LONG_SMALL) :				\
								 ((val) > AXP_R_LONG_LARGE))

#endif /* _AXP_21264_FBOX_DEFS_ */