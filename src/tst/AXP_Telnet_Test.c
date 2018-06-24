/*
 * Copyright (C) Jonathan D. Belanger 2018.
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
 *  This file contains the test code for the TELNET processing.
 *
 * Revision History:
 *
 *  V01.000	16-Jun-2018	Jonathan D. Belanger
 *  Initially written.
 */
#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_Trace.h"
#include "AXP_Telnet.h"

extern AXP_StateMachine	TN_Option_SM[AXP_OPT_MAX_ACTION][AXP_OPT_MAX_STATE];
extern AXP_StateMachine	TN_Receive_SM[AXP_ACT_MAX][AXP_RCV_MAX_STATE];
void Send_DO(void *, ...);
void Send_DONT(void *, ...);
void Send_WILL(void *, ...);
void Send_WONT(void *, ...);
void Echo_Data(void *, ...);
void Process_CMD(void *, ...);
void Cvt_Process_IAC(void *, ...);
void SubOpt_Clear(void *, ...);
void SubOpt_Accumulate(void *, ...);
void SubOpt_TermProcess(void *, ...);
void Process_Suboption(void *, ...);

typedef struct
{
    u8	currentState;
    u8	action;
    u8	resultantState;
    u16	actionMask;
} AXP_Test_SM;
u16 testActionMask;
#define NO_ACTION		0x0000
#define WILL_SENT		0x0001
#define WONT_SENT		0x0002
#define DO_SENT			0x0004
#define DONT_SENT		0x0008
#define ECHO_DATA		0x0010
#define PROC_CMD		0x0100
#define SUBOPT_CLEAR		0x0200
#define SUBOPT_ACCUM		0x0400
#define PROC_IAC		0x0800
#define PROC_SUBOPT		0x1000
#define CVT_PROC_IAC		0x2000
#define SUBOPT_TERM		0x4000
#define CLEAR_ACTION_MASK	(testActionMask) = 0;

#define YES_CLI_NOPREF		(YES_CLI-YES_SRV)*2
#define YES_CLI_PREF		(YES_CLI-YES_SRV)*2+1
#define NO_CLI_NOPREF		(NO_CLI-YES_SRV)*2
#define NO_CLI_PREF		(NO_CLI-YES_SRV)*2+1
#define YES_SRV_NOPREF		(YES_SRV-YES_SRV)*2
#define YES_SRV_PREF		(YES_SRV-YES_SRV)*2+1
#define NO_SRV_NOPREF		(NO_SRV-YES_SRV)*2
#define NO_SRV_PREF		(NO_SRV-YES_SRV)*2+1
#define WILL_NOPREF		(WILL-YES_SRV)*2
#define WILL_PREF		(WILL-YES_SRV)*2+1
#define WONT_NOPREF		(WONT-YES_SRV)*2
#define WONT_PREF		(WONT-YES_SRV)*2+1
#define DO_NOPREF		(DO-YES_SRV)*2
#define DO_PREF			(DO-YES_SRV)*2+1
#define DONT_NOPREF		(DONT-YES_SRV)*2
#define DONT_PREF		(DONT-YES_SRV)*2+1

AXP_Test_SM SM_Opt_Tests[] =
{

    /*
     * Set Remote Option (Yes) (using their options state machine)
     *
     * Current State		Action		ResultantState		ActionMask
     */
    {AXP_OPT_NO,		YES_CLI_PREF,	AXP_OPT_WANTYES_LOCAL,	DO_SENT},	/* 0 */
    {AXP_OPT_NO,		YES_CLI_NOPREF,	AXP_OPT_WANTYES_LOCAL,	DO_SENT},
    {AXP_OPT_YES,		YES_CLI_PREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_YES,		YES_CLI_NOPREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_WANTNO_LOCAL,	YES_CLI_PREF,	AXP_OPT_WANTNO_REMOTE,	NO_ACTION},
    {AXP_OPT_WANTNO_LOCAL,	YES_CLI_NOPREF,	AXP_OPT_WANTNO_REMOTE,	NO_ACTION},
    {AXP_OPT_WANTNO_REMOTE,	YES_CLI_PREF,	AXP_OPT_WANTNO_REMOTE,	NO_ACTION},
    {AXP_OPT_WANTNO_REMOTE,	YES_CLI_NOPREF,	AXP_OPT_WANTNO_REMOTE,	NO_ACTION},
    {AXP_OPT_WANTYES_LOCAL,	YES_CLI_PREF,	AXP_OPT_WANTYES_LOCAL,	NO_ACTION},
    {AXP_OPT_WANTYES_LOCAL,	YES_CLI_NOPREF,	AXP_OPT_WANTYES_LOCAL,	NO_ACTION},
    {AXP_OPT_WANTYES_REMOTE,	YES_CLI_PREF,	AXP_OPT_WANTYES_LOCAL,	NO_ACTION},
    {AXP_OPT_WANTYES_REMOTE,	YES_CLI_NOPREF,	AXP_OPT_WANTYES_LOCAL,	NO_ACTION},

    /*
     * Set Remote Option (No) (using their options state machine)
     *
     * Current State		Action		ResultantState		ActionMask
     */
    {AXP_OPT_NO,		NO_CLI_PREF,	AXP_OPT_NO,		NO_ACTION},	/* 12 */
    {AXP_OPT_NO,		NO_CLI_NOPREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_YES,		NO_CLI_PREF,	AXP_OPT_WANTNO_LOCAL,	DONT_SENT},
    {AXP_OPT_YES,		NO_CLI_NOPREF,	AXP_OPT_WANTNO_LOCAL,	DONT_SENT},
    {AXP_OPT_WANTNO_LOCAL,	NO_CLI_PREF,	AXP_OPT_WANTNO_LOCAL,	NO_ACTION},
    {AXP_OPT_WANTNO_LOCAL,	NO_CLI_NOPREF,	AXP_OPT_WANTNO_LOCAL,	NO_ACTION},
    {AXP_OPT_WANTNO_REMOTE,	NO_CLI_PREF,	AXP_OPT_WANTNO_LOCAL,	NO_ACTION},
    {AXP_OPT_WANTNO_REMOTE,	NO_CLI_NOPREF,	AXP_OPT_WANTNO_LOCAL,	NO_ACTION},
    {AXP_OPT_WANTYES_LOCAL,	NO_CLI_PREF,	AXP_OPT_WANTYES_REMOTE,	NO_ACTION},
    {AXP_OPT_WANTYES_LOCAL,	NO_CLI_NOPREF,	AXP_OPT_WANTYES_REMOTE,	NO_ACTION},
    {AXP_OPT_WANTYES_REMOTE,	NO_CLI_PREF,	AXP_OPT_WANTYES_REMOTE, NO_ACTION},
    {AXP_OPT_WANTYES_REMOTE,	NO_CLI_NOPREF,	AXP_OPT_WANTYES_REMOTE, NO_ACTION},

    /*
     * Set Local Option (Yes) (using my options state machine)
     *
     * Current State		Action		ResultantState		ActionMask
     */
    {AXP_OPT_NO,		YES_SRV_PREF,	AXP_OPT_WANTYES_LOCAL,	WILL_SENT},	/* 24 */
    {AXP_OPT_NO,		YES_SRV_NOPREF,	AXP_OPT_WANTYES_LOCAL,	WILL_SENT},
    {AXP_OPT_YES,		YES_SRV_PREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_YES,		YES_SRV_NOPREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_WANTNO_LOCAL,	YES_SRV_PREF,	AXP_OPT_WANTNO_REMOTE,	NO_ACTION},
    {AXP_OPT_WANTNO_LOCAL,	YES_SRV_NOPREF,	AXP_OPT_WANTNO_REMOTE,	NO_ACTION},
    {AXP_OPT_WANTNO_REMOTE,	YES_SRV_PREF,	AXP_OPT_WANTNO_REMOTE,	NO_ACTION},
    {AXP_OPT_WANTNO_REMOTE,	YES_SRV_NOPREF,	AXP_OPT_WANTNO_REMOTE,	NO_ACTION},
    {AXP_OPT_WANTYES_LOCAL,	YES_SRV_PREF,	AXP_OPT_WANTYES_LOCAL,	NO_ACTION},
    {AXP_OPT_WANTYES_LOCAL,	YES_SRV_NOPREF,	AXP_OPT_WANTYES_LOCAL,	NO_ACTION},
    {AXP_OPT_WANTYES_REMOTE,	YES_SRV_PREF,	AXP_OPT_WANTYES_LOCAL,	NO_ACTION},
    {AXP_OPT_WANTYES_REMOTE,	YES_SRV_NOPREF,	AXP_OPT_WANTYES_LOCAL,	NO_ACTION},

    /*
     * Set Local Option (No) (using my options state machine)
     *
     * Current State		Action		ResultantState		ActionMask
     */
    {AXP_OPT_NO,		NO_SRV_PREF,	AXP_OPT_NO,		NO_ACTION},	/* 36 */
    {AXP_OPT_NO,		NO_SRV_NOPREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_YES,		NO_SRV_PREF,	AXP_OPT_WANTNO_LOCAL,	WONT_SENT},
    {AXP_OPT_YES,		NO_SRV_NOPREF,	AXP_OPT_WANTNO_LOCAL,	WONT_SENT},
    {AXP_OPT_WANTNO_LOCAL,	NO_SRV_PREF,	AXP_OPT_WANTNO_LOCAL,	NO_ACTION},
    {AXP_OPT_WANTNO_LOCAL,	NO_SRV_NOPREF,	AXP_OPT_WANTNO_LOCAL,	NO_ACTION},
    {AXP_OPT_WANTNO_REMOTE,	NO_SRV_PREF,	AXP_OPT_WANTNO_LOCAL,	NO_ACTION},
    {AXP_OPT_WANTNO_REMOTE,	NO_SRV_NOPREF,	AXP_OPT_WANTNO_LOCAL,	NO_ACTION},
    {AXP_OPT_WANTYES_LOCAL,	NO_SRV_PREF,	AXP_OPT_WANTYES_REMOTE,	NO_ACTION},
    {AXP_OPT_WANTYES_LOCAL,	NO_SRV_NOPREF,	AXP_OPT_WANTYES_REMOTE,	NO_ACTION},
    {AXP_OPT_WANTYES_REMOTE,	NO_SRV_PREF,	AXP_OPT_WANTYES_REMOTE,	NO_ACTION},
    {AXP_OPT_WANTYES_REMOTE,	NO_SRV_NOPREF,	AXP_OPT_WANTYES_REMOTE,	NO_ACTION},

    /*
     * Receive WILL (using their options state machine)
     *
     * Current State		Action		ResultantState		ActionMask
     */
    {AXP_OPT_NO,		WILL_PREF,	AXP_OPT_YES,		DO_SENT},	/* 48 */
    {AXP_OPT_NO,		WILL_NOPREF,	AXP_OPT_NO,		DONT_SENT},
    {AXP_OPT_YES,		WILL_PREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_YES,		WILL_NOPREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_WANTNO_LOCAL,	WILL_PREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_WANTNO_LOCAL,	WILL_NOPREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_WANTNO_REMOTE,	WILL_PREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_WANTNO_REMOTE,	WILL_NOPREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_WANTYES_LOCAL,	WILL_PREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_WANTYES_LOCAL,	WILL_NOPREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_WANTYES_REMOTE,	WILL_PREF,	AXP_OPT_WANTNO_LOCAL,	DONT_SENT},
    {AXP_OPT_WANTYES_REMOTE,	WILL_NOPREF,	AXP_OPT_WANTNO_LOCAL,	DONT_SENT},

    /*
     * Receive WONT (using their options state machine)
     *
     * Current State		Action		ResultantState		ActionMask
     */
    {AXP_OPT_NO,		WONT_PREF,	AXP_OPT_NO,		NO_ACTION},	/* 60 */
    {AXP_OPT_NO,		WONT_NOPREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_YES,		WONT_PREF,	AXP_OPT_NO,		DONT_SENT},
    {AXP_OPT_YES,		WONT_NOPREF,	AXP_OPT_NO,		DONT_SENT},
    {AXP_OPT_WANTNO_LOCAL,	WONT_PREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_WANTNO_LOCAL,	WONT_NOPREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_WANTNO_REMOTE,	WONT_PREF,	AXP_OPT_WANTYES_LOCAL,	DO_SENT},
    {AXP_OPT_WANTNO_REMOTE,	WONT_NOPREF,	AXP_OPT_WANTYES_LOCAL,	DO_SENT},
    {AXP_OPT_WANTYES_LOCAL,	WONT_PREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_WANTYES_LOCAL,	WONT_NOPREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_WANTYES_REMOTE,	WONT_PREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_WANTYES_REMOTE,	WONT_NOPREF,	AXP_OPT_NO,		NO_ACTION},

    /*
     * Receive DO (using my options state machine)
     *
     * Current State		Action		ResultantState		ActionMask
     */
    {AXP_OPT_NO,		DO_PREF,	AXP_OPT_YES,		WILL_SENT},	/* 72 */
    {AXP_OPT_NO,		DO_NOPREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_YES,		DO_PREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_YES,		DO_NOPREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_WANTNO_LOCAL,	DO_PREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_WANTNO_LOCAL,	DO_NOPREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_WANTNO_REMOTE,	DO_PREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_WANTNO_REMOTE,	DO_NOPREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_WANTYES_LOCAL,	DO_PREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_WANTYES_LOCAL,	DO_NOPREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_WANTYES_REMOTE,	DO_PREF,	AXP_OPT_WANTNO_LOCAL,	WONT_SENT},
    {AXP_OPT_WANTYES_REMOTE,	DO_NOPREF,	AXP_OPT_WANTNO_LOCAL,	WONT_SENT},

    /*
     * Receive DONT (using my options state machine)
     *
     * Current State		Action		ResultantState		ActionMask
     */
    {AXP_OPT_NO,		DONT_PREF,	AXP_OPT_NO,		NO_ACTION},	/* 84 */
    {AXP_OPT_NO,		DONT_NOPREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_YES,		DONT_PREF,	AXP_OPT_NO,		WONT_SENT},
    {AXP_OPT_YES,		DONT_NOPREF,	AXP_OPT_NO,		WONT_SENT},
    {AXP_OPT_WANTNO_LOCAL,	DONT_PREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_WANTNO_LOCAL,	DONT_NOPREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_WANTNO_REMOTE,	DONT_PREF,	AXP_OPT_WANTYES_LOCAL,	WILL_SENT},
    {AXP_OPT_WANTNO_REMOTE,	DONT_NOPREF,	AXP_OPT_WANTYES_LOCAL,	WILL_SENT},
    {AXP_OPT_WANTYES_LOCAL,	DONT_PREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_WANTYES_LOCAL,	DONT_NOPREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_WANTYES_REMOTE,	DONT_PREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_WANTYES_REMOTE,	DONT_NOPREF,	AXP_OPT_NO,		NO_ACTION},

    /*
     * This is the last entry.
     */
    {AXP_OPT_MAX_STATE,		0,		0,			0}		/* 96 */
};

AXP_Test_SM SM_Rcv_Tests[] =
{

    /*
     * Current State		Action		ResultantState		ActionMask
     */
    {AXP_RCV_DATA,		AXP_ACT_NUL,	AXP_RCV_DATA,		ECHO_DATA},	/* 0 */
    {AXP_RCV_DATA,		AXP_ACT_IAC,	AXP_RCV_IAC,		NO_ACTION},
    {AXP_RCV_DATA,		AXP_ACT_R,	AXP_RCV_CR,		ECHO_DATA},
    {AXP_RCV_DATA,		AXP_ACT_CMD,	AXP_RCV_DATA,		ECHO_DATA},
    {AXP_RCV_DATA,		AXP_ACT_SE,	AXP_RCV_DATA,		CVT_PROC_IAC},
    {AXP_RCV_DATA,		AXP_ACT_SB,	AXP_RCV_DATA,		ECHO_DATA},
    {AXP_RCV_DATA,		AXP_ACT_CATCHALL,AXP_RCV_DATA,		ECHO_DATA},

    /*
     * Current State		Action		ResultantState		ActionMask
     */
    {AXP_RCV_IAC,		AXP_ACT_NUL,	AXP_RCV_DATA,		NO_ACTION},	/* 7 */
    {AXP_RCV_IAC,		AXP_ACT_IAC,	AXP_RCV_DATA,		ECHO_DATA},
    {AXP_RCV_IAC,		AXP_ACT_R,	AXP_RCV_DATA,		NO_ACTION},
    {AXP_RCV_IAC,		AXP_ACT_CMD,	AXP_RCV_CMD,		NO_ACTION},
    {AXP_RCV_IAC,		AXP_ACT_SE,	AXP_RCV_DATA,		NO_ACTION},
    {AXP_RCV_IAC,		AXP_ACT_SB,	AXP_RCV_SB,		SUBOPT_CLEAR},
    {AXP_RCV_IAC,		AXP_ACT_CATCHALL,AXP_RCV_DATA,		NO_ACTION},

    /*
     * Current State		Action		ResultantState		ActionMask
     */
    {AXP_RCV_CMD,		AXP_ACT_NUL,	AXP_RCV_DATA,		PROC_CMD},	/* 14 */
    {AXP_RCV_CMD,		AXP_ACT_IAC,	AXP_RCV_DATA,		PROC_CMD},
    {AXP_RCV_CMD,		AXP_ACT_R,	AXP_RCV_DATA,		PROC_CMD},
    {AXP_RCV_CMD,		AXP_ACT_CMD,	AXP_RCV_DATA,		PROC_CMD},
    {AXP_RCV_CMD,		AXP_ACT_SE,	AXP_RCV_DATA,		PROC_CMD},
    {AXP_RCV_CMD,		AXP_ACT_SB,	AXP_RCV_DATA,		PROC_CMD},
    {AXP_RCV_CMD,		AXP_ACT_CATCHALL,AXP_RCV_DATA,		PROC_CMD},

    /*
     * Current State		Action		ResultantState		ActionMask
     */
    {AXP_RCV_CR,		AXP_ACT_NUL,	AXP_RCV_DATA,		NO_ACTION},	/* 21 */
    {AXP_RCV_CR,		AXP_ACT_IAC,	AXP_RCV_DATA,		ECHO_DATA},
    {AXP_RCV_CR,		AXP_ACT_R,	AXP_RCV_DATA,		NO_ACTION},
    {AXP_RCV_CR,		AXP_ACT_CMD,	AXP_RCV_DATA,		ECHO_DATA},
    {AXP_RCV_CR,		AXP_ACT_SE,	AXP_RCV_DATA,		ECHO_DATA},
    {AXP_RCV_CR,		AXP_ACT_SB,	AXP_RCV_DATA,		ECHO_DATA},
    {AXP_RCV_CR,		AXP_ACT_CATCHALL,AXP_RCV_DATA,		ECHO_DATA},

    /*
     * Current State		Action		ResultantState		ActionMask
     */
    {AXP_RCV_SB,		AXP_ACT_NUL,	AXP_RCV_SB,		SUBOPT_ACCUM},	/* 28 */
    {AXP_RCV_SB,		AXP_ACT_IAC,	AXP_RCV_SE,		NO_ACTION},
    {AXP_RCV_SB,		AXP_ACT_R,	AXP_RCV_SB,		SUBOPT_ACCUM},
    {AXP_RCV_SB,		AXP_ACT_CMD,	AXP_RCV_SB,		SUBOPT_ACCUM},
    {AXP_RCV_SB,		AXP_ACT_SE,	AXP_RCV_SB,		SUBOPT_ACCUM},
    {AXP_RCV_SB,		AXP_ACT_SB,	AXP_RCV_SB,		SUBOPT_TERM},
    {AXP_RCV_SB,		AXP_ACT_CATCHALL,AXP_RCV_SB,		SUBOPT_ACCUM},

    /*
     * Current State		Action		ResultantState		ActionMask
     */
    {AXP_RCV_SE,		AXP_ACT_NUL,	AXP_RCV_IAC,		CVT_PROC_IAC},	/* 35 */
    {AXP_RCV_SE,		AXP_ACT_IAC,	AXP_RCV_SB,		SUBOPT_ACCUM},
    {AXP_RCV_SE,		AXP_ACT_R,	AXP_RCV_IAC,		CVT_PROC_IAC},
    {AXP_RCV_SE,		AXP_ACT_CMD,	AXP_RCV_IAC,		CVT_PROC_IAC},
    {AXP_RCV_SE,		AXP_ACT_SE,	AXP_RCV_DATA,		SUBOPT_TERM},
    {AXP_RCV_SE,		AXP_ACT_SB,	AXP_RCV_IAC,		CVT_PROC_IAC},
    {AXP_RCV_SE,		AXP_ACT_CATCHALL,AXP_RCV_IAC,		CVT_PROC_IAC},

    /*
     * This is the last entry.
     */
    {AXP_RCV_MAX_STATE,		0,		0,			0}		/* 42 */
};

/*
 * These are test action routines that will be called instead of the real ones.
 */
void Test_Send_DO(void *sesPtr, ...)
{
    testActionMask |= DO_SENT;
    return;
}
void Test_Send_DONT(void *sesPtr, ...)
{
    testActionMask |= DONT_SENT;
    return;
}
void Test_Send_WILL(void *sesPtr, ...)
{
    testActionMask |= WILL_SENT;
    return;
}
void Test_Send_WONT(void *sesPtr, ...)
{
    testActionMask |= WONT_SENT;
    return;
}
void Test_Echo_Data(void *sesPtr, ...)
{
    testActionMask |= ECHO_DATA;
    return;
}
void Test_Process_CMD(void *sesPtr, ...)
{
    testActionMask |= PROC_CMD;
    return;
}
void Test_Cvt_Process_IAC(void *sesPtr, ...)
{
    testActionMask |= CVT_PROC_IAC;
    return;
}
void Test_SubOpt_Clear(void *sesPtr, ...)
{
    testActionMask |= SUBOPT_CLEAR;
    return;
}
void Test_SubOpt_Accumulate(void *sesPtr, ...)
{
    testActionMask |= SUBOPT_ACCUM;
    return;
}
void Test_SubOpt_TermProcess(void *sesPtr, ...)
{
    testActionMask |= SUBOPT_TERM;
    return;
}
void Test_Process_IAC(void *sesPtr, ...)
{
    testActionMask |= PROC_IAC;
    return;
}
void Test_Process_Suboption(void *sesPtr, ...)
{
    testActionMask |= PROC_SUBOPT;
    return;
}

bool test_options_StateMachine(void)
{
    bool		retVal = true;
    int			ii, jj, kk = 1;
    u8			nextState;

    /*
     * First things first, we need to substitute the real action routines with
     * the test versions.
     */
    printf("...Initializing Option State Machine for Testing...\n");
    for (ii = 0; ii < AXP_OPT_MAX_ACTION; ii++)
	for (jj = 0; jj < AXP_OPT_MAX_STATE; jj++)
	{
	    if (TN_Option_SM[ii][jj].actionRtn != NULL)
	    {
		if (TN_Option_SM[ii][jj].actionRtn == Send_DO)
		    TN_Option_SM[ii][jj].actionRtn = Test_Send_DO;
		else if (TN_Option_SM[ii][jj].actionRtn == Send_DONT)
		    TN_Option_SM[ii][jj].actionRtn = Test_Send_DONT;
		else if (TN_Option_SM[ii][jj].actionRtn == Send_WILL)
		    TN_Option_SM[ii][jj].actionRtn = Test_Send_WILL;
		else if (TN_Option_SM[ii][jj].actionRtn == Send_WONT)
		    TN_Option_SM[ii][jj].actionRtn = Test_Send_WONT;
	    }
	}

    /*
     * Now we can run our tests.  Loop through the test cases, execute the
     * state machine and determine if what occurred is what was expected (as
     * far as state transitions and action routines called).
     */
    ii = 0;
    while ((SM_Opt_Tests[ii].currentState < AXP_OPT_MAX_STATE) && (retVal == true))
    {
	printf(
	    "...Executing Option State Machine Test %3d for [%d]="
	    "{curState: %d, action: %d, nextState: %d, action: 0x%04x}...",
	    kk++,
	    ii,
	    SM_Opt_Tests[ii].currentState,
	    SM_Opt_Tests[ii].action,
	    SM_Opt_Tests[ii].resultantState,
	    SM_Opt_Tests[ii].actionMask);
	testActionMask = 0;
	nextState = AXP_Execute_SM(
			AXP_OPT_MAX_ACTION,
			AXP_OPT_MAX_STATE,
			TN_Option_SM,
			SM_Opt_Tests[ii].action,
			SM_Opt_Tests[ii].currentState,
			NULL);
	printf(
	    " got {x,x, nextState: %d, action: 0x%04x}...\n",
	    nextState,
	    testActionMask);
	if ((nextState != SM_Opt_Tests[ii].resultantState) ||
	    (testActionMask != SM_Opt_Tests[ii].actionMask))
	    retVal = false;
	ii++;
    }

    /*
     * Last things last, we need to replace the real action routines.
     */
    printf("...Resetting Option State Machine for Use...\n");
    for (ii = 0; ii < AXP_OPT_MAX_ACTION; ii++)
	for (jj = 0; jj < AXP_OPT_MAX_STATE; jj++)
	{
	    if (TN_Option_SM[ii][jj].actionRtn != NULL)
	    {
		if (TN_Option_SM[ii][jj].actionRtn == Test_Send_DO)
		    TN_Option_SM[ii][jj].actionRtn = Send_DO;
		else if (TN_Option_SM[ii][jj].actionRtn == Test_Send_DONT)
		    TN_Option_SM[ii][jj].actionRtn = Send_DONT;
		else if (TN_Option_SM[ii][jj].actionRtn == Test_Send_WILL)
		    TN_Option_SM[ii][jj].actionRtn = Send_WILL;
		else if (TN_Option_SM[ii][jj].actionRtn == Test_Send_WONT)
		    TN_Option_SM[ii][jj].actionRtn = Send_WONT;
	    }
	}

    /*
     * Now, let's do the session state machine.
     */
    if (retVal == true)
    {

	    /*
	     * First things first, we need to substitute the real action
	     * routines with the test versions.
	     */
	    printf("...Initializing Receive State Machine for Testing...\n");
	    for (ii = 0; ii < AXP_ACT_MAX; ii++)
		for (jj = 0; jj < AXP_RCV_MAX_STATE; jj++)
		{
		    if (TN_Receive_SM[ii][jj].actionRtn != NULL)
		    {
			if (TN_Receive_SM[ii][jj].actionRtn == Echo_Data)
			    TN_Receive_SM[ii][jj].actionRtn = Test_Echo_Data;
			else if (TN_Receive_SM[ii][jj].actionRtn == Process_CMD)
			    TN_Receive_SM[ii][jj].actionRtn = Test_Process_CMD;
			else if (TN_Receive_SM[ii][jj].actionRtn == Cvt_Process_IAC)
			    TN_Receive_SM[ii][jj].actionRtn = Test_Cvt_Process_IAC;
			else if (TN_Receive_SM[ii][jj].actionRtn == SubOpt_Clear)
			    TN_Receive_SM[ii][jj].actionRtn = Test_SubOpt_Clear;
			else if (TN_Receive_SM[ii][jj].actionRtn == SubOpt_Accumulate)
			    TN_Receive_SM[ii][jj].actionRtn = Test_SubOpt_Accumulate;
			else if (TN_Receive_SM[ii][jj].actionRtn == SubOpt_TermProcess)
			    TN_Receive_SM[ii][jj].actionRtn = Test_SubOpt_TermProcess;
			else if (TN_Receive_SM[ii][jj].actionRtn == Process_Suboption)
			    TN_Receive_SM[ii][jj].actionRtn = Test_Process_Suboption;
		    }
		}

	    /*
	     * Now we can run our tests.  Loop through the test cases, execute the
	     * state machine and determine if what occurred is what was expected (as
	     * far as state transitions and action routines called).
	     */
	    ii = 0;
	    while ((SM_Rcv_Tests[ii].currentState < AXP_RCV_MAX_STATE) && (retVal == true))
	    {
		printf(
		    "...Executing Receive State Machine Test %3d for [%d]="
		    "{curState: %d, action: %d, nextState: %d, action: 0x%04x}...",
		    kk++,
		    ii,
		    SM_Rcv_Tests[ii].currentState,
		    SM_Rcv_Tests[ii].action,
		    SM_Rcv_Tests[ii].resultantState,
		    SM_Rcv_Tests[ii].actionMask);
		testActionMask = 0;
		nextState = AXP_Execute_SM(
				AXP_ACT_MAX,
				AXP_RCV_MAX_STATE,
				TN_Receive_SM,
				SM_Rcv_Tests[ii].action,
				SM_Rcv_Tests[ii].currentState,
				NULL);
		printf(
		    " got {x,x, nextState: %d, action: 0x%04x}...\n",
		    nextState,
		    testActionMask);
		if ((nextState != SM_Rcv_Tests[ii].resultantState) ||
		    (testActionMask != SM_Rcv_Tests[ii].actionMask))
		    retVal = false;
		ii++;
	    }

	    /*
	     * Last things last, we need to substitute the real action routines
	     * with the test versions.
	     */
	    printf("...Resetting Receive State Machine for Use...\n");
	    for (ii = 0; ii < AXP_ACT_MAX; ii++)
		for (jj = 0; jj < AXP_RCV_MAX_STATE; jj++)
		{
		    if (TN_Receive_SM[ii][jj].actionRtn != NULL)
		    {
			if (TN_Receive_SM[ii][jj].actionRtn == Test_Echo_Data)
			    TN_Receive_SM[ii][jj].actionRtn = Echo_Data;
			else if (TN_Receive_SM[ii][jj].actionRtn == Test_Process_CMD)
			    TN_Receive_SM[ii][jj].actionRtn = Process_CMD;
			else if (TN_Receive_SM[ii][jj].actionRtn == Test_Cvt_Process_IAC)
			    TN_Receive_SM[ii][jj].actionRtn = Cvt_Process_IAC;
			else if (TN_Receive_SM[ii][jj].actionRtn == Test_SubOpt_Clear)
			    TN_Receive_SM[ii][jj].actionRtn = SubOpt_Clear;
			else if (TN_Receive_SM[ii][jj].actionRtn == Test_SubOpt_Accumulate)
			    TN_Receive_SM[ii][jj].actionRtn = SubOpt_Accumulate;
			else if (TN_Receive_SM[ii][jj].actionRtn == Test_SubOpt_TermProcess)
			    TN_Receive_SM[ii][jj].actionRtn = SubOpt_TermProcess;
			else if (TN_Receive_SM[ii][jj].actionRtn == Test_Process_Suboption)
			    TN_Receive_SM[ii][jj].actionRtn = Process_Suboption;
		    }
		}
    }

    /*
     * Return the results of this test back to the caller.
     */
    return(retVal);
}

int main(void)
{
    bool retVal = true;

    printf("\nDECaxp Telnet Testing...\n\n");
    retVal = test_options_StateMachine();
#if 0
    if (retVal == true)
	AXP_Telnet_Main();
#endif
    if (retVal == true)
	printf("All Tests Successful!\n");
    else
	printf("At Least One Test Failed.\n");
    return(0);
}
