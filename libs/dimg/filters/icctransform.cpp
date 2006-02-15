/* ============================================================
 * Author: F.J. Cruz <fj.cruz@supercable.es>
 * Date  : 2005-11-18
 * Copyright 2005-2006 by F.J. Cruz
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

// Littlecms library includes.

#include <config.h>
#include LCMS_HEADER

// QT includes

#include <qstring.h>
#include <qcstring.h>
#include <qfile.h>

// KDE includes

#include <kdebug.h>
#include <kconfig.h>
#include <kapplication.h>


// Local includes

#include "icctransform.h"

namespace Digikam
{

class IccTransformPriv
{
public:

    IccTransformPriv()
    {
        has_profile      = false;
        do_proof_profile = false;
    }

    bool       do_proof_profile;
    bool       has_profile; 
    
    QString    input_profile;
    QString    output_profile;
    QString    proof_profile;
    
    QByteArray embedded_profile;
};

IccTransform::IccTransform()
{
    d = new IccTransformPriv;
}

IccTransform::~IccTransform()
{
    delete d;
}

void IccTransform::getTransformType(bool do_proof_profile)
{
    if (do_proof_profile)
    {
        d->do_proof_profile = true;
    }
    else
    {
        d->do_proof_profile = false;
    }
}

void IccTransform::getEmbeddedProfile(DImg image)
{
    if (!image.getICCProfil().isNull())
    {
        d->embedded_profile = image.getICCProfil();
        d->has_profile = true;
    }
    else
    {
        return;
    }
}

void IccTransform::setProfiles(QString input_profile, QString output_profile)
{
    d->input_profile  = input_profile;
    d->output_profile = output_profile;
}

void IccTransform::setProfiles(QString input_profile, QString output_profile, 
                               QString proof_profile)
{
    d->input_profile  = input_profile;
    d->output_profile = output_profile;
    d->proof_profile  = proof_profile;
}

void IccTransform::setProfiles(QString output_profile)
{
    d->output_profile = output_profile;
}

void IccTransform::setProfiles( QString output_profile, QString proof_profile, bool forProof )
{
    if (forProof)
    {
        d->output_profile = output_profile;
        d->proof_profile  = proof_profile;
    }
}

QString IccTransform::getEmbeddedProfileDescriptor()
{
    cmsHPROFILE tmpProfile = cmsOpenProfileFromMem(d->embedded_profile.data(), (DWORD)d->embedded_profile.size());
    QString embeddedProfileDescriptor =QString(cmsTakeProductDesc(tmpProfile));
    cmsCloseProfile(tmpProfile);
    return embeddedProfileDescriptor;
}

void IccTransform::apply(DImg& image)
{
    cmsHPROFILE   inprofile=0, outprofile=0, proofprofile=0;
    cmsHTRANSFORM transform;
    int inputFormat = 0;
    int intent;

    switch (getRenderingIntent())
    {
        case 0:
            intent = INTENT_PERCEPTUAL;
            break;
        case 1:
            intent = INTENT_RELATIVE_COLORIMETRIC;
            break;
        case 2:
            intent = INTENT_SATURATION;
            break;
        case 3:
            intent = INTENT_ABSOLUTE_COLORIMETRIC;
            break;
    }
    kdDebug() << "intent: " << getRenderingIntent() << endl;

    if (d->has_profile)
    {
        inprofile = cmsOpenProfileFromMem(d->embedded_profile.data(),
                                          (DWORD)d->embedded_profile.size());
    }
    else
    {
        inprofile = cmsOpenProfileFromFile(QFile::encodeName( d->input_profile ), "r");
    }

    outprofile = cmsOpenProfileFromFile(QFile::encodeName( d->output_profile ), "r");

    if (!d->do_proof_profile)
    {
        if (image.sixteenBit())
        {
            if (image.hasAlpha())
            {
                switch (cmsGetColorSpace(inprofile))
                 {
                     case icSigGrayData:
                         inputFormat = TYPE_GRAYA_16;
                         break;
                     case icSigCmykData:
                         inputFormat = TYPE_CMYK_16;
                         break;
                     default:
                         inputFormat = TYPE_BGRA_16;
                 }
                transform = cmsCreateTransform( inprofile,
                                                inputFormat,
                                                outprofile,
                                                TYPE_BGRA_16,
                                                intent,
                                                cmsFLAGS_WHITEBLACKCOMPENSATION);
            }
            else
            {
                switch (cmsGetColorSpace(inprofile))
                 {
                     case icSigGrayData:
                         inputFormat = TYPE_GRAY_16;
                         break;
                     case icSigCmykData:
                         inputFormat = TYPE_CMYK_16;
                         break;
                     default:
                         inputFormat = TYPE_BGR_16;
                 }
                transform = cmsCreateTransform( inprofile,
                                                inputFormat,
                                                outprofile,
                                                TYPE_BGR_16,
                                                intent,
                                                cmsFLAGS_WHITEBLACKCOMPENSATION);
            }

        }
        else
        {
            if (image.hasAlpha())
            {
                switch (cmsGetColorSpace(inprofile))
                 {
                     case icSigGrayData:
                         inputFormat = TYPE_GRAYA_8;
                         break;
                     case icSigCmykData:
                         inputFormat = TYPE_CMYK_8;
                         break;
                     default:
                         inputFormat = TYPE_BGRA_8;
                 }
                transform = cmsCreateTransform( inprofile,
                                                inputFormat,
                                                outprofile,
                                                TYPE_BGRA_8,
                                                intent,
                                                cmsFLAGS_WHITEBLACKCOMPENSATION);
            }
            else
            {
                switch (cmsGetColorSpace(inprofile))
                 {
                     case icSigGrayData:
                         inputFormat = TYPE_GRAYA_8;
                         break;
                     case icSigCmykData:
                         inputFormat = TYPE_CMYK_8;
                         kdDebug() << "input profile: cmyk no alpha" << endl;
                         break;
                     default:
                         inputFormat = TYPE_BGR_8;
                         kdDebug() << "input profile: default no alpha" << endl;
                 }
                transform = cmsCreateTransform( inprofile,
                                                inputFormat,
                                                outprofile,
                                                TYPE_BGR_8,
                                                intent,
                                                cmsFLAGS_WHITEBLACKCOMPENSATION);
            }

        }
    }
    else
    {
        proofprofile = cmsOpenProfileFromFile(QFile::encodeName( d->proof_profile ), "r");

        if (image.sixteenBit())
        {
            if (image.hasAlpha())
            {
                transform = cmsCreateProofingTransform( inprofile,
                                                        TYPE_BGRA_16,
                                                        outprofile,
                                                        TYPE_BGRA_16,
                                                        proofprofile,
                                                        INTENT_ABSOLUTE_COLORIMETRIC,
                                                        INTENT_ABSOLUTE_COLORIMETRIC,
                                                        cmsFLAGS_WHITEBLACKCOMPENSATION);
            }
            else
            {
                transform = cmsCreateProofingTransform( inprofile,
                                                        TYPE_BGR_16,
                                                        outprofile,
                                                        TYPE_BGR_16,
                                                        proofprofile,
                                                        INTENT_ABSOLUTE_COLORIMETRIC,
                                                        INTENT_ABSOLUTE_COLORIMETRIC,
                                                        cmsFLAGS_WHITEBLACKCOMPENSATION);
            }
        }
        else
        {
            if (image.hasAlpha())
            {
                transform = cmsCreateProofingTransform( inprofile,
                                                        TYPE_BGR_8,
                                                        outprofile,
                                                        TYPE_BGR_8,
                                                        proofprofile,
                                                        INTENT_ABSOLUTE_COLORIMETRIC,
                                                        INTENT_ABSOLUTE_COLORIMETRIC,
                                                        cmsFLAGS_WHITEBLACKCOMPENSATION);
            }
            else
            {
                transform = cmsCreateProofingTransform( inprofile,
                                                        TYPE_BGR_8,
                                                        outprofile,
                                                        TYPE_BGR_8,
                                                        proofprofile,
                                                        INTENT_ABSOLUTE_COLORIMETRIC,
                                                        INTENT_ABSOLUTE_COLORIMETRIC,
                                                        cmsFLAGS_WHITEBLACKCOMPENSATION);
            }
        }
    }

     // We need to work using temp pixel buffer to apply ICC transformations.
    uchar  transdata[image.bytesDepth()];
    
    // Always working with uchar* prevent endianess problem.
    uchar *data = image.bits();

    // We scan all image pixels one by one.
    for (uint i=0; i < image.width()*image.height()*image.bytesDepth(); i+=image.bytesDepth())
    {
        // Apply ICC transformations.
        cmsDoTransform( transform, &data[i], &transdata[0], 1);
        
        // Copy buufer to source to update original image with ICC corrections.
        // Alpha channel is restored in all cases.
        memcpy (&data[i], &transdata[0], (image.bytesDepth() == 8) ? 6 : 3);        
    }
    
    cmsDeleteTransform(transform);
    cmsCloseProfile(inprofile);
    cmsCloseProfile(outprofile);
    
    if (d->do_proof_profile)
       cmsCloseProfile(proofprofile);
}

void IccTransform::apply( DImg& image, QByteArray& profile, int intent, bool useBPC, bool checkGamut, bool useBuiltin )
{

    cmsHPROFILE   inprofile=0, outprofile=0, proofprofile=0;
    cmsHTRANSFORM transform;
    int transformFlags = 0, inputFormat = 0;
    int renderingIntent;

    switch (intent)
    {
        case 0:
            intent = INTENT_PERCEPTUAL;
            break;
        case 1:
            intent = INTENT_RELATIVE_COLORIMETRIC;
            break;
        case 2:
            intent = INTENT_SATURATION;
            break;
        case 3:
            intent = INTENT_ABSOLUTE_COLORIMETRIC;
            break;
    }

    kdDebug() << "icctransform.cpp/301-Intent is: " << intent << endl;

    if (!profile.isNull())
    {
        inprofile = cmsOpenProfileFromMem(profile.data(),
                                          (DWORD)profile.size());
    }
    else if (useBuiltin)
    {
        inprofile = cmsCreate_sRGBProfile();
    }
    else
    {
        inprofile = cmsOpenProfileFromFile(QFile::encodeName( d->input_profile ), "r");
    }

    outprofile = cmsOpenProfileFromFile(QFile::encodeName( d->output_profile ),
"r");

    if (useBPC)
    {
        transformFlags |= cmsFLAGS_WHITEBLACKCOMPENSATION;
    }
    
    if (!d->do_proof_profile)
    {
        if (image.sixteenBit())
        {
            if (image.hasAlpha())
            {
                switch (cmsGetColorSpace(inprofile))
                 {
                     case icSigGrayData:
                         inputFormat = TYPE_GRAYA_16;
                         break;
                     case icSigCmykData:
                         inputFormat = TYPE_CMYK_16;
                         break;
                     default:
                         inputFormat = TYPE_BGRA_16;
                 }
                transform = cmsCreateTransform( inprofile,
                                                inputFormat,
                                                outprofile,
                                                TYPE_BGRA_16,
                                                intent,
                                                transformFlags);
            }
            else
            {
                switch (cmsGetColorSpace(inprofile))
                 {
                     case icSigGrayData:
                         inputFormat = TYPE_GRAY_16;
                         break;
                     case icSigCmykData:
                         inputFormat = TYPE_CMYK_16;
                         break;
                     default:
                         inputFormat = TYPE_BGR_16;
                 }
                transform = cmsCreateTransform( inprofile,
                                                inputFormat,
                                                outprofile,
                                                TYPE_BGR_16,
                                                intent,
                                                transformFlags);
            }

        }
        else
        {
            if (image.hasAlpha())
            {
                switch (cmsGetColorSpace(inprofile))
                 {
                     case icSigGrayData:
                         inputFormat = TYPE_GRAYA_8;
                         break;
                     case icSigCmykData:
                         inputFormat = TYPE_CMYK_8;
                         break;
                     default:
                         inputFormat = TYPE_BGRA_8;
                 }
                transform = cmsCreateTransform( inprofile,
                                                inputFormat,
                                                outprofile,
                                                TYPE_BGRA_8,
                                                intent,
                                                transformFlags);
            }
            else
            {
                switch (cmsGetColorSpace(inprofile))
                 {
                     case icSigGrayData:
                         inputFormat = TYPE_GRAY_8;
                         break;
                     case icSigCmykData:
                         inputFormat = TYPE_CMYK_8;
                         break;
                     default:
                         inputFormat = TYPE_BGR_8;
                 }
                transform = cmsCreateTransform( inprofile,
                                                inputFormat,
                                                outprofile,
                                                TYPE_BGR_8,
                                                intent,
                                                transformFlags);
            }

        }
    }
    else
    {
        proofprofile = cmsOpenProfileFromFile(QFile::encodeName( d->proof_profile ), "r");

        transformFlags |= cmsFLAGS_SOFTPROOFING;
        if (checkGamut)
        {
            cmsSetAlarmCodes(126, 255, 255);
            transformFlags |= cmsFLAGS_GAMUTCHECK;
        }

        if (image.sixteenBit())
        {
            if (image.hasAlpha())
            {
                transform = cmsCreateProofingTransform( inprofile,
                                                        TYPE_BGRA_16,
                                                        outprofile,
                                                        TYPE_BGRA_16,
                                                        proofprofile,
                                                        intent,
                                                        intent,
                                                        transformFlags);
            }
            else
            {
                transform = cmsCreateProofingTransform( inprofile,
                                                        TYPE_BGR_16,
                                                        outprofile,
                                                        TYPE_BGR_16,
                                                        proofprofile,
                                                        intent,
                                                        intent,
                                                        transformFlags);
            }
        }
        else
        {
            if (image.hasAlpha())
            {
                transform = cmsCreateProofingTransform( inprofile,
                                                        TYPE_BGR_8,
                                                        outprofile,
                                                        TYPE_BGR_8,
                                                        proofprofile,
                                                        intent,
                                                        intent,
                                                        transformFlags);
            }
            else
            {
                transform = cmsCreateProofingTransform( inprofile,
                                                        TYPE_BGR_8,
                                                        outprofile,
                                                        TYPE_BGR_8,
                                                        proofprofile,
                                                        intent,
                                                        intent,
                                                        transformFlags);
            }
        }
    }

    kdDebug() << "icctransform.cpp/439 Transform flags are: " << transformFlags << endl;

     // We need to work using temp pixel buffer to apply ICC transformations.
    uchar  transdata[image.bytesDepth()];
    
    // Always working with uchar* prevent endianess problem.
    uchar *data = image.bits();

    // We scan all image pixels one by one.
    for (uint i=0; i < image.width()*image.height()*image.bytesDepth(); i+=image.bytesDepth())
    {
        // Apply ICC transformations.
        cmsDoTransform( transform, &data[i], &transdata[0], 1);
        
        // Copy buufer to source to update original image with ICC corrections.
        // Alpha channel is restored in all cases.
        memcpy (&data[i], &transdata[0], (image.bytesDepth() == 8) ? 6 : 3);        
    }
    
    cmsDeleteTransform(transform);
    cmsCloseProfile(inprofile);
    cmsCloseProfile(outprofile);
    
    if (d->do_proof_profile)
       cmsCloseProfile(proofprofile);
}

QString IccTransform::getProfileDescription(QString profile)
{

    cmsHPROFILE _profile = cmsOpenProfileFromFile(QFile::encodeName(profile), "r");
    QString _description = cmsTakeProductDesc(_profile);
    cmsCloseProfile(_profile);
    return _description;

}

int IccTransform::getRenderingIntent()
{
    KConfig* config = kapp->config();

    config->setGroup("Color Management");

    return config->readNumEntry("RenderingIntent", 0);
}

}  // NameSpace Digikam
