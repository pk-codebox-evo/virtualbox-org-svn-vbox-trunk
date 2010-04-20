/* $Id$ */
/** @file
 * IPRT - Mutex Semaphores, Ring-0 Driver, NT.
 */

/*
 * Copyright (C) 2006-2010 Sun Microsystems, Inc.
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 *
 * The contents of this file may alternatively be used under the terms
 * of the Common Development and Distribution License Version 1.0
 * (CDDL) only, as it comes in the "COPYING.CDDL" file of the
 * VirtualBox OSE distribution, in which case the provisions of the
 * CDDL are applicable instead of those of the GPL.
 *
 * You may elect to license modified versions of this file under the
 * terms and conditions of either the GPL or the CDDL or both.
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa
 * Clara, CA 95054 USA or visit http://www.sun.com if you need
 * additional information or have any questions.
 */



/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#include "the-nt-kernel.h"
#include <iprt/semaphore.h>
#include <iprt/alloc.h>
#include <iprt/assert.h>
#include <iprt/asm.h>
#include <iprt/err.h>

#include "internal/magics.h"


/*******************************************************************************
*   Structures and Typedefs                                                    *
*******************************************************************************/
/**
 * NT mutex semaphore.
 */
typedef struct RTSEMMUTEXINTERNAL
{
    /** Magic value (RTSEMMUTEX_MAGIC). */
    uint32_t volatile   u32Magic;
#ifdef RT_USE_FAST_MUTEX
    /** The fast mutex object. */
    FAST_MUTEX          Mutex;
#else
    /** The NT Mutex object. */
    KMUTEX              Mutex;
#endif
} RTSEMMUTEXINTERNAL, *PRTSEMMUTEXINTERNAL;



RTDECL(int)  RTSemMutexCreate(PRTSEMMUTEX phMutexSem)
{
    return RTSemMutexCreateEx(phMutexSem, 0 /*fFlags*/, NIL_RTLOCKVALCLASS, RTLOCKVAL_SUB_CLASS_NONE, NULL);
}


RTDECL(int) RTSemMutexCreateEx(PRTSEMMUTEX phMutexSem, uint32_t fFlags,
                               RTLOCKVALCLASS hClass, uint32_t uSubClass, const char *pszNameFmt, ...)
{
    AssertReturn(!(fFlags & ~RTSEMMUTEX_FLAGS_NO_LOCK_VAL), VERR_INVALID_PARAMETER);

    AssertCompile(sizeof(RTSEMMUTEXINTERNAL) > sizeof(void *));
    PRTSEMMUTEXINTERNAL pThis = (PRTSEMMUTEXINTERNAL)RTMemAlloc(sizeof(*pThis));
    if (!pThis)
        return VERR_NO_MEMORY;

    pThis->u32Magic = RTSEMMUTEX_MAGIC;
#ifdef RT_USE_FAST_MUTEX
    ExInitializeFastMutex(&pThis->Mutex);
#else
    KeInitializeMutex(&pThis->Mutex, 0);
#endif

    *phMutexSem = pThis;
    return VINF_SUCCESS;
}


RTDECL(int) RTSemMutexDestroy(RTSEMMUTEX hMutexSem)
{
    /*
     * Validate input.
     */
    PRTSEMMUTEXINTERNAL pThis = (PRTSEMMUTEXINTERNAL)hMutexSem;
    if (pThis == NIL_RTSEMMUTEX)
        return VINF_SUCCESS;
    AssertPtrReturn(pThis, VERR_INVALID_HANDLE);
    AssertReturn(pThis->u32Magic == RTSEMMUTEX_MAGIC, VERR_INVALID_HANDLE);

    /*
     * Invalidate it and signal the object just in case.
     */
    AssertReturn(ASMAtomicCmpXchgU32(&pThis->u32Magic, RTSEMMUTEX_MAGIC_DEAD, RTSEMMUTEX_MAGIC), VERR_INVALID_HANDLE);
    RTMemFree(pThis);
    return VINF_SUCCESS;
}


/**
 * Internal worker for RTSemMutexRequest and RTSemMutexRequestNoResume
 *
 * @returns IPRT status code.
 * @param   hMutexSem            The mutex handle.
 * @param   cMillies            The timeout.
 * @param   fInterruptible      Whether it's interruptible
 *                              (RTSemMutexRequestNoResume) or not
 *                              (RTSemMutexRequest).
 */
static int rtSemMutexRequest(RTSEMMUTEX hMutexSem, RTMSINTERVAL cMillies, BOOLEAN fInterruptible)
{
    /*
     * Validate input.
     */
    PRTSEMMUTEXINTERNAL pThis = (PRTSEMMUTEXINTERNAL)hMutexSem;
    AssertPtrReturn(pThis, VERR_INVALID_HANDLE);
    AssertReturn(pThis->u32Magic == RTSEMMUTEX_MAGIC, VERR_INVALID_HANDLE);

    /*
     * Get the mutex.
     */
#ifdef RT_USE_FAST_MUTEX
    AssertMsg(cMillies == RT_INDEFINITE_WAIT, ("timeouts are not supported when using fast mutexes!\n"));
    ExAcquireFastMutex(&pThis->Mutex);
#else  /* !RT_USE_FAST_MUTEX */
    NTSTATUS rcNt;
    if (cMillies == RT_INDEFINITE_WAIT)
        rcNt = KeWaitForSingleObject(&pThis->Mutex, Executive, KernelMode, fInterruptible, NULL);
    else
    {
        LARGE_INTEGER Timeout;
        Timeout.QuadPart = -(int64_t)cMillies * 10000;
        rcNt = KeWaitForSingleObject(&pThis->Mutex, Executive, KernelMode, fInterruptible, &Timeout);
    }
    switch (rcNt)
    {
        case STATUS_SUCCESS:
            if (pThis->u32Magic == RTSEMMUTEX_MAGIC)
                return VINF_SUCCESS;
            return VERR_SEM_DESTROYED;

        case STATUS_ALERTED:
        case STATUS_USER_APC:
            Assert(fInterruptible);
            return VERR_INTERRUPTED;

        case STATUS_TIMEOUT:
            return VERR_TIMEOUT;

        default:
            AssertMsgFailed(("pThis->u32Magic=%RX32 pThis=%p: wait returned %lx!\n",
                             pThis->u32Magic, pThis, (long)rcNt));
            return VERR_INTERNAL_ERROR;
    }
#endif /* !RT_USE_FAST_MUTEX */
    return VINF_SUCCESS;
}


#undef RTSemMutexRequest
RTDECL(int) RTSemMutexRequest(RTSEMMUTEX hMutexSem, RTMSINTERVAL cMillies)
{
    return rtSemMutexRequest(hMutexSem, cMillies, FALSE /*fInterruptible*/);
}


RTDECL(int) RTSemMutexRequestDebug(RTSEMMUTEX hMutexSem, RTMSINTERVAL cMillies, RTHCUINTPTR uId, RT_SRC_POS_DECL)
{
    return RTSemMutexRequest(hMutexSem, cMillies);
}


#undef RTSemMutexRequestNoResume
RTDECL(int) RTSemMutexRequestNoResume(RTSEMMUTEX hMutexSem, RTMSINTERVAL cMillies)
{
    return rtSemMutexRequest(hMutexSem, cMillies, TRUE /*fInterruptible*/);
}


RTDECL(int) RTSemMutexRequestNoResumeDebug(RTSEMMUTEX hMutexSem, RTMSINTERVAL cMillies, RTHCUINTPTR uId, RT_SRC_POS_DECL)
{
    return RTSemMutexRequestNoResume(hMutexSem, cMillies);
}


RTDECL(int) RTSemMutexRelease(RTSEMMUTEX hMutexSem)
{
    /*
     * Validate input.
     */
    PRTSEMMUTEXINTERNAL pThis = (PRTSEMMUTEXINTERNAL)hMutexSem;
    AssertPtrReturn(pThis, VERR_INVALID_HANDLE);
    AssertReturn(pThis->u32Magic == RTSEMMUTEX_MAGIC, VERR_INVALID_HANDLE);

    /*
     * Release the mutex.
     */
#ifdef RT_USE_FAST_MUTEX
    ExReleaseFastMutex(&pThis->Mutex);
#else
    KeReleaseMutex(&pThis->Mutex, FALSE /*Wait*/);
#endif
    return VINF_SUCCESS;
}


RTDECL(bool) RTSemMutexIsOwned(RTSEMMUTEX hMutexSem)
{
    /*
     * Validate.
     */
    RTSEMMUTEXINTERNAL *pThis = hMutexSem;
    AssertPtrReturn(pThis, false);
    AssertReturn(pThis->u32Magic == RTSEMMUTEX_MAGIC, false);

#ifdef RT_USE_FAST_MUTEX
    return pThis->Mutex && pThis->Mutex->Owner != NULL;
#else
    return KeReadStateMutex(&pThis->Mutex) == 1;
#endif
}

