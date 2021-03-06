/* $Id$ */
/** @file
 * VBoxCredProvUtils - Misc. utility functions for VBoxCredProv.
 */

/*
 * Copyright (C) 2012 Oracle Corporation
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 */


/*********************************************************************************************************************************
*   Header Files                                                                                                                 *
*********************************************************************************************************************************/
#include <Windows.h>
#include <iprt/string.h>
#include <VBox/log.h>
#include <VBox/VBoxGuestLib.h>


/*********************************************************************************************************************************
*   Global Variables                                                                                                             *
*********************************************************************************************************************************/
/** Verbosity flag for guest logging. */
DWORD g_dwVerbosity = 0;


/**
 * Displays a verbose message.
 *
 * @param   iLevel      Minimum log level required to display this message.
 * @param   pszFormat   The message text.
 * @param   ...         Format arguments.
 */
void VBoxCredProvVerbose(DWORD dwLevel, const char *pszFormat, ...)
{
    if (dwLevel <= g_dwVerbosity)
    {
        va_list args;
        va_start(args, pszFormat);
        char *psz = NULL;
        RTStrAPrintfV(&psz, pszFormat, args);
        va_end(args);

        AssertPtr(psz);
        LogRel(("%s", psz));

        RTStrFree(psz);
    }
}


/**
 * Reports VBoxGINA's status to the host (treated as a guest facility).
 *
 * @return  IPRT status code.
 * @param   enmStatus               Status to report to the host.
 */
int VBoxCredProvReportStatus(VBoxGuestFacilityStatus enmStatus)
{
    VBoxCredProvVerbose(0, "VBoxCredProv: reporting status %d\n", enmStatus);

    int rc = VbglR3AutoLogonReportStatus(enmStatus);
    if (RT_FAILURE(rc))
        VBoxCredProvVerbose(0, "VBoxCredProv: failed to report status %d, rc=%Rrc\n", enmStatus, rc);
    return rc;
}

