/* $Id$ */
/** @file
 * DevIchHdaCodec - VBox ICH Intel HD Audio Codec.
 */

/*
 * Copyright (C) 2006-2016 Oracle Corporation
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 */

#ifndef DEV_HDA_CODEC_H
#define DEV_HDA_CODEC_H

/** The ICH HDA (Intel) controller. */
typedef struct HDASTATE *PHDASTATE;
/** The ICH HDA (Intel) codec state. */
typedef struct HDACODEC *PHDACODEC;
/** The HDA host driver backend. */
typedef struct HDADRIVER *PHDADRIVER;
typedef struct PDMIAUDIOCONNECTOR *PPDMIAUDIOCONNECTOR;
typedef struct PDMAUDIOGSTSTRMOUT *PPDMAUDIOGSTSTRMOUT;
typedef struct PDMAUDIOGSTSTRMIN  *PPDMAUDIOGSTSTRMIN;

/**
 * Verb processor method.
 */
typedef DECLCALLBACK(int) FNHDACODECVERBPROCESSOR(PHDACODEC pThis, uint32_t cmd, uint64_t *pResp);
typedef FNHDACODECVERBPROCESSOR *PFNHDACODECVERBPROCESSOR;
typedef FNHDACODECVERBPROCESSOR **PPFNHDACODECVERBPROCESSOR;

/* PRM 5.3.1 */
#define CODEC_RESPONSE_UNSOLICITED RT_BIT_64(34)

typedef struct CODECVERB
{
    /** Verb. */
    uint32_t                 verb;
    /** Verb mask. */
    uint32_t                 mask;
    /** Function pointer for implementation callback. */
    PFNHDACODECVERBPROCESSOR pfn;
    /** Friendly name, for debugging. */
    const char              *pszName;
} CODECVERB;

union CODECNODE;
typedef union CODECNODE CODECNODE, *PCODECNODE;

/**
 * Structure for keeping a HDA codec state.
 */
typedef struct HDACODEC
{
    uint16_t                id;
    uint16_t                u16VendorId;
    uint16_t                u16DeviceId;
    uint8_t                 u8BSKU;
    uint8_t                 u8AssemblyId;
    /** List of assigned HDA drivers to this codec.
     * A driver only can be assigned to one codec at a time. */
    RTLISTANCHOR            lstDrv;

    CODECVERB const        *paVerbs;
    size_t                  cVerbs;

    PCODECNODE              paNodes;
    /** Pointer to HDA state (controller) this
     *  codec is assigned to. */
    PHDASTATE               pHDAState;
    bool                    fInReset;

    const uint8_t           cTotalNodes;
    const uint8_t          *au8Ports;
    const uint8_t          *au8Dacs;
    const uint8_t          *au8AdcVols;
    const uint8_t          *au8Adcs;
    const uint8_t          *au8AdcMuxs;
    const uint8_t          *au8Pcbeeps;
    const uint8_t          *au8SpdifIns;
    const uint8_t          *au8SpdifOuts;
    const uint8_t          *au8DigInPins;
    const uint8_t          *au8DigOutPins;
    const uint8_t          *au8Cds;
    const uint8_t          *au8VolKnobs;
    const uint8_t          *au8Reserveds;
    const uint8_t           u8AdcVolsLineIn;
    const uint8_t           u8DacLineOut;

    /** Callbacks to the HDA controller, mostly used for multiplexing to the various host backends. */
    DECLR3CALLBACKMEMBER(int, pfnMixerAddStream, (PHDASTATE pThis, PDMAUDIOMIXERCTL enmMixerCtl, PPDMAUDIOSTREAMCFG pCfg));
    DECLR3CALLBACKMEMBER(int, pfnMixerRemoveStream, (PHDASTATE pThis, PDMAUDIOMIXERCTL enmMixerCtl));
    DECLR3CALLBACKMEMBER(int, pfnMixerSetStream, (PHDASTATE pThis, PDMAUDIOMIXERCTL enmMixerCtl, uint8_t uSD, uint8_t uChannel));
    DECLR3CALLBACKMEMBER(int, pfnMixerSetVolume, (PHDASTATE pThis, PDMAUDIOMIXERCTL enmMixerCtl, PPDMAUDIOVOLUME pVol));
    /** Callbacks by codec implementation. */
    DECLR3CALLBACKMEMBER(int, pfnLookup, (PHDACODEC pThis, uint32_t uVerb, uint64_t *puResp));
    DECLR3CALLBACKMEMBER(int, pfnReset, (PHDACODEC pThis));
    DECLR3CALLBACKMEMBER(int, pfnCodecNodeReset, (PHDACODEC pThis, uint8_t, PCODECNODE));
    /** These callbacks are set by codec implementation to answer debugger requests. */
    DECLR3CALLBACKMEMBER(void, pfnDbgListNodes, (PHDACODEC pThis, PCDBGFINFOHLP pHlp, const char *pszArgs));
    DECLR3CALLBACKMEMBER(void, pfnDbgSelector, (PHDACODEC pThis, PCDBGFINFOHLP pHlp, const char *pszArgs));
} HDACODEC;

int hdaCodecConstruct(PPDMDEVINS pDevIns, PHDACODEC pThis, uint16_t uLUN, PCFGMNODE pCfg);
void hdaCodecDestruct(PHDACODEC pThis);
void hdaCodecPowerOff(PHDACODEC pThis);
int hdaCodecSaveState(PHDACODEC pThis, PSSMHANDLE pSSM);
int hdaCodecLoadState(PHDACODEC pThis, PSSMHANDLE pSSM, uint32_t uVersion);
int hdaCodecAddStream(PHDACODEC pThis, PDMAUDIOMIXERCTL enmMixerCtl, PPDMAUDIOSTREAMCFG pCfg);
int hdaCodecRemoveStream(PHDACODEC pThis, PDMAUDIOMIXERCTL enmMixerCtl);

#define HDA_SSM_VERSION   6
/** Introduced dynamic number of streams + stream identifiers for serialization.
 *  Bug: Did not save the BDLE states correctly.
 *  Those will be skipped on load then. */
#define HDA_SSM_VERSION_5 5
/** Since this version the number of MMIO registers can be flexible. */
#define HDA_SSM_VERSION_4 4
#define HDA_SSM_VERSION_3 3
#define HDA_SSM_VERSION_2 2
#define HDA_SSM_VERSION_1 1

#endif /* DEV_HDA_CODEC_H */

