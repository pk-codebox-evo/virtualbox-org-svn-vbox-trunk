/* $Id$ */
/** @file
 * Windows USB R0 Tooling.
 */

/*
 * Copyright (C) 2011-2015 Oracle Corporation
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 */
#ifndef ___VBoxUsbTool_h___
#define ___VBoxUsbTool_h___

#include "VBoxDrvTool.h"

RT_C_DECLS_BEGIN
#pragma warning( disable : 4200 )
#include <usbdi.h>
#pragma warning( default : 4200 )
#include <usbdlib.h>

RT_C_DECLS_END

#if 0
/* enable this in case we include this in a dll*/
# ifdef IN_VBOXUSBTOOL
#  define VBOXUSBTOOL_DECL(a_Type) DECLEXPORT(a_Type)
# else
#  define VBOXUSBTOOL_DECL(a_Type) DECLIMPORT(a_Type)
# endif
#else
/*enable this in case we include this in a static lib*/
# define VBOXUSBTOOL_DECL(a_Type) a_Type VBOXCALL
#endif


RT_C_DECLS_BEGIN

VBOXUSBTOOL_DECL(PURB) VBoxUsbToolUrbAlloc(USHORT u16Function, USHORT cbSize);
VBOXUSBTOOL_DECL(PURB) VBoxUsbToolUrbAllocZ(USHORT u16Function, USHORT cbSize);
VBOXUSBTOOL_DECL(PURB) VBoxUsbToolUrbReinit(PURB pUrb, USHORT cbSize, USHORT u16Function);
VBOXUSBTOOL_DECL(VOID) VBoxUsbToolUrbFree(PURB pUrb);
VBOXUSBTOOL_DECL(NTSTATUS) VBoxUsbToolUrbPost(PDEVICE_OBJECT pDevObj, PURB pUrb, ULONG dwTimeoutMs);
VBOXUSBTOOL_DECL(NTSTATUS) VBoxUsbToolGetDescriptor(PDEVICE_OBJECT pDevObj, void *pvBuffer, int cbBuffer, int Type, int iIndex, int LangId, ULONG dwTimeoutMs);
VBOXUSBTOOL_DECL(NTSTATUS) VBoxUsbToolGetStringDescriptor(PDEVICE_OBJECT pDevObj, char *pszResult, ULONG cbResult, int iIndex, int LangId, ULONG dwTimeoutMs);
VBOXUSBTOOL_DECL(NTSTATUS) VBoxUsbToolGetLangID(PDEVICE_OBJECT pDevObj, int *pLangId, ULONG dwTimeoutMs);
VBOXUSBTOOL_DECL(NTSTATUS) VBoxUsbToolGetDeviceSpeed(PDEVICE_OBJECT pDevObj, BOOLEAN *pbIsHigh);
VBOXUSBTOOL_DECL(NTSTATUS) VBoxUsbToolPipeClear(PDEVICE_OBJECT pDevObj, HANDLE hPipe, bool fReset);
VBOXUSBTOOL_DECL(NTSTATUS) VBoxUsbToolCurrentFrame(PDEVICE_OBJECT pDevObj, PIRP pIrp, PULONG piFrame);
VBOXUSBTOOL_DECL(NTSTATUS) VBoxUsbToolDevUnconfigure(PDEVICE_OBJECT pDevObj);
VBOXUSBTOOL_DECL(NTSTATUS) VBoxUsbToolIoInternalCtlSendAsync(PDEVICE_OBJECT pDevObj, ULONG uCtl, void *pvArg1, void *pvArg2, PKEVENT pEvent, PIO_STATUS_BLOCK pIoStatus);
VBOXUSBTOOL_DECL(NTSTATUS) VBoxUsbToolIoInternalCtlSendSync(PDEVICE_OBJECT pDevObj, ULONG uCtl, void *pvArg1, void *pvArg2);
VBOXUSBTOOL_DECL(PIRP) VBoxUsbToolIoBuildAsyncInternalCtl(PDEVICE_OBJECT pDevObj, ULONG uCtl, void *pvArg1, void *pvArg2);
VBOXUSBTOOL_DECL(NTSTATUS) VBoxUsbToolIoInternalCtlSendSyncWithTimeout(PDEVICE_OBJECT pDevObj, ULONG uCtl, void *pvArg1, void *pvArg2, ULONG dwTimeoutMs);
VBOXUSBTOOL_DECL(VOID) VBoxUsbToolStringDescriptorToUnicodeString(PUSB_STRING_DESCRIPTOR pDr, PUNICODE_STRING pUnicode);


RT_C_DECLS_END

#endif /* #ifndef ___VBoxUsbTool_h___ */
