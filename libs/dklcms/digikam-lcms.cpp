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

#include <config-digikam.h>

#if defined(USE_LCMS_VERSION_2000)

#include <lcms2.h>
#include "digikam-lcms.h"


LCMSAPI int    LCMSEXPORT dkCmsErrorAction(int nAction)
{
    // TODO: there is error logging
    return 0;
}

LCMSAPI DWORD  LCMSEXPORT dkCmsGetProfileICCversion(cmsHPROFILE hProfile)
{
    // ./libs/widgets/iccprofiles/iccprofilewidget.cpp
    // metaDataMap.insert("Icc.Header.ProfileVersion", QString::number((uint)cmsGetProfileICCversion(hProfile)));
    return 0;
}

LCMSEXPORT void dkCmsSetAlarmCodes(int r, int g, int b)
{
    cmsUInt16Number NewAlarm[cmsMAXCHANNELS];
    NewAlarm[0] = (cmsUInt16Number)r;
    NewAlarm[1] = (cmsUInt16Number)g;
    NewAlarm[2] = (cmsUInt16Number)b;
    cmsSetAlarmCodes(NewAlarm);
    //void cmsSetAlarmCodes(cmsUInt16Number NewAlarm[cmsMAXCHANNELS]);
}

LCMSAPI const char*   LCMSEXPORT dkCmsTakeProductName(cmsHPROFILE hProfile)
{
    //static char Name[LCMS_DESC_MAX*2+4];
    static char Name[1];
    Name[0] = '\0';
    return Name;
}

LCMSAPI const char*   LCMSEXPORT dkCmsTakeProductDesc(cmsHPROFILE hProfile)
{
    static char ret[1]; ret[0] = '\0'; return ret;
}

LCMSAPI const char*   LCMSEXPORT dkCmsTakeProductInfo(cmsHPROFILE hProfile)
{
    static char ret[1]; ret[0] = '\0'; return ret;
}

LCMSAPI const char*   LCMSEXPORT dkCmsTakeManufacturer(cmsHPROFILE hProfile)
{
    static char ret[1]; ret[0] = '\0'; return ret;
}

LCMSAPI LCMSBOOL      LCMSEXPORT dkCmsTakeMediaWhitePoint(LPcmsCIEXYZ Dest, cmsHPROFILE hProfile)
{
    //TODO:
    return FALSE;
}

LCMSAPI const char*   LCMSEXPORT dkCmsTakeModel(cmsHPROFILE hProfile)
{
    static char ret[1]; ret[0] = '\0'; return ret;
}

LCMSAPI const char*   LCMSEXPORT dkCmsTakeCopyright(cmsHPROFILE hProfile)
{
    static char ret[1]; ret[0] = '\0'; return ret;
}


LCMSAPI DWORD         LCMSEXPORT dkCmsTakeHeaderFlags(cmsHPROFILE hProfile)
{
    //TODO: LPLCMSICCPROFILE  Icc = (LPLCMSICCPROFILE) hProfile;
    return (DWORD) 0;
}

LCMSAPI const BYTE*   LCMSEXPORT dkCmsTakeProfileID(cmsHPROFILE hProfile)
{
    const BYTE* ret = new BYTE(0);
    return ret;
}

LCMSAPI int           LCMSEXPORT dkCmsTakeRenderingIntent(cmsHPROFILE hProfile)
{
    //TODO: return (int) Icc -> RenderingIntent;
    return 0;
}

LCMSAPI LCMSBOOL      LCMSEXPORT dkCmsTakeCharTargetData(cmsHPROFILE hProfile, char** Data, size_t* len)
{
    *Data = NULL;
    *len  = 0;
    //TODO: Data = ???;
    //TODO: len = ???;
    return FALSE;
}

LCMSBOOL dkCmsAdaptMatrixFromD50(LPMAT3 r, LPcmsCIExyY DestWhitePt)
{
    // FIXME:
    return FALSE;
}

LCMSBOOL dkCmsReadICCMatrixRGB2XYZ(LPMAT3 r, cmsHPROFILE hProfile)
{
    // FIXME:
    return FALSE;
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
