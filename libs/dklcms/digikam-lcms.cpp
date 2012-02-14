/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-03
 * Description : wrapper to help on lcms2 porting
 *
 * Copyright (C) 2012 by Francesco Riosa <francesco+kde dot pnpitalia dot it>
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

#include <QtCore/QString>

#include <config-digikam.h>

#if defined(USE_LCMS_VERSION_2000)

#include <lcms2.h>

#include "digikam-lcms.h"


LCMSAPI int    LCMSEXPORT dkCmsErrorAction(int nAction)
{
    // TODO: Where is error logging?
    return 0;
}

LCMSAPI DWORD  LCMSEXPORT dkCmsGetProfileICCversion(cmsHPROFILE hProfile)
{
    return (DWORD) cmsGetEncodedICCversion(hProfile);
}

LCMSEXPORT void dkCmsSetAlarmCodes(int r, int g, int b)
{
    cmsUInt16Number NewAlarm[cmsMAXCHANNELS];
    NewAlarm[0] = (cmsUInt16Number)r;
    NewAlarm[1] = (cmsUInt16Number)g;
    NewAlarm[2] = (cmsUInt16Number)b;
    cmsSetAlarmCodes(NewAlarm);
}

LCMSAPI QString        LCMSEXPORT dkCmsTakeProductName(cmsHPROFILE hProfile)
{
    char buffer[1024];
    const cmsMLU* mlu = (const cmsMLU*)cmsReadTag(hProfile, cmsSigCrdInfoTag);
    if (mlu == NULL) return QString();
    cmsMLUgetASCII(mlu, "PS", "nm", buffer, 1024);
    return QString(buffer);
}

LCMSAPI const char*    LCMSEXPORT dkCmsTakeProductDesc(cmsHPROFILE hProfile)
{
    // TODO: What I'm supposed to use here??
    static char ret[1]; ret[0] = '\0'; return ret;
}

LCMSAPI QString        LCMSEXPORT dkCmsTakeProductInfo(cmsHPROFILE hProfile)
{
    char buffer[1024];
    cmsGetProfileInfoASCII(hProfile, cmsInfoDescription, "en", "US", buffer, 1024);
    return QString(buffer);
}

LCMSAPI QString        LCMSEXPORT dkCmsTakeManufacturer(cmsHPROFILE hProfile)
{
    char buffer[1024];
    cmsGetProfileInfoASCII(hProfile, cmsInfoManufacturer, "en", "US", buffer, 1024);
    return QString(buffer);
}

LCMSAPI LCMSBOOL      LCMSEXPORT dkCmsTakeMediaWhitePoint(LPcmsCIEXYZ Dest, cmsHPROFILE hProfile)
{
    Dest = (LPcmsCIEXYZ)cmsReadTag(hProfile, cmsSigMediaWhitePointTag);
    return (Dest != NULL);
}

LCMSAPI QString       LCMSEXPORT dkCmsTakeModel(cmsHPROFILE hProfile)
{
    char buffer[1024];
    const cmsMLU* mlu = (const cmsMLU*)cmsReadTag(hProfile, cmsSigDeviceModelDescTag);
    if (mlu == NULL) return QString();
    cmsMLUgetASCII(mlu, "en", "US", buffer, 1024);
    return QString(buffer);
}

LCMSAPI QString        LCMSEXPORT dkCmsTakeCopyright(cmsHPROFILE hProfile)
{
    char buffer[1024];
    const cmsMLU* mlu = (const cmsMLU*)cmsReadTag(hProfile, cmsSigCopyrightTag);
    if (mlu == NULL) return QString();
    cmsMLUgetASCII(mlu, "en", "US", buffer, 1024);
    return QString(buffer);
}


LCMSAPI DWORD         LCMSEXPORT dkCmsTakeHeaderFlags(cmsHPROFILE hProfile)
{
    return (DWORD) cmsGetHeaderFlags(hProfile);
}

LCMSAPI const BYTE*   LCMSEXPORT dkCmsTakeProfileID(cmsHPROFILE hProfile)
{
    cmsUInt8Number* ProfileID = new cmsUInt8Number();
    cmsGetHeaderProfileID(hProfile, ProfileID);
    return (BYTE*) ProfileID;
}

LCMSAPI int           LCMSEXPORT dkCmsTakeRenderingIntent(cmsHPROFILE hProfile)
{
    return (int) cmsGetHeaderRenderingIntent(hProfile);
}

// White Point & Primary chromas handling
// Returns the final chrmatic adaptation from illuminant FromIll to Illuminant ToIll
// The cone matrix can be specified in ConeMatrix. 
// If NULL, assuming D50 source. White point is given in xyY
LCMSBOOL dkCmsAdaptMatrixFromD50(LPMAT3 r, LPcmsCIExyY DestWhitePt)
{
    // TODO: all based on private stuff, need to understand what digikam do in cietonguewidget with dkCmsAdaptMatrixFromD50
    return TRUE;
}

cmsBool GetProfileRGBPrimaries(cmsHPROFILE hProfile,
                                cmsCIEXYZTRIPLE *result,
                                cmsUInt32Number intent)
{
    cmsHPROFILE hXYZ;
    cmsHTRANSFORM hTransform;
    cmsFloat64Number rgb[3][3] = {{1., 0., 0.},
    {0., 1., 0.},
    {0., 0., 1.}};

    hXYZ = cmsCreateXYZProfile();
    if (hXYZ == NULL) return FALSE;

    hTransform = cmsCreateTransform(hProfile, TYPE_RGB_DBL, hXYZ, TYPE_XYZ_DBL,
        intent, cmsFLAGS_NOCACHE | cmsFLAGS_NOOPTIMIZE);
    cmsCloseProfile(hXYZ);
    if (hTransform == NULL) return FALSE;

    cmsDoTransform(hTransform, rgb, result, 3);
    cmsDeleteTransform(hTransform);
    return TRUE;
}


LCMSBOOL dkCmsReadICCMatrixRGB2XYZ(LPMAT3 r, cmsHPROFILE hProfile)
{
    // See README @ Monday, July 27, 2009 @ Less is more
    // The example seem to be wrong, let's see if this one work
    return (LCMSBOOL) GetProfileRGBPrimaries(hProfile, r, INTENT_PERCEPTUAL);
}

LCMSAPI cmsHPROFILE   LCMSEXPORT dkCmsOpenProfileFromMem(LPVOID MemPtr, DWORD dwSize)
{
    return cmsOpenProfileFromMem(MemPtr, (cmsUInt32Number) dwSize);
}

LCMSAPI icProfileClassSignature LCMSEXPORT dkCmsGetDeviceClass(cmsHPROFILE hProfile)
{
    return (icProfileClassSignature) cmsGetDeviceClass(hProfile);
}

LCMSAPI LCMSBOOL      LCMSEXPORT dkCmsCloseProfile(cmsHPROFILE hProfile)
{
    return (LCMSBOOL) cmsCloseProfile(hProfile);
}

LCMSAPI cmsHTRANSFORM LCMSEXPORT dkCmsCreateProofingTransform(cmsHPROFILE Input,
                                                              DWORD InputFormat,
                                                              cmsHPROFILE Output,
                                                              DWORD OutputFormat,
                                                              cmsHPROFILE Proofing,
                                                              int Intent,
                                                              int ProofingIntent,
                                                              DWORD dwFlags)
{
    return cmsCreateProofingTransform(Input,
                                      (cmsUInt32Number) InputFormat,
                                      (cmsHPROFILE) Output,
                                      (cmsUInt32Number) OutputFormat,
                                      Proofing,
                                      (cmsUInt32Number) Intent,
                                      (cmsUInt32Number) ProofingIntent,
                                      (cmsUInt32Number) dwFlags);
}

LCMSAPI cmsHTRANSFORM LCMSEXPORT dkCmsCreateTransform(cmsHPROFILE Input,
                                                      DWORD InputFormat,
                                                      cmsHPROFILE Output,
                                                      DWORD OutputFormat,
                                                      int Intent,
                                                      DWORD dwFlags)
{
    return cmsCreateTransform(Input,
                              (cmsUInt32Number) InputFormat,
                              Output,
                              (cmsUInt32Number) OutputFormat,
                              (cmsUInt32Number) Intent,
                              (cmsUInt32Number) dwFlags);
}

LCMSAPI cmsHPROFILE   LCMSEXPORT dkCmsCreateXYZProfile()
{
    return cmsCreateXYZProfile();
}

LCMSAPI cmsHPROFILE   LCMSEXPORT dkCmsCreate_sRGBProfile()
{
    return cmsCreate_sRGBProfile();
}

LCMSAPI void         LCMSEXPORT dkCmsDeleteTransform(cmsHTRANSFORM hTransform)
{
    cmsDeleteTransform(hTransform);
}

LCMSAPI double        LCMSEXPORT dkCmsDeltaE(LPcmsCIELab Lab1, LPcmsCIELab Lab2)
{
    return (double) cmsDeltaE((cmsCIELab*) Lab1, (cmsCIELab*) Lab2);
}

LCMSAPI void          LCMSEXPORT dkCmsDoTransform(cmsHTRANSFORM Transform,
                                                  LPVOID InputBuffer,
                                                  LPVOID OutputBuffer,
                                                  unsigned int Size)
{
    cmsDoTransform(Transform,
                   (const void *) InputBuffer,
                   (void *) OutputBuffer,
                   (cmsUInt32Number) Size);

}

LCMSAPI void          LCMSEXPORT dkCmsFloat2XYZEncoded(WORD XYZ[3], const cmsCIEXYZ* fXYZ)
{
    cmsFloat2XYZEncoded((cmsUInt16Number*) &XYZ[3], (const cmsCIEXYZ*)fXYZ);
}

LCMSAPI icColorSpaceSignature   LCMSEXPORT dkCmsGetColorSpace(cmsHPROFILE hProfile)
{
    return (icColorSpaceSignature) cmsGetColorSpace(hProfile);
}

LCMSAPI icColorSpaceSignature   LCMSEXPORT dkCmsGetPCS(cmsHPROFILE hProfile)
{
    return (icColorSpaceSignature) cmsGetPCS(hProfile);
}

LCMSAPI LCMSBOOL      LCMSEXPORT dkCmsIsTag(cmsHPROFILE hProfile, icTagSignature sig)
{
    return (LCMSBOOL) cmsIsTag(hProfile, (cmsTagSignature) sig);
}

LCMSAPI cmsHPROFILE   LCMSEXPORT dkCmsOpenProfileFromFile(const char* ICCProfile, const char* sAccess)
{
    return cmsOpenProfileFromFile(ICCProfile, sAccess);
}

LCMSAPI void          LCMSEXPORT dkCmsXYZ2xyY(LPcmsCIExyY Dest, const cmsCIEXYZ* Source)
{
    cmsXYZ2xyY((cmsCIExyY*) Dest, Source);
}

LCMSAPI void          LCMSEXPORT dkCmsXYZEncoded2Float(LPcmsCIEXYZ fxyz, const WORD XYZ[3])
{
    cmsXYZEncoded2Float((cmsCIEXYZ*) fxyz, (const cmsUInt16Number*) &XYZ[3]);
}

#endif // defined(USE_LCMS_VERSION_2000)
