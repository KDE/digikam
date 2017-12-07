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

// Qt includes

#include <QString>

// Local includes

#include "digikam_debug.h"

#include "digikam-lcms.h"

#include <lcms2_plugin.h>

///////////////////////////////////////////////////////////////////////

void _l2tol1MAT3(MAT3* const l2, MAT3* const l1)
{
    // TODO: this seem plain wrong and don't provide perfect result
    l1->Red.X   = static_cast<cmsFloat64Number>( l2->Red.X   );
    l1->Red.Y   = static_cast<cmsFloat64Number>( l2->Green.X );
    l1->Red.Z   = static_cast<cmsFloat64Number>( l2->Blue.X  );
    l1->Green.X = static_cast<cmsFloat64Number>( l2->Red.Y   );
    l1->Green.Y = static_cast<cmsFloat64Number>( l2->Green.Y );
    l1->Green.Z = static_cast<cmsFloat64Number>( l2->Blue.Y  );
    l1->Blue.X  = static_cast<cmsFloat64Number>( l2->Red.Z   );
    l1->Blue.Y  = static_cast<cmsFloat64Number>( l2->Green.Z );
    l1->Blue.Z  = static_cast<cmsFloat64Number>( l2->Blue.Z  );
}

void _l1LPMAT3tol2cmsMAT3(LPMAT3 l1, cmsMAT3* const l2)
{
    l2->v[0].n[0] = static_cast<cmsFloat64Number>( l1->Red.X   );
    l2->v[0].n[1] = static_cast<cmsFloat64Number>( l1->Red.Y   );
    l2->v[0].n[2] = static_cast<cmsFloat64Number>( l1->Red.Z   );
    l2->v[1].n[0] = static_cast<cmsFloat64Number>( l1->Green.X );
    l2->v[1].n[1] = static_cast<cmsFloat64Number>( l1->Green.Y );
    l2->v[1].n[2] = static_cast<cmsFloat64Number>( l1->Green.Z );
    l2->v[2].n[0] = static_cast<cmsFloat64Number>( l1->Blue.X  );
    l2->v[2].n[1] = static_cast<cmsFloat64Number>( l1->Blue.Y  );
    l2->v[2].n[2] = static_cast<cmsFloat64Number>( l1->Blue.Z  );
}

void _l2cmsMAT3tol1LPMAT3(cmsMAT3* const l2, LPMAT3 l1)
{
    l1->Red.X   = static_cast<cmsFloat64Number>( l2->v[0].n[0] );
    l1->Red.Y   = static_cast<cmsFloat64Number>( l2->v[0].n[1] );
    l1->Red.Z   = static_cast<cmsFloat64Number>( l2->v[0].n[2] );
    l1->Green.X = static_cast<cmsFloat64Number>( l2->v[1].n[0] );
    l1->Green.Y = static_cast<cmsFloat64Number>( l2->v[1].n[1] );
    l1->Green.Z = static_cast<cmsFloat64Number>( l2->v[1].n[2] );
    l1->Blue.X  = static_cast<cmsFloat64Number>( l2->v[2].n[0] );
    l1->Blue.Y  = static_cast<cmsFloat64Number>( l2->v[2].n[1] );
    l1->Blue.Z  = static_cast<cmsFloat64Number>( l2->v[2].n[2] );
}

///////////////////////////////////////////////////////////////////////

#define MATRIX_DET_TOLERANCE    0.0001

/// Compute chromatic adaptation matrix using Chad as cone matrix
static cmsBool ComputeChromaticAdaptation(cmsMAT3* const Conversion,
                                          const cmsCIEXYZ* const SourceWhitePoint,
                                          const cmsCIEXYZ* const DestWhitePoint,
                                          const cmsMAT3* const Chad)
{
    cmsMAT3 Chad_Inv;
    cmsVEC3 ConeSourceXYZ, ConeSourceRGB;
    cmsVEC3 ConeDestXYZ, ConeDestRGB;
    cmsMAT3 Cone, Tmp;

    Tmp = *Chad;

    if (!_cmsMAT3inverse(&Tmp, &Chad_Inv))
        return FALSE;

    _cmsVEC3init(&ConeSourceXYZ, SourceWhitePoint -> X,
                                 SourceWhitePoint -> Y,
                                 SourceWhitePoint -> Z);

    _cmsVEC3init(&ConeDestXYZ, DestWhitePoint -> X,
                               DestWhitePoint -> Y,
                               DestWhitePoint -> Z);

    _cmsMAT3eval(&ConeSourceRGB, Chad, &ConeSourceXYZ);
    _cmsMAT3eval(&ConeDestRGB,   Chad, &ConeDestXYZ);

    // Build matrix
    _cmsVEC3init(&Cone.v[0], ConeDestRGB.n[0]/ConeSourceRGB.n[0], 0.0,                                   0.0                                );
    _cmsVEC3init(&Cone.v[1], 0.0,                                 ConeDestRGB.n[1]/ConeSourceRGB.n[1],   0.0                                );
    _cmsVEC3init(&Cone.v[2], 0.0,                                 0.0,                                   ConeDestRGB.n[2]/ConeSourceRGB.n[2]);

    // Normalize
    _cmsMAT3per(&Tmp, &Cone, Chad);
    _cmsMAT3per(Conversion, &Chad_Inv, &Tmp);

    return TRUE;
}

/** Returns the final chromatic adaptation from illuminant FromIll to Illuminant ToIll
 *  The cone matrix can be specified in ConeMatrix. If NULL, Bradford is assumed
 */
cmsBool _cmsAdaptationMatrix(cmsMAT3* const r, const cmsMAT3* ConeMatrix, const cmsCIEXYZ* const FromIll, const cmsCIEXYZ* const ToIll)
{
    // Bradford matrix
    cmsMAT3 LamRigg =
    {{
        {{  0.8951,  0.2664, -0.1614 }},
        {{ -0.7502,  1.7135,  0.0367 }},
        {{  0.0389, -0.0685,  1.0296 }}
    }};

    if (ConeMatrix == NULL)
        ConeMatrix = &LamRigg;

    return ComputeChromaticAdaptation(r, FromIll, ToIll, ConeMatrix);
}

/// Same as anterior, but assuming D50 destination. White point is given in xyY
static cmsBool _cmsAdaptMatrixToD50(cmsMAT3* const r, const cmsCIExyY* const SourceWhitePt)
{
    cmsCIEXYZ Dn;
    cmsMAT3   Bradford;
    cmsMAT3   Tmp;

    cmsxyY2XYZ(&Dn, SourceWhitePt);

    if (!_cmsAdaptationMatrix(&Bradford, NULL, &Dn, cmsD50_XYZ()))
        return FALSE;

    Tmp = *r;
    _cmsMAT3per(r, &Bradford, &Tmp);

    return TRUE;
}

/** Build a White point, primary chromas transfer matrix from RGB to CIE XYZ
    This is just an approximation, I am not handling all the non-linear
    aspects of the RGB to XYZ process, and assumming that the gamma correction
    has transitive property in the tranformation chain.

    the alghoritm:

               - First I build the absolute conversion matrix using
                 primaries in XYZ. This matrix is next inverted
               - Then I eval the source white point across this matrix
                 obtaining the coeficients of the transformation
               - Then, I apply these coeficients to the original matrix
*/
cmsBool _cmsBuildRGB2XYZtransferMatrix(cmsMAT3* const r, const cmsCIExyY* const WhitePt, const cmsCIExyYTRIPLE* const Primrs)
{
    cmsVEC3 WhitePoint, Coef;
    cmsMAT3 Result, Primaries;
    cmsFloat64Number xn, yn;
    cmsFloat64Number xr, yr;
    cmsFloat64Number xg, yg;
    cmsFloat64Number xb, yb;

    xn = WhitePt -> x;
    yn = WhitePt -> y;
    xr = Primrs -> Red.x;
    yr = Primrs -> Red.y;
    xg = Primrs -> Green.x;
    yg = Primrs -> Green.y;
    xb = Primrs -> Blue.x;
    yb = Primrs -> Blue.y;

    // Build Primaries matrix
    _cmsVEC3init(&Primaries.v[0], xr,        xg,         xb       );
    _cmsVEC3init(&Primaries.v[1], yr,        yg,         yb       );
    _cmsVEC3init(&Primaries.v[2], (1-xr-yr), (1-xg-yg),  (1-xb-yb));

    // Result = Primaries ^ (-1) inverse matrix
    if (!_cmsMAT3inverse(&Primaries, &Result))
        return FALSE;

    _cmsVEC3init(&WhitePoint, xn/yn, 1.0, (1.0-xn-yn)/yn);

    // Across inverse primaries ...
    _cmsMAT3eval(&Coef, &Result, &WhitePoint);

    // Give us the Coefs, then I build transformation matrix
    _cmsVEC3init(&r -> v[0], Coef.n[VX]*xr,          Coef.n[VY]*xg,          Coef.n[VZ]*xb         );
    _cmsVEC3init(&r -> v[1], Coef.n[VX]*yr,          Coef.n[VY]*yg,          Coef.n[VZ]*yb         );
    _cmsVEC3init(&r -> v[2], Coef.n[VX]*(1.0-xr-yr), Coef.n[VY]*(1.0-xg-yg), Coef.n[VZ]*(1.0-xb-yb));

    return _cmsAdaptMatrixToD50(r, WhitePt);
}

///////////////////////////////////////////////////////////////////////

/// WAS: Same as anterior, but assuming D50 source. White point is given in xyY
static cmsBool cmsAdaptMatrixFromD50(cmsMAT3* const r, const cmsCIExyY* const DestWhitePt)
{
    cmsCIEXYZ Dn;
    cmsMAT3 Bradford;
    cmsMAT3 Tmp;

    cmsxyY2XYZ(&Dn, DestWhitePt);

    if (!_cmsAdaptationMatrix(&Bradford, NULL, &Dn, cmsD50_XYZ()))
        return FALSE;

    Tmp = *r;
    _cmsMAT3per(r, &Bradford, &Tmp);

    return TRUE;
}

////////////////////////////////////////////////////

int dkCmsErrorAction(int nAction)
{
    Q_UNUSED(nAction);

    // TODO: Where is error logging?
    return 0;
}

DWORD dkCmsGetProfileICCversion(cmsHPROFILE hProfile)
{
    return (DWORD) cmsGetEncodedICCversion(hProfile);
}

void dkCmsSetAlarmCodes(int r, int g, int b)
{
    cmsUInt16Number NewAlarm[cmsMAXCHANNELS];
    NewAlarm[0] = (cmsUInt16Number)r * 256;
    NewAlarm[1] = (cmsUInt16Number)g * 256;
    NewAlarm[2] = (cmsUInt16Number)b * 256;
    cmsSetAlarmCodes(NewAlarm);
}

QString dkCmsTakeProductName(cmsHPROFILE hProfile)
{
    static char Name[1024*2+4];
    char Manufacturer[1024], Model[1024];

    Name[0]         = '\0';
    Manufacturer[0] = Model[0] = '\0';
    cmsMLU* mlu     = 0;

    if (cmsIsTag(hProfile, cmsSigDeviceMfgDescTag))
    {
        mlu = static_cast<cmsMLU*>( cmsReadTag(hProfile, cmsSigDeviceMfgDescTag) );
        cmsMLUgetASCII(mlu, "en", "US", Manufacturer, 1024);
    }

    if (cmsIsTag(hProfile, cmsSigDeviceModelDescTag))
    {
        mlu = static_cast<cmsMLU*>( cmsReadTag(hProfile, cmsSigDeviceModelDescTag) );
        cmsMLUgetASCII(mlu, "en", "US", Model, 1024);
    }

    if (!Manufacturer[0] && !Model[0])
    {

        if (cmsIsTag(hProfile, cmsSigProfileDescriptionTag))
        {
            mlu = static_cast<cmsMLU*>( cmsReadTag(hProfile, cmsSigProfileDescriptionTag) );
            cmsMLUgetASCII(mlu, "en", "US", Name, 1024);
            return QString::fromLatin1(Name);
        }
        else
        {
            return QString::fromLatin1("{no name}");
        }
    }

    if (!Manufacturer[0] || strncmp(Model, Manufacturer, 8) == 0 || strlen(Model) > 30)
    {
        strcpy(Name, Model);
    }
    else
    {
        sprintf(Name, "%s - %s", Model, Manufacturer);
    }

    return QString::fromLatin1(Name);
}

QString dkCmsTakeProductDesc(cmsHPROFILE hProfile)
{
    static char Name[2048];

    if (cmsIsTag(hProfile, cmsSigProfileDescriptionTag))
    {
        cmsMLU* const mlu = static_cast<cmsMLU*>( cmsReadTag(hProfile, cmsSigProfileDescriptionTag) );
        cmsMLUgetASCII(mlu, "en", "US", Name, 1024);
    }
    else
    {
        return dkCmsTakeProductName(hProfile);
    }

    if (strncmp(Name, "Copyrig", 7) == 0)
    {
        return dkCmsTakeProductName(hProfile);
    }

    return QString::fromLatin1(Name);
}

QString dkCmsTakeProductInfo(cmsHPROFILE hProfile)
{
    static char Info[4096];
    cmsMLU*     mlu = 0;
    Info[0]         = '\0';

    if (cmsIsTag(hProfile, cmsSigProfileDescriptionTag))
    {
        char Desc[1024];

        mlu = static_cast<cmsMLU*>( cmsReadTag(hProfile, cmsSigProfileDescriptionTag) );
        cmsMLUgetASCII(mlu, "en", "US", Desc, 1024);
        strcat(Info, Desc);
    }

    if (cmsIsTag(hProfile, cmsSigCopyrightTag))
    {
        char Copyright[1024];

        mlu = static_cast<cmsMLU*>( cmsReadTag(hProfile, cmsSigCopyrightTag) );
        cmsMLUgetASCII(mlu, "en", "US", Copyright, 1024);
        strcat(Info, " - ");
        strcat(Info, Copyright);
    }

#define K007 static_cast<cmsTagSignature>( 0x4B303037 )

    if (cmsIsTag(hProfile, K007))
    {
        char MonCal[1024];

        mlu = static_cast<cmsMLU*>( cmsReadTag(hProfile, K007) );
        cmsMLUgetASCII(mlu, "en", "US", MonCal, 1024);
        strcat(Info, " - ");
        strcat(Info, MonCal);
    }
    else
    {
        /*
         *  _cmsIdentifyWhitePoint is complex and partly redundant
         *  with cietonguewidget, leave this part off
         *  untill the full lcms2 implementation
         *
        cmsCIEXYZ WhitePt;
        char WhiteStr[1024];

        dkCmsTakeMediaWhitePoint(&WhitePt, hProfile);
        _cmsIdentifyWhitePoint(WhiteStr, &WhitePt);
        strcat(Info, " - ");
        strcat(Info, WhiteStr);
        */
    }

#undef K007

    return QString::fromLatin1(Info);
}

QString dkCmsTakeManufacturer(cmsHPROFILE hProfile)
{
    char buffer[1024];
    buffer[0] = '\0';
    cmsGetProfileInfoASCII(hProfile, cmsInfoManufacturer, "en", "US", buffer, 1024);
    return QString::fromLatin1(buffer);
}

LCMSBOOL dkCmsTakeMediaWhitePoint(LPcmsCIEXYZ Dest, cmsHPROFILE hProfile)
{
    LPcmsCIEXYZ tag = static_cast<LPcmsCIEXYZ>( cmsReadTag(hProfile, cmsSigMediaWhitePointTag) );

    if (tag == NULL)
        return FALSE;

    *Dest = *tag;
    return TRUE;
}

QString dkCmsTakeModel(cmsHPROFILE hProfile)
{
    char buffer[1024];
    const cmsMLU* const mlu = (const cmsMLU* const)cmsReadTag(hProfile, cmsSigDeviceModelDescTag);
    buffer[0]               = '\0';

    if (mlu == NULL)
        return QString();

    cmsMLUgetASCII(mlu, "en", "US", buffer, 1024);
    return QString::fromLatin1(buffer);
}

QString dkCmsTakeCopyright(cmsHPROFILE hProfile)
{
    char buffer[1024];
    const cmsMLU* const mlu = (const cmsMLU* const)cmsReadTag(hProfile, cmsSigCopyrightTag);
    buffer[0]               = '\0';

    if (mlu == NULL)
        return QString();

    cmsMLUgetASCII(mlu, "en", "US", buffer, 1024);
    return QString::fromLatin1(buffer);
}

DWORD dkCmsTakeHeaderFlags(cmsHPROFILE hProfile)
{
    return static_cast<DWORD>( cmsGetHeaderFlags(hProfile) );
}

const BYTE* dkCmsTakeProfileID(cmsHPROFILE hProfile)
{
    cmsUInt8Number* const ProfileID = new cmsUInt8Number[16];
    cmsGetHeaderProfileID(hProfile, ProfileID);
    return static_cast<BYTE*>( ProfileID );
}

int dkCmsTakeRenderingIntent(cmsHPROFILE hProfile)
{
    return static_cast<int>( cmsGetHeaderRenderingIntent(hProfile) );
}

// White Point & Primary chromas handling
// Returns the final chrmatic adaptation from illuminant FromIll to Illuminant ToIll
// The cone matrix can be specified in ConeMatrix.
// If NULL, assuming D50 source. White point is given in xyY

LCMSBOOL dkCmsAdaptMatrixFromD50(LPMAT3 r, LPcmsCIExyY DestWhitePt)
{
    // TODO: all based on private stuff, need to understand what digikam do in cietonguewidget with dkCmsAdaptMatrixFromD50
    cmsMAT3 result;

    _l1LPMAT3tol2cmsMAT3(r, &result);

    bool ret = cmsAdaptMatrixFromD50(&result, static_cast<const cmsCIExyY*>( DestWhitePt ));

    _l2cmsMAT3tol1LPMAT3(&result, r);

    return ret;
}

// LCMSBOOL dkCmsAdaptMatrixFromD50(LPMAT3 r, LPcmsCIExyY DestWhitePt)
// {
//     // TODO: all based on private stuff, need to understand what digikam do in cietonguewidget with dkCmsAdaptMatrixFromD50
//     cmsMAT3 result;
//
//     result.v[0].n[0] = r->Red.X  ;
//     result.v[0].n[1] = r->Red.Y  ;
//     result.v[0].n[2] = r->Red.Z  ;
//     result.v[1].n[0] = r->Green.X;
//     result.v[1].n[1] = r->Green.Y;
//     result.v[1].n[2] = r->Green.Z;
//     result.v[2].n[0] = r->Blue.X ;
//     result.v[2].n[1] = r->Blue.Y ;
//     result.v[2].n[2] = r->Blue.Z ;
//
//     bool ret   = cmsAdaptMatrixFromD50(&result, static_cast<const cmsCIExyY*>( DestWhitePt ));
//
//     r->Red.X   = result.v[0].n[0];
//     r->Red.Y   = result.v[0].n[1];
//     r->Red.Z   = result.v[0].n[2];
//     r->Green.X = result.v[1].n[0];
//     r->Green.Y = result.v[1].n[1];
//     r->Green.Z = result.v[1].n[2];
//     r->Blue.X  = result.v[2].n[0];
//     r->Blue.Y  = result.v[2].n[1];
//     r->Blue.Z  = result.v[2].n[2];
//
//     return ret;
// }

cmsBool GetProfileRGBPrimaries(cmsHPROFILE hProfile, cmsCIEXYZTRIPLE* const result, cmsUInt32Number intent)
{
    cmsHPROFILE hXYZ;
    cmsHTRANSFORM hTransform;
    cmsFloat64Number rgb[3][3] = {{1., 0., 0.},
                                  {0., 1., 0.},
                                  {0., 0., 1.}};

    hXYZ = cmsCreateXYZProfile();

    if (hXYZ == NULL)
        return FALSE;

    hTransform = cmsCreateTransform(hProfile, TYPE_RGB_DBL, hXYZ, TYPE_XYZ_DBL,
                                    intent, cmsFLAGS_NOCACHE | cmsFLAGS_NOOPTIMIZE);
    cmsCloseProfile(hXYZ);

    if (hTransform == NULL)
        return FALSE;

    cmsDoTransform(hTransform, rgb, result, 3);
    cmsDeleteTransform(hTransform);
    return TRUE;
}

LCMSBOOL dkCmsReadICCMatrixRGB2XYZ(LPMAT3 r, cmsHPROFILE hProfile)
{
    MAT3 result;
    LCMSBOOL ret;

    // See README @ Monday, July 27, 2009 @ Less is more
    // return static_cast<LCMSBOOL>( GetProfileRGBPrimaries(hProfile, r, INTENT_RELATIVE_COLORIMETRIC) );

    ret = GetProfileRGBPrimaries(hProfile, &result, INTENT_RELATIVE_COLORIMETRIC);

    _l2tol1MAT3(&result, r);

    return ret;
}

cmsHPROFILE dkCmsOpenProfileFromMem(LPVOID MemPtr, DWORD dwSize)
{
    return cmsOpenProfileFromMem(MemPtr, static_cast<cmsUInt32Number>( dwSize ));
}

icProfileClassSignature dkCmsGetDeviceClass(cmsHPROFILE hProfile)
{
    return static_cast<icProfileClassSignature>( cmsGetDeviceClass(hProfile) );
}

LCMSBOOL dkCmsCloseProfile(cmsHPROFILE hProfile)
{
    return static_cast<LCMSBOOL>( cmsCloseProfile(hProfile) );
}

cmsHTRANSFORM dkCmsCreateProofingTransform(cmsHPROFILE Input,
                                           DWORD InputFormat,
                                           cmsHPROFILE Output,
                                           DWORD OutputFormat,
                                           cmsHPROFILE Proofing,
                                           int Intent,
                                           int ProofingIntent,
                                           DWORD dwFlags)
{
    return cmsCreateProofingTransform(Input,
                                      static_cast<cmsUInt32Number>( InputFormat ),
                                      static_cast<cmsHPROFILE>( Output ),
                                      static_cast<cmsUInt32Number>( OutputFormat ),
                                      Proofing,
                                      static_cast<cmsUInt32Number>( Intent ),
                                      static_cast<cmsUInt32Number>( ProofingIntent ),
                                      static_cast<cmsUInt32Number>( dwFlags ));
}

cmsHTRANSFORM dkCmsCreateTransform(cmsHPROFILE Input,
                                   DWORD InputFormat,
                                   cmsHPROFILE Output,
                                   DWORD OutputFormat,
                                   int Intent,
                                   DWORD dwFlags)
{
    return cmsCreateTransform(Input,
                              static_cast<cmsUInt32Number>( InputFormat ),
                              Output,
                              static_cast<cmsUInt32Number>( OutputFormat ),
                              static_cast<cmsUInt32Number>( Intent ),
                              static_cast<cmsUInt32Number>( dwFlags ));
}

cmsHPROFILE dkCmsCreateXYZProfile()
{
    return cmsCreateXYZProfile();
}

cmsHPROFILE dkCmsCreate_sRGBProfile()
{
    return cmsCreate_sRGBProfile();
}

void dkCmsDeleteTransform(cmsHTRANSFORM hTransform)
{
    cmsDeleteTransform(hTransform);
}

double dkCmsDeltaE(LPcmsCIELab Lab1, LPcmsCIELab Lab2)
{
    return static_cast<double>( cmsDeltaE(static_cast<cmsCIELab*>( Lab1 ), static_cast<cmsCIELab*>( Lab2 )) );
}

void dkCmsDoTransform(cmsHTRANSFORM Transform,
                      LPVOID InputBuffer,
                      LPVOID OutputBuffer,
                      unsigned int Size)
{
    cmsDoTransform(Transform,
                   static_cast<const void*>( InputBuffer ),
                   static_cast<void*>( OutputBuffer ),
                   static_cast<cmsUInt32Number>( Size ));
}

void dkCmsFloat2XYZEncoded(WORD XYZ[3], const cmsCIEXYZ* const fXYZ)
{
    cmsFloat2XYZEncoded(XYZ, fXYZ);
}

icColorSpaceSignature dkCmsGetColorSpace(cmsHPROFILE hProfile)
{
    return static_cast<icColorSpaceSignature>( cmsGetColorSpace(hProfile) );
}

icColorSpaceSignature dkCmsGetPCS(cmsHPROFILE hProfile)
{
    return static_cast<icColorSpaceSignature>( cmsGetPCS(hProfile) );
}

LCMSBOOL dkCmsIsTag(cmsHPROFILE hProfile, icTagSignature sig)
{
    return static_cast<LCMSBOOL>( cmsIsTag(hProfile, static_cast<cmsTagSignature>( sig )) );
}

cmsHPROFILE dkCmsOpenProfileFromFile(const char* const ICCProfile, const char* const sAccess)
{
    return cmsOpenProfileFromFile(ICCProfile, sAccess);
}

void dkCmsXYZ2xyY(LPcmsCIExyY Dest, const cmsCIEXYZ* const Source)
{
    cmsXYZ2xyY(static_cast<cmsCIExyY*>(Dest), Source);
}
