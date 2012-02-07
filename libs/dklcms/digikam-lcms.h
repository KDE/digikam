/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-03
 * Description : aid to lcms2 porting
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

#ifndef DIGIKAM_LCMS_H
#define DIGIKAM_LCMS_H


/*

Friday, December 17, 2010

Multithreading question


Question: Do I need a rocket science degree to deal with lcms2 in multithreading mode?
What are ContextID and THR functions?


Actually it is a lot more simple. ContexID is nothing else that a void pointer that user
can associate to profiles and/or transforms. It has no meaning. Is just a sort of used
defined cargo that you can use on your convenience. lcms does nothing with that . It has
no relationship with threads, but can be used to store information about the thread.
Obviously you can ignore it if wish so. Then, by default this void pointer is set to NULL
when creating the transform or opening the profiles. Additionally, if the programmer wish,
there are functions which end with THR that can set the this to values other than NULL. In
this way the threads, processes or wathever that are using the profiles and transforms can
retrieve the value. It is just a way to store a 32 bit value along the handles.


On the other hand we have the 1-pixel cache. This is very convenient on slow interpolation
methods when most of the pixels in the image are similar. Obviously, caching means the
transform should store the result of last processed pixel, then in the case two threads
are using the same transform at the same time, memory read/write operations on this value
may clash and therefore you need some sort of semaphore. Ok, you can use a semaphore (the
pthreads) or just get rid of the cache enterely. Please note that in some situations the
cache is not used at all, i.e., on matrix-shaper to matrix-shaper 8 bit, it is actually
faster to do always the computations, so the cache schema is discarded on this case. On
CMYK trilinear, cache is being used as interpolation tends to be slow.


So, to answer your questions: If you use redundant transforms, you need not to worry about
anything as each transform is using different cache. May be fast, but this is big a waste
of memory. If you share the same transform on several threads, which is very efficient,
you have either to disable the cache or to enable pthreads. I would reccomend to disable
the cache, the performance gain when using multiple threads is huge, the performance gain
when using cache is  small. If you need more performance, just add more threads. You have
not to use cmsCreateTransformTHR, this is just a way to add a user-defined variable to the
handle, and finally cmsDoTransform does not have any ContexID, the error reports the
ContextID associated with the transform being used. As a hint, ContexID are more useful
when you want write a memory management plug-in to specialize memory mangement for
multithreading, as the memory management pluging does recive ContextID when a memory
operation is requested. The testebed application does use this feature to check memory
consistency.

Posted by Marti Maria

-----------

Saturday, June 26, 2010
Reusing same transform on different pixel types
I got this question twice, so here are some comments.

cmsChangeBuffersFormat() is gone in 2.0

There is a good reason to do that: optimization


When you create a transform, you supply the profiles and the expected buffer format. Then,
the engine, on depending on things like number of channels and bit depth can choose to
implement such transform in different ways.


 Let's take an example. If you create a AdobeRGB to  sRGB transform using TYPE_RGB_8 for
both input and output, the engine can guess that the maximum precision you would require
is 8 bits, and then simplify the curve and matrix handling to, for example 1.14 fixed
point.

 
This precision is enough for 8 bits but not for 16 bits, so if you change the format after
creating the transform to TYPE_RGB_16, you would end either with artifacts or throughput
loss.


Remember lcms 2 allows you to close the profiles after creating the transform. This is
very convenient feature but prevents to recalculate the transform by reading the profile
again. And there are situations, MPE for example when different precision means different
tags.


Overall I think the balancing of losing "change format" versus optimization and early
profile closing is good. Otherwise you can always create a new transform for each format.
Since you can close the profiles after creation, the amount of  allocated resources should
remain low. 


------------


Saturday, May 8, 2010
 I am pleased to the announce the release 2.0 of the LittleCMS open source color engine.

 Version 2.0 is an important milestone, among other improvements, it delivers: 

    * Full implementation of the ICC standard 
    * Improved documentation 
    * Better portability 
    * Easier extensibility


Migration to 2.x branch is highly encouraged. 
 

Little CMS intends to be a small-footprint color management engine, with special focus on accuracy and performance. It uses the International Color Consortium standard (ICC), which is the modern standard when regarding to color management. The ICC specification is widely used and is referred to in many International and other de-facto standards.
 
For more information, please take a look on: 

Main site: 
http://www.littlecms.com/
Downloads:
http://www.littlecms.com/download.html

 
Posted by Marti Maria at 7:16 PM 5 comments
Labels: release 


----------------

Monday, November 30, 2009
Backwards compatibility
Little CMS 2 is almost a full rewrite of 1.x series, so there is no guarantee of backwards
compatibility. Having said this, if your application doesnât make use of advanced
features, probably all what you need to do is to change the include file from lcms.h to
lcms2.h and maybe to do some minor tweaks on your code. Profile opening and transform
creation functions are kept the same, but there are some changes in the flags.  Little CMS
2 does offer more ways to access profiles, so it is certainly possible your code will get
simplified.  The basic parts where Little CMS 2 differs from 1.x series are:

    * Â·    Transform flags
    * Â·    Error handling
    * Â·    Textual information retrieval
    * Â·    New non-ICC intents
    * Â·    Floating point modes
    * Â·    Pipelines



On internal advanced functions, the underlying implementation has changed significantly.
You still can do all what lcms1 did, but in some cases by using a different approach.
There are no longer gamma curves or matrix-shaper functions. Even the LUT functions are
gone. All that has been superseded by:

    * Â·    Gamma functions -> Tone curves
    * Â·    Matrix Shaper, LUT -> Pipelines
    * Â·    LUT resampling -> Optimization engine

There is no one-to-one correspondence between old and new functions, but most old
functionality can be implemented with new functions.
Posted by Marti Maria at 7:01 PM 0 comments
Labels: documentation, icc 


------------------

Sunday, November 29, 2009
What is new from lcms 1.x
First obvious question is âwhy should I upgrade to Little CMS 2.0â. Here are some clues:

Little CMS 2.0 is a full v4 CMM, which can accept v2 profiles. Little CMS 1.xx was a v2
CMM which can deal with (some) V4 profiles. The difference is important, as 2.0 handling
of PCS is different, definitively better and far more accurate.

    * It does accept and understand floating point profiles (MPE) with DToBxx tags. (Yes,
it works!) It has 32 bits precision. (lcms 1.xx was 16 bits)
    * It handles float and double formats directly. MPE profiles are evaluated in floating
point with no precision loss.
    * It has plug-in architecture that allows you to change interpolation, add new
proprietary tags, add new âsmart CMMâ intents, etc.
    * Is faster. In some combinations, has a x 6 throughput boost.
    * Some new algorithms, incomplete state of adaptation, Jan Morovicâs segment maxima
gamut boundary descriptor, better K preservationâ¦
    * Historic issues, like faulty icc34.h, freeing profiles after creating transform,
etc. All is solved.

Posted by Marti Maria at 5:25 PM 0 comments
Labels: documentation, speed 

*/

#include <config-digikam.h>

#if defined(USE_LCMS_VERSION_1000)

#include <lcms.h>
#if LCMS_VERSION < 114
#define cmsTakeCopyright(profile) "Unknown"
#endif // LCMS_VERSION < 114


#define dkCmsCloseProfile            cmsCloseProfile
#define dkCmsCreateProofingTransform cmsCreateProofingTransform
#define dkCmsCreateTransform         cmsCreateTransform
#define dkCmsCreateXYZProfile        cmsCreateXYZProfile
#define dkCmsCreate_sRGBProfile      cmsCreate_sRGBProfile
#define dkCmsDeleteTransform         cmsDeleteTransform
#define dkCmsDeltaE                  cmsDeltaE
#define dkCmsDoTransform             cmsDoTransform
#define dkCmsErrorAction             cmsErrorAction
#define dkCmsFloat2XYZEncoded        cmsFloat2XYZEncoded
#define dkCmsGetColorSpace           cmsGetColorSpace
#define dkCmsGetDeviceClass          cmsGetDeviceClass
#define dkCmsGetPCS                  cmsGetPCS
#define dkCmsGetProfileICCversion    cmsGetProfileICCversion
#define dkCmsIsTag                   cmsIsTag
#define dkCmsOpenProfileFromFile     cmsOpenProfileFromFile
#define dkCmsOpenProfileFromMem      cmsOpenProfileFromMem
#define dkCmsSetAlarmCodes           cmsSetAlarmCodes
#define dkCmsTakeCharTargetData      cmsTakeCharTargetData
#define dkCmsTakeCopyright           cmsTakeCopyright
#define dkCmsTakeHeaderFlags         cmsTakeHeaderFlags
#define dkCmsTakeManufacturer        cmsTakeManufacturer
#define dkCmsTakeMediaWhitePoint     cmsTakeMediaWhitePoint
#define dkCmsTakeModel               cmsTakeModel
#define dkCmsTakeProductDesc         cmsTakeProductDesc
#define dkCmsTakeProductInfo         cmsTakeProductInfo
#define dkCmsTakeProductName         cmsTakeProductName
#define dkCmsTakeProfileID           cmsTakeProfileID
#define dkCmsTakeRenderingIntent     cmsTakeRenderingIntent
#define dkCmsXYZ2xyY                 cmsXYZ2xyY
#define dkCmsXYZEncoded2Float        cmsXYZEncoded2Float
#define dkCmsAdaptMatrixFromD50      cmsAdaptMatrixFromD50
#define dkCmsReadICCMatrixRGB2XYZ    cmsReadICCMatrixRGB2XYZ


#endif // defined(USE_LCMS_VERSION_000)


#if defined(USE_LCMS_VERSION_2000)

#define CMS_USE_CPP_API 1
#include <lcms2.h> 

#define LCMS_DESC_MAX     512

#define ZeroMemory(p,l)     memset((p),0,(l))
#define CopyMemory(d,s,l)   memcpy((d),(s),(l))
#define FAR

#define LCMS_ERROR_SHOW     1
#define cmsFLAGS_NOTPRECALC               0x0100
#define cmsFLAGS_WHITEBLACKCOMPENSATION   0x2000

#define NON_WINDOWS // TODO:
#ifdef NON_WINDOWS
#  define LCMSEXPORT
#  define LCMSAPI
#else
#  ifdef LCMS_DLL
#    define LCMSEXPORT  _stdcall
#    ifdef LCMS_DLL_BUILD
#        define LCMSAPI     __declspec(dllexport)
#    else
#        define LCMSAPI     __declspec(dllimport)
#    endif
#  else
#    define LCMSEXPORT cdecl
#    define LCMSAPI
#  endif
#endif


typedef int   LCMSBOOL;
typedef unsigned char BYTE, *LPBYTE; 
typedef unsigned short WORD, *LPWORD;
typedef unsigned long DWORD, *LPDWORD;
typedef void *LPVOID;

// Colorspaces
typedef cmsCIEXYZ FAR* LPcmsCIEXYZ;
typedef cmsCIExyY FAR* LPcmsCIExyY;
typedef cmsCIELab FAR* LPcmsCIELab;

typedef void* cmsHPROFILE;             // Opaque typedefs to hide internals

// Vectors
typedef struct {                // Float Vector
    double n[3];
} VEC3;

typedef struct {                // Matrix
    VEC3 v[3];
} MAT3; 

typedef MAT3 FAR* LPMAT3;

/* profileClass enumerations */
typedef enum {
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
typedef enum {
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
typedef enum {
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

LCMSAPI int    LCMSEXPORT dkCmsErrorAction(int nAction);

LCMSAPI DWORD  LCMSEXPORT dkCmsGetProfileICCversion(cmsHPROFILE hProfile);

LCMSEXPORT void dkCmsSetAlarmCodes(int r, int g, int b);

LCMSAPI const char*   LCMSEXPORT dkCmsTakeProductName(cmsHPROFILE hProfile);

LCMSAPI const char*   LCMSEXPORT dkCmsTakeProductDesc(cmsHPROFILE hProfile);

LCMSAPI const char*   LCMSEXPORT dkCmsTakeProductInfo(cmsHPROFILE hProfile);

LCMSAPI const char*   LCMSEXPORT dkCmsTakeManufacturer(cmsHPROFILE hProfile);

LCMSAPI LCMSBOOL      LCMSEXPORT dkCmsTakeMediaWhitePoint(LPcmsCIEXYZ Dest, cmsHPROFILE hProfile);

LCMSAPI const char*   LCMSEXPORT dkCmsTakeModel(cmsHPROFILE hProfile);

LCMSAPI const char*   LCMSEXPORT dkCmsTakeCopyright(cmsHPROFILE hProfile);

LCMSAPI DWORD         LCMSEXPORT dkCmsTakeHeaderFlags(cmsHPROFILE hProfile);

LCMSAPI const BYTE*   LCMSEXPORT dkCmsTakeProfileID(cmsHPROFILE hProfile);

LCMSAPI LCMSBOOL      LCMSEXPORT dkCmsTakeCreationDateTime(struct tm *Dest, cmsHPROFILE hProfile);

LCMSAPI LCMSBOOL      LCMSEXPORT dkCmsTakeCalibrationDateTime(struct tm *Dest, cmsHPROFILE hProfile);

LCMSAPI LCMSBOOL      LCMSEXPORT dkCmsIsTag(cmsHPROFILE hProfile, icTagSignature sig);

LCMSAPI int           LCMSEXPORT dkCmsTakeRenderingIntent(cmsHPROFILE hProfile);

LCMSAPI LCMSBOOL      LCMSEXPORT dkCmsTakeCharTargetData(cmsHPROFILE hProfile, char** Data, size_t* len);

LCMSBOOL                         dkCmsAdaptMatrixFromD50(LPMAT3 r, LPcmsCIExyY DestWhitePt);

LCMSBOOL                         dkCmsReadICCMatrixRGB2XYZ(LPMAT3 r, cmsHPROFILE hProfile);

LCMSAPI cmsHPROFILE   LCMSEXPORT dkCmsOpenProfileFromMem(LPVOID MemPtr, DWORD dwSize);

LCMSAPI icProfileClassSignature LCMSEXPORT dkCmsGetDeviceClass(cmsHPROFILE hProfile);

LCMSAPI LCMSBOOL      LCMSEXPORT dkCmsCloseProfile(cmsHPROFILE hProfile);

LCMSAPI cmsHTRANSFORM LCMSEXPORT dkCmsCreateProofingTransform(cmsHPROFILE Input,
                                               DWORD InputFormat,
                                               cmsHPROFILE Output,
                                               DWORD OutputFormat,
                                               cmsHPROFILE Proofing,
                                               int Intent,
                                               int ProofingIntent,
                                               DWORD dwFlags);

LCMSAPI cmsHTRANSFORM LCMSEXPORT dkCmsCreateTransform(cmsHPROFILE Input,
                                               DWORD InputFormat,
                                               cmsHPROFILE Output,
                                               DWORD OutputFormat,
                                               int Intent,
                                               DWORD dwFlags);

LCMSAPI cmsHPROFILE   LCMSEXPORT dkCmsCreateXYZProfile(void);

LCMSAPI cmsHPROFILE   LCMSEXPORT dkCmsCreate_sRGBProfile(void);

LCMSAPI void          LCMSEXPORT dkCmsDeleteTransform(cmsHTRANSFORM hTransform);

LCMSAPI double        LCMSEXPORT dkCmsDeltaE(LPcmsCIELab Lab1, LPcmsCIELab Lab2);

LCMSAPI void          LCMSEXPORT dkCmsDoTransform(cmsHTRANSFORM Transform,
                                                  LPVOID InputBuffer,
                                                  LPVOID OutputBuffer,
                                                  unsigned int Size);

LCMSAPI void                    LCMSEXPORT dkCmsFloat2XYZEncoded(WORD XYZ[3], const cmsCIEXYZ* fXYZ);

LCMSAPI icColorSpaceSignature   LCMSEXPORT dkCmsGetColorSpace(cmsHPROFILE hProfile);

LCMSAPI icColorSpaceSignature   LCMSEXPORT dkCmsGetPCS(cmsHPROFILE hProfile);

LCMSAPI LCMSBOOL                LCMSEXPORT dkCmsIsTag(cmsHPROFILE hProfile, icTagSignature sig);

LCMSAPI cmsHPROFILE             LCMSEXPORT dkCmsOpenProfileFromFile(const char *ICCProfile, const char *sAccess);

LCMSAPI void                    LCMSEXPORT dkCmsXYZ2xyY(LPcmsCIExyY Dest, const cmsCIEXYZ* Source);

LCMSAPI void                    LCMSEXPORT dkCmsXYZEncoded2Float(LPcmsCIEXYZ fxyz, const WORD XYZ[3]);


#endif // defined(USE_LCMS_VERSION_2000)

#endif // DIGIKAM_LCMS_H
