/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-03
 * Description : LCMS2 wrapper
 *
 * Copyright (C) 2012      by Francesco Riosa <francesco+kde at pnpitalia dot it>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIGIKAM_LCMS_H
#define DIGIKAM_LCMS_H

#include "digikam_export.h"

#if defined (__MINGW32__)
#  define CMS_IS_WINDOWS_ 1
#else
#   ifndef CMS_DLL
#       define CMS_DLL
#   endif
#endif

// Turn off the specific compiler warnings with LCMS header.

#if defined(__APPLE__) && defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wundef"
#endif

#include <lcms2.h>

#if defined(__APPLE__) && defined(__clang__)
#    pragma clang diagnostic pop
#endif

#define LCMS_DESC_MAX                    512

#if !defined FAR
#  define FAR
#endif

#define LCMS_ERROR_SHOW                  1
#define cmsFLAGS_NOTPRECALC              0x0100
#define cmsFLAGS_WHITEBLACKCOMPENSATION  0x2000

typedef int            LCMSBOOL;
typedef unsigned char  BYTE,     *LPBYTE;
typedef unsigned short WORD,     *LPWORD;
typedef unsigned long  DWORD,    *LPDWORD;
typedef void*          LPVOID;

// Colorspaces
typedef cmsCIEXYZ FAR* LPcmsCIEXYZ;
typedef cmsCIExyY FAR* LPcmsCIExyY;
typedef cmsCIELab FAR* LPcmsCIELab;

typedef void* cmsHPROFILE;             // Opaque typedefs to hide internals

// these have changed from previous definitions
typedef cmsCIEXYZTRIPLE MAT3;
typedef cmsCIEXYZTRIPLE FAR* LPMAT3;

/* profileClass enumerations */
typedef enum
{
    icSigInputClass                     = 0x73636E72L,  /* 'scnr' */
    icSigDisplayClass                   = 0x6D6E7472L,  /* 'mntr' */
    icSigOutputClass                    = 0x70727472L,  /* 'prtr' */
    icSigLinkClass                      = 0x6C696E6BL,  /* 'link' */
    icSigAbstractClass                  = 0x61627374L,  /* 'abst' */
    icSigColorSpaceClass                = 0x73706163L,  /* 'spac' */
    icSigNamedColorClass                = 0x6e6d636cL,  /* 'nmcl' */
    icMaxEnumClass                      = 0xFFFFFFFFL
} icProfileClassSignature;

/*
 * Color Space Signatures
 * Note that only icSigXYZData and icSigLabData are valid
 * Profile Connection Spaces (PCSs)
 */
typedef enum
{
    icSigXYZData                        = 0x58595A20L,  /* 'XYZ ' */
    icSigLabData                        = 0x4C616220L,  /* 'Lab ' */
    icSigLuvData                        = 0x4C757620L,  /* 'Luv ' */
    icSigYCbCrData                      = 0x59436272L,  /* 'YCbr' */
    icSigYxyData                        = 0x59787920L,  /* 'Yxy ' */
    icSigRgbData                        = 0x52474220L,  /* 'RGB ' */
    icSigGrayData                       = 0x47524159L,  /* 'GRAY' */
    icSigHsvData                        = 0x48535620L,  /* 'HSV ' */
    icSigHlsData                        = 0x484C5320L,  /* 'HLS ' */
    icSigCmykData                       = 0x434D594BL,  /* 'CMYK' */
    icSigCmyData                        = 0x434D5920L,  /* 'CMY ' */
    icSig2colorData                     = 0x32434C52L,  /* '2CLR' */
    icSig3colorData                     = 0x33434C52L,  /* '3CLR' */
    icSig4colorData                     = 0x34434C52L,  /* '4CLR' */
    icSig5colorData                     = 0x35434C52L,  /* '5CLR' */
    icSig6colorData                     = 0x36434C52L,  /* '6CLR' */
    icSig7colorData                     = 0x37434C52L,  /* '7CLR' */
    icSig8colorData                     = 0x38434C52L,  /* '8CLR' */
    icSig9colorData                     = 0x39434C52L,  /* '9CLR' */
    icSig10colorData                    = 0x41434C52L,  /* 'ACLR' */
    icSig11colorData                    = 0x42434C52L,  /* 'BCLR' */
    icSig12colorData                    = 0x43434C52L,  /* 'CCLR' */
    icSig13colorData                    = 0x44434C52L,  /* 'DCLR' */
    icSig14colorData                    = 0x45434C52L,  /* 'ECLR' */
    icSig15colorData                    = 0x46434C52L,  /* 'FCLR' */
    icMaxEnumData                       = 0xFFFFFFFFL
} icColorSpaceSignature;


/*------------------------------------------------------------------------*/
/* public tags and sizes */
typedef enum
{
    icSigAToB0Tag                       = 0x41324230L,  /* 'A2B0' */
    icSigAToB1Tag                       = 0x41324231L,  /* 'A2B1' */
    icSigAToB2Tag                       = 0x41324232L,  /* 'A2B2' */
    icSigBlueColorantTag                = 0x6258595AL,  /* 'bXYZ' */
    icSigBlueTRCTag                     = 0x62545243L,  /* 'bTRC' */
    icSigBToA0Tag                       = 0x42324130L,  /* 'B2A0' */
    icSigBToA1Tag                       = 0x42324131L,  /* 'B2A1' */
    icSigBToA2Tag                       = 0x42324132L,  /* 'B2A2' */
    icSigCalibrationDateTimeTag         = 0x63616C74L,  /* 'calt' */
    icSigCharTargetTag                  = 0x74617267L,  /* 'targ' */
    icSigCopyrightTag                   = 0x63707274L,  /* 'cprt' */
    icSigCrdInfoTag                     = 0x63726469L,  /* 'crdi' */
    icSigDeviceMfgDescTag               = 0x646D6E64L,  /* 'dmnd' */
    icSigDeviceModelDescTag             = 0x646D6464L,  /* 'dmdd' */
    icSigGamutTag                       = 0x67616D74L,  /* 'gamt ' */
    icSigGrayTRCTag                     = 0x6b545243L,  /* 'kTRC' */
    icSigGreenColorantTag               = 0x6758595AL,  /* 'gXYZ' */
    icSigGreenTRCTag                    = 0x67545243L,  /* 'gTRC' */
    icSigLuminanceTag                   = 0x6C756d69L,  /* 'lumi' */
    icSigMeasurementTag                 = 0x6D656173L,  /* 'meas' */
    icSigMediaBlackPointTag             = 0x626B7074L,  /* 'bkpt' */
    icSigMediaWhitePointTag             = 0x77747074L,  /* 'wtpt' */
    icSigNamedColorTag                  = 0x6E636f6CL,  /* 'ncol'
                                                         * OBSOLETE, use ncl2 */
    icSigNamedColor2Tag                 = 0x6E636C32L,  /* 'ncl2' */
    icSigPreview0Tag                    = 0x70726530L,  /* 'pre0' */
    icSigPreview1Tag                    = 0x70726531L,  /* 'pre1' */
    icSigPreview2Tag                    = 0x70726532L,  /* 'pre2' */
    icSigProfileDescriptionTag          = 0x64657363L,  /* 'desc' */
    icSigProfileSequenceDescTag         = 0x70736571L,  /* 'pseq' */
    icSigPs2CRD0Tag                     = 0x70736430L,  /* 'psd0' */
    icSigPs2CRD1Tag                     = 0x70736431L,  /* 'psd1' */
    icSigPs2CRD2Tag                     = 0x70736432L,  /* 'psd2' */
    icSigPs2CRD3Tag                     = 0x70736433L,  /* 'psd3' */
    icSigPs2CSATag                      = 0x70733273L,  /* 'ps2s' */
    icSigPs2RenderingIntentTag          = 0x70733269L,  /* 'ps2i' */
    icSigRedColorantTag                 = 0x7258595AL,  /* 'rXYZ' */
    icSigRedTRCTag                      = 0x72545243L,  /* 'rTRC' */
    icSigScreeningDescTag               = 0x73637264L,  /* 'scrd' */
    icSigScreeningTag                   = 0x7363726EL,  /* 'scrn' */
    icSigTechnologyTag                  = 0x74656368L,  /* 'tech' */
    icSigUcrBgTag                       = 0x62666420L,  /* 'bfd ' */
    icSigViewingCondDescTag             = 0x76756564L,  /* 'vued' */
    icSigViewingConditionsTag           = 0x76696577L,  /* 'view' */
    icMaxEnumTag                        = 0xFFFFFFFFL
} icTagSignature;

DIGIKAM_EXPORT int                     dkCmsErrorAction(int nAction);

DIGIKAM_EXPORT DWORD                   dkCmsGetProfileICCversion(cmsHPROFILE hProfile);

DIGIKAM_EXPORT void                    dkCmsSetAlarmCodes(int r, int g, int b);

DIGIKAM_EXPORT QString                 dkCmsTakeProductName(cmsHPROFILE hProfile);

DIGIKAM_EXPORT QString                 dkCmsTakeProductDesc(cmsHPROFILE hProfile);

DIGIKAM_EXPORT QString                 dkCmsTakeProductInfo(cmsHPROFILE hProfile);

DIGIKAM_EXPORT QString                 dkCmsTakeManufacturer(cmsHPROFILE hProfile);

DIGIKAM_EXPORT LCMSBOOL                dkCmsTakeMediaWhitePoint(LPcmsCIEXYZ Dest, cmsHPROFILE hProfile);

DIGIKAM_EXPORT QString                 dkCmsTakeModel(cmsHPROFILE hProfile);

DIGIKAM_EXPORT QString                 dkCmsTakeCopyright(cmsHPROFILE hProfile);

DIGIKAM_EXPORT DWORD                   dkCmsTakeHeaderFlags(cmsHPROFILE hProfile);

DIGIKAM_EXPORT const BYTE*             dkCmsTakeProfileID(cmsHPROFILE hProfile);

DIGIKAM_EXPORT LCMSBOOL                dkCmsIsTag(cmsHPROFILE hProfile, icTagSignature sig);

DIGIKAM_EXPORT int                     dkCmsTakeRenderingIntent(cmsHPROFILE hProfile);

DIGIKAM_EXPORT LCMSBOOL                dkCmsAdaptMatrixFromD50(LPMAT3 r, LPcmsCIExyY DestWhitePt);

DIGIKAM_EXPORT LCMSBOOL                dkCmsReadICCMatrixRGB2XYZ(LPMAT3 r, cmsHPROFILE hProfile);

DIGIKAM_EXPORT cmsHPROFILE             dkCmsOpenProfileFromMem(LPVOID MemPtr, DWORD dwSize);

DIGIKAM_EXPORT icProfileClassSignature dkCmsGetDeviceClass(cmsHPROFILE hProfile);

DIGIKAM_EXPORT LCMSBOOL                dkCmsCloseProfile(cmsHPROFILE hProfile);

DIGIKAM_EXPORT cmsHTRANSFORM           dkCmsCreateProofingTransform(cmsHPROFILE Input,
                                                                         DWORD InputFormat,
                                                                         cmsHPROFILE Output,
                                                                         DWORD OutputFormat,
                                                                         cmsHPROFILE Proofing,
                                                                         int Intent,
                                                                         int ProofingIntent,
                                                                         DWORD dwFlags);

DIGIKAM_EXPORT cmsHTRANSFORM           dkCmsCreateTransform(cmsHPROFILE Input,
                                                                 DWORD InputFormat,
                                                                 cmsHPROFILE Output,
                                                                 DWORD OutputFormat,
                                                                 int Intent,
                                                                 DWORD dwFlags);

DIGIKAM_EXPORT cmsHPROFILE             dkCmsCreateXYZProfile();

DIGIKAM_EXPORT cmsHPROFILE             dkCmsCreate_sRGBProfile();

DIGIKAM_EXPORT void                    dkCmsDeleteTransform(cmsHTRANSFORM hTransform);

DIGIKAM_EXPORT double                  dkCmsDeltaE(LPcmsCIELab Lab1, LPcmsCIELab Lab2);

DIGIKAM_EXPORT void                    dkCmsDoTransform(cmsHTRANSFORM Transform,
                                                             LPVOID InputBuffer,
                                                             LPVOID OutputBuffer,
                                                             unsigned int Size);

DIGIKAM_EXPORT void                    dkCmsFloat2XYZEncoded(WORD XYZ[3], const cmsCIEXYZ* const fXYZ);

DIGIKAM_EXPORT icColorSpaceSignature   dkCmsGetColorSpace(cmsHPROFILE hProfile);

DIGIKAM_EXPORT icColorSpaceSignature   dkCmsGetPCS(cmsHPROFILE hProfile);

DIGIKAM_EXPORT LCMSBOOL                dkCmsIsTag(cmsHPROFILE hProfile, icTagSignature sig);

DIGIKAM_EXPORT cmsHPROFILE             dkCmsOpenProfileFromFile(const char* const ICCProfile, const char* const sAccess);

DIGIKAM_EXPORT void                    dkCmsXYZ2xyY(LPcmsCIExyY Dest, const cmsCIEXYZ* const Source);

#endif // DIGIKAM_LCMS_H
