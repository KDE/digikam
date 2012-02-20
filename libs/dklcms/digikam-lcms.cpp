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
#include <kdebug.h>


#if defined(USE_LCMS_VERSION_2000)

#include <lcms2.h>

#include <QtCore/QString>

#include "digikam-lcms.h"

#include <lcms2_plugin.h>

// CMSAPI void               CMSEXPORT _cmsVEC3init(cmsVEC3* r, cmsFloat64Number x, cmsFloat64Number y, cmsFloat64Number z) { };
// CMSAPI void               CMSEXPORT _cmsMAT3per(cmsMAT3* r, const cmsMAT3* a, const cmsMAT3* b) { };
// CMSAPI cmsBool            CMSEXPORT _cmsMAT3inverse(const cmsMAT3* a, cmsMAT3* b) { return TRUE; };
// CMSAPI void               CMSEXPORT _cmsMAT3eval(cmsVEC3* r, const cmsMAT3* a, const cmsVEC3* v) { };

#define MATRIX_DET_TOLERANCE    0.0001

// Initiate a vector
void CMSEXPORT _cmsVEC3init(cmsVEC3* r, cmsFloat64Number x, cmsFloat64Number y, cmsFloat64Number z)
{
    r -> n[VX] = x;
    r -> n[VY] = y;
    r -> n[VZ] = z;
}

// Multiply two matrices
void CMSEXPORT _cmsMAT3per(cmsMAT3* r, const cmsMAT3* a, const cmsMAT3* b)
{
#define ROWCOL(i, j) \
    a->v[i].n[0]*b->v[0].n[j] + a->v[i].n[1]*b->v[1].n[j] + a->v[i].n[2]*b->v[2].n[j]

    _cmsVEC3init(&r-> v[0], ROWCOL(0,0), ROWCOL(0,1), ROWCOL(0,2));
    _cmsVEC3init(&r-> v[1], ROWCOL(1,0), ROWCOL(1,1), ROWCOL(1,2));
    _cmsVEC3init(&r-> v[2], ROWCOL(2,0), ROWCOL(2,1), ROWCOL(2,2));

#undef ROWCOL //(i, j)
}


// Inverse of a matrix b = a^(-1)
cmsBool  CMSEXPORT _cmsMAT3inverse(const cmsMAT3* a, cmsMAT3* b)
{
   cmsFloat64Number det, c0, c1, c2;

   c0 =  a -> v[1].n[1]*a -> v[2].n[2] - a -> v[1].n[2]*a -> v[2].n[1];
   c1 = -a -> v[1].n[0]*a -> v[2].n[2] + a -> v[1].n[2]*a -> v[2].n[0];
   c2 =  a -> v[1].n[0]*a -> v[2].n[1] - a -> v[1].n[1]*a -> v[2].n[0];

   det = a -> v[0].n[0]*c0 + a -> v[0].n[1]*c1 + a -> v[0].n[2]*c2;

   if (fabs(det) < MATRIX_DET_TOLERANCE) return FALSE;  // singular matrix; can't invert

   b -> v[0].n[0] = c0/det;
   b -> v[0].n[1] = (a -> v[0].n[2]*a -> v[2].n[1] - a -> v[0].n[1]*a -> v[2].n[2])/det;
   b -> v[0].n[2] = (a -> v[0].n[1]*a -> v[1].n[2] - a -> v[0].n[2]*a -> v[1].n[1])/det;
   b -> v[1].n[0] = c1/det;
   b -> v[1].n[1] = (a -> v[0].n[0]*a -> v[2].n[2] - a -> v[0].n[2]*a -> v[2].n[0])/det;
   b -> v[1].n[2] = (a -> v[0].n[2]*a -> v[1].n[0] - a -> v[0].n[0]*a -> v[1].n[2])/det;
   b -> v[2].n[0] = c2/det;
   b -> v[2].n[1] = (a -> v[0].n[1]*a -> v[2].n[0] - a -> v[0].n[0]*a -> v[2].n[1])/det;
   b -> v[2].n[2] = (a -> v[0].n[0]*a -> v[1].n[1] - a -> v[0].n[1]*a -> v[1].n[0])/det;

   return TRUE;
}

// Evaluate a vector across a matrix
void CMSEXPORT _cmsMAT3eval(cmsVEC3* r, const cmsMAT3* a, const cmsVEC3* v)
{
    r->n[VX] = a->v[0].n[VX]*v->n[VX] + a->v[0].n[VY]*v->n[VY] + a->v[0].n[VZ]*v->n[VZ];
    r->n[VY] = a->v[1].n[VX]*v->n[VX] + a->v[1].n[VY]*v->n[VY] + a->v[1].n[VZ]*v->n[VZ];
    r->n[VZ] = a->v[2].n[VX]*v->n[VX] + a->v[2].n[VY]*v->n[VY] + a->v[2].n[VZ]*v->n[VZ];
}


// Compute chromatic adaptation matrix using Chad as cone matrix
static
cmsBool ComputeChromaticAdaptation(cmsMAT3* Conversion,
                                const cmsCIEXYZ* SourceWhitePoint,
                                const cmsCIEXYZ* DestWhitePoint,
                                const cmsMAT3* Chad)

{

    cmsMAT3 Chad_Inv;
    cmsVEC3 ConeSourceXYZ, ConeSourceRGB;
    cmsVEC3 ConeDestXYZ, ConeDestRGB;
    cmsMAT3 Cone, Tmp;


    Tmp = *Chad;
    if (!_cmsMAT3inverse(&Tmp, &Chad_Inv)) return FALSE;

    _cmsVEC3init(&ConeSourceXYZ, SourceWhitePoint -> X,
                             SourceWhitePoint -> Y,
                             SourceWhitePoint -> Z);

    _cmsVEC3init(&ConeDestXYZ,   DestWhitePoint -> X,
                             DestWhitePoint -> Y,
                             DestWhitePoint -> Z);

    _cmsMAT3eval(&ConeSourceRGB, Chad, &ConeSourceXYZ);
    _cmsMAT3eval(&ConeDestRGB,   Chad, &ConeDestXYZ);

    // Build matrix
    _cmsVEC3init(&Cone.v[0], ConeDestRGB.n[0]/ConeSourceRGB.n[0],    0.0,  0.0);
    _cmsVEC3init(&Cone.v[1], 0.0,   ConeDestRGB.n[1]/ConeSourceRGB.n[1],   0.0);
    _cmsVEC3init(&Cone.v[2], 0.0,   0.0,   ConeDestRGB.n[2]/ConeSourceRGB.n[2]);


    // Normalize
    _cmsMAT3per(&Tmp, &Cone, Chad);
    _cmsMAT3per(Conversion, &Chad_Inv, &Tmp);

    return TRUE;
}

// Returns the final chrmatic adaptation from illuminant FromIll to Illuminant ToIll
// The cone matrix can be specified in ConeMatrix. If NULL, Bradford is assumed
cmsBool  _cmsAdaptationMatrix(cmsMAT3* r, const cmsMAT3* ConeMatrix, const cmsCIEXYZ* FromIll, const cmsCIEXYZ* ToIll)
{
    cmsMAT3 LamRigg   = {{ // Bradford matrix
        {{  0.8951,  0.2664, -0.1614 }},
        {{ -0.7502,  1.7135,  0.0367 }},
        {{  0.0389, -0.0685,  1.0296 }}
    }};

    if (ConeMatrix == NULL)
        ConeMatrix = &LamRigg;

    return ComputeChromaticAdaptation(r, FromIll, ToIll, ConeMatrix);
}

// Same as anterior, but assuming D50 destination. White point is given in xyY
static
cmsBool _cmsAdaptMatrixToD50(cmsMAT3* r, const cmsCIExyY* SourceWhitePt)
{
    cmsCIEXYZ Dn;
    cmsMAT3 Bradford;
    cmsMAT3 Tmp;

    cmsxyY2XYZ(&Dn, SourceWhitePt);

    if (!_cmsAdaptationMatrix(&Bradford, NULL, &Dn, cmsD50_XYZ())) return FALSE;

    Tmp = *r;
    _cmsMAT3per(r, &Bradford, &Tmp);

    return TRUE;
}

// Build a White point, primary chromas transfer matrix from RGB to CIE XYZ
// This is just an approximation, I am not handling all the non-linear
// aspects of the RGB to XYZ process, and assumming that the gamma correction
// has transitive property in the tranformation chain.
//
// the alghoritm:
//
//            - First I build the absolute conversion matrix using
//              primaries in XYZ. This matrix is next inverted
//            - Then I eval the source white point across this matrix
//              obtaining the coeficients of the transformation
//            - Then, I apply these coeficients to the original matrix
//
cmsBool _cmsBuildRGB2XYZtransferMatrix(cmsMAT3* r, const cmsCIExyY* WhitePt, const cmsCIExyYTRIPLE* Primrs)
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
    _cmsVEC3init(&Primaries.v[0], xr,        xg,         xb);
    _cmsVEC3init(&Primaries.v[1], yr,        yg,         yb);
    _cmsVEC3init(&Primaries.v[2], (1-xr-yr), (1-xg-yg),  (1-xb-yb));


    // Result = Primaries ^ (-1) inverse matrix
    if (!_cmsMAT3inverse(&Primaries, &Result))
        return FALSE;


    _cmsVEC3init(&WhitePoint, xn/yn, 1.0, (1.0-xn-yn)/yn);

    // Across inverse primaries ...
    _cmsMAT3eval(&Coef, &Result, &WhitePoint);

    // Give us the Coefs, then I build transformation matrix
    _cmsVEC3init(&r -> v[0], Coef.n[VX]*xr,          Coef.n[VY]*xg,          Coef.n[VZ]*xb);
    _cmsVEC3init(&r -> v[1], Coef.n[VX]*yr,          Coef.n[VY]*yg,          Coef.n[VZ]*yb);
    _cmsVEC3init(&r -> v[2], Coef.n[VX]*(1.0-xr-yr), Coef.n[VY]*(1.0-xg-yg), Coef.n[VZ]*(1.0-xb-yb));


    return _cmsAdaptMatrixToD50(r, WhitePt);

}




// WAS: Same as anterior, but assuming D50 source. White point is given in xyY
static
cmsBool cmsAdaptMatrixFromD50(cmsMAT3* r, const cmsCIExyY* DestWhitePt)
{
    cmsCIEXYZ Dn;
    cmsMAT3 Bradford;
    cmsMAT3 Tmp;

    cmsxyY2XYZ(&Dn, DestWhitePt);

    if (!_cmsAdaptationMatrix(&Bradford, NULL, &Dn, cmsD50_XYZ())) return FALSE;

    Tmp = *r;
    _cmsMAT3per(r, &Bradford, &Tmp);

    return TRUE;
}


////////////////////////////////////////////////////

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

  LPcmsCIEXYZ tag = static_cast<LPcmsCIEXYZ>( cmsReadTag(hProfile, cmsSigMediaWhitePointTag) );
  if (tag == NULL) return FALSE;

  *Dest = *tag;
  return TRUE;
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
    return static_cast<DWORD>( cmsGetHeaderFlags(hProfile) );
}

LCMSAPI const BYTE*   LCMSEXPORT dkCmsTakeProfileID(cmsHPROFILE hProfile)
{
    cmsUInt8Number* ProfileID = new cmsUInt8Number();
    cmsGetHeaderProfileID(hProfile, ProfileID);
    return static_cast<BYTE*>( ProfileID );
}

LCMSAPI int           LCMSEXPORT dkCmsTakeRenderingIntent(cmsHPROFILE hProfile)
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
    bool ret = FALSE;

    result.v[0].n[0] = r->Red.X  ;
    result.v[0].n[1] = r->Red.Y  ;
    result.v[0].n[2] = r->Red.Z  ;
    result.v[1].n[0] = r->Green.X;
    result.v[1].n[1] = r->Green.Y;
    result.v[1].n[2] = r->Green.Z;
    result.v[2].n[0] = r->Blue.X ;
    result.v[2].n[1] = r->Blue.Y ;
    result.v[2].n[2] = r->Blue.Z ;

    ret = cmsAdaptMatrixFromD50(&result, static_cast<const cmsCIExyY*>( DestWhitePt ));

    r->Red.X   = result.v[0].n[0];
    r->Red.Y   = result.v[0].n[1];
    r->Red.Z   = result.v[0].n[2];
    r->Green.X = result.v[1].n[0];
    r->Green.Y = result.v[1].n[1];
    r->Green.Z = result.v[1].n[2];
    r->Blue.X  = result.v[2].n[0];
    r->Blue.Y  = result.v[2].n[1];
    r->Blue.Z  = result.v[2].n[2];

    return ret;
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

    MAT3 result;
    LCMSBOOL ret;

    // See README @ Monday, July 27, 2009 @ Less is more
    return (LCMSBOOL) GetProfileRGBPrimaries(hProfile, r, INTENT_RELATIVE_COLORIMETRIC);
}

LCMSAPI cmsHPROFILE   LCMSEXPORT dkCmsOpenProfileFromMem(LPVOID MemPtr, DWORD dwSize)
{
    return cmsOpenProfileFromMem(MemPtr, static_cast<cmsUInt32Number>( dwSize ));
}

LCMSAPI icProfileClassSignature LCMSEXPORT dkCmsGetDeviceClass(cmsHPROFILE hProfile)
{
    return static_cast<icProfileClassSignature>( cmsGetDeviceClass(hProfile) );
}

LCMSAPI LCMSBOOL      LCMSEXPORT dkCmsCloseProfile(cmsHPROFILE hProfile)
{
    return static_cast<LCMSBOOL>( cmsCloseProfile(hProfile) );
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
                                      static_cast<cmsUInt32Number>( InputFormat ),
                                      static_cast<cmsHPROFILE>( Output ),
                                      static_cast<cmsUInt32Number>( OutputFormat ),
                                      Proofing,
                                      static_cast<cmsUInt32Number>( Intent ),
                                      static_cast<cmsUInt32Number>( ProofingIntent ),
                                      static_cast<cmsUInt32Number>( dwFlags ));
}

LCMSAPI cmsHTRANSFORM LCMSEXPORT dkCmsCreateTransform(cmsHPROFILE Input,
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
    return static_cast<double>( cmsDeltaE(static_cast<cmsCIELab*>( Lab1 ), static_cast<cmsCIELab*>( Lab2 )) );
}

LCMSAPI void          LCMSEXPORT dkCmsDoTransform(cmsHTRANSFORM Transform,
                                                  LPVOID InputBuffer,
                                                  LPVOID OutputBuffer,
                                                  unsigned int Size)
{
    cmsDoTransform(Transform,
                   static_cast<const void *>( InputBuffer ),
                   static_cast<void *> ( OutputBuffer ),
                   static_cast<cmsUInt32Number>( Size ));

}

LCMSAPI void          LCMSEXPORT dkCmsFloat2XYZEncoded(WORD XYZ[3], const cmsCIEXYZ* fXYZ)
{
    cmsFloat2XYZEncoded(static_cast<cmsUInt16Number*>( &XYZ[3] ), static_cast<const cmsCIEXYZ*>( fXYZ ));
}

LCMSAPI icColorSpaceSignature   LCMSEXPORT dkCmsGetColorSpace(cmsHPROFILE hProfile)
{
    return static_cast<icColorSpaceSignature>( cmsGetColorSpace(hProfile) );
}

LCMSAPI icColorSpaceSignature   LCMSEXPORT dkCmsGetPCS(cmsHPROFILE hProfile)
{
    return static_cast<icColorSpaceSignature>( cmsGetPCS(hProfile) );
}

LCMSAPI LCMSBOOL      LCMSEXPORT dkCmsIsTag(cmsHPROFILE hProfile, icTagSignature sig)
{
    return static_cast<LCMSBOOL>( cmsIsTag(hProfile, (cmsTagSignature) sig) );
}

LCMSAPI cmsHPROFILE   LCMSEXPORT dkCmsOpenProfileFromFile(const char* ICCProfile, const char* sAccess)
{
    return cmsOpenProfileFromFile(ICCProfile, sAccess);
}

LCMSAPI void          LCMSEXPORT dkCmsXYZ2xyY(LPcmsCIExyY Dest, const cmsCIEXYZ* Source)
{
    cmsXYZ2xyY(static_cast<cmsCIExyY*>(Dest), Source);
}

LCMSAPI void          LCMSEXPORT dkCmsXYZEncoded2Float(LPcmsCIEXYZ fxyz, const WORD XYZ[3])
{
    cmsXYZEncoded2Float(static_cast<cmsCIEXYZ*>(fxyz) , static_cast<const cmsUInt16Number*>(&XYZ[3]));
}

#endif // defined(USE_LCMS_VERSION_2000)
