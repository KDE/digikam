/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-11-18
 * Description : a class to apply ICC color correction to image.
 *
 * Copyright (C) 2005-2006 by F.J. Cruz <fj dot cruz at supercable dot es>
 * Copyright (C) 2009      by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "icctransform.h"
#include "digikam-lcms.h"

// Qt includes

#include <QDataStream>
#include <QFile>
#include <QImage>
#include <QVarLengthArray>

// KDE includes

#include <kconfiggroup.h>


// Local includes

#include "digikam_debug.h"
#include "dimgloaderobserver.h"

namespace Digikam
{

class TransformDescription
{
public:

    TransformDescription()
    {
        inputFormat    = 0;
        outputFormat   = 0;
        intent         = INTENT_PERCEPTUAL;
        transformFlags = 0;
        proofIntent    = INTENT_ABSOLUTE_COLORIMETRIC;
    }

    bool operator==(const TransformDescription& other) const
    {
        return inputProfile   == other.inputProfile   &&
               inputFormat    == other.inputFormat    &&
               outputProfile  == other.outputProfile  &&
               outputFormat   == other.outputFormat   &&
               intent         == other.intent         &&
               transformFlags == other.transformFlags &&
               proofProfile   == other.proofProfile   &&
               proofIntent    == other.proofIntent;
    }

public:

    IccProfile inputProfile;
    int        inputFormat;
    IccProfile outputProfile;
    int        outputFormat;
    int        intent;
    int        transformFlags;
    IccProfile proofProfile;
    int        proofIntent;
};

class IccTransform::Private : public QSharedData
{
public:

    Private()
    {
        intent          = IccTransform::Perceptual;
        proofIntent     = IccTransform::AbsoluteColorimetric;
        useBPC          = false;
        checkGamut      = false;
        doNotEmbed      = false;
        checkGamutColor = QColor(126, 255, 255);
        handle          = 0;
    }

    Private(const Private& other)
        : QSharedData(other)
    {
        handle = 0;
        operator=(other);
    }

    Private& operator=(const Private& other)
    {
        // Attention: This is sensitive. Add any new members here.
        // We can't use the default operator= because of handle.
        intent             = other.intent;
        proofIntent        = other.proofIntent;
        useBPC             = other.useBPC;
        checkGamut         = other.checkGamut;
        doNotEmbed         = other.doNotEmbed;
        checkGamutColor    = other.checkGamutColor;

        embeddedProfile    = other.embeddedProfile;
        inputProfile       = other.inputProfile;
        outputProfile      = other.outputProfile;
        proofProfile       = other.proofProfile;
        builtinProfile     = other.builtinProfile;

        close();
        handle             = 0;
        currentDescription = TransformDescription();

        return *this;
    }

    ~Private()
    {
        close();
    }

    void close()
    {
        if (handle)
        {
            currentDescription = TransformDescription();
            LcmsLock lock;
            dkCmsDeleteTransform(handle);
            handle = 0;
        }
    }

    IccProfile& sRGB()
    {
        if (builtinProfile.isNull())
        {
            builtinProfile = IccProfile::sRGB();
        }

        return builtinProfile;
    }

    IccProfile& effectiveInputProfile()
    {
        if (!embeddedProfile.isNull())
        {
            return embeddedProfile;
        }
        else if (!inputProfile.isNull())
        {
            return inputProfile;
        }
        else
        {
            return sRGB();
        }
    }

    IccProfile effectiveInputProfileConst() const
    {
        if (!embeddedProfile.isNull())
        {
            return embeddedProfile;
        }
        else if (!inputProfile.isNull())
        {
            return inputProfile;
        }
        else
        {
            return IccProfile::sRGB();
        }
    }

public:

    IccTransform::RenderingIntent intent;
    IccTransform::RenderingIntent proofIntent;
    bool                          useBPC;
    bool                          checkGamut;
    bool                          doNotEmbed;
    QColor                        checkGamutColor;

    IccProfile                    embeddedProfile;
    IccProfile                    inputProfile;
    IccProfile                    outputProfile;
    IccProfile                    proofProfile;
    IccProfile                    builtinProfile;

    cmsHTRANSFORM                 handle;
    TransformDescription          currentDescription;
};

IccTransform::IccTransform()
    : d(new Private)
{
}

IccTransform::IccTransform(const IccTransform& other)
    : d(other.d)
{
}

IccTransform& IccTransform::operator=(const IccTransform& other)
{
    d = other.d;
    return *this;
}

IccTransform::~IccTransform()
{
    // close() is done in ~Private
}

void IccTransform::init()
{
    LcmsLock lock;
    dkCmsErrorAction(LCMS_ERROR_SHOW);
}

void IccTransform::setInputProfile(const IccProfile& profile)
{
    if (profile == d->inputProfile)
    {
        return;
    }

    close();
    d->inputProfile = profile;
}

void IccTransform::setEmbeddedProfile(const DImg& image)
{
    IccProfile profile = image.getIccProfile();

    if (profile == d->embeddedProfile)
    {
        return;
    }

    close();
    d->embeddedProfile = profile;
}

void IccTransform::setOutputProfile(const IccProfile& profile)
{
    if (profile == d->outputProfile)
    {
        return;
    }

    close();
    d->outputProfile = profile;
}

void IccTransform::setProofProfile(const IccProfile& profile)
{
    if (profile == d->proofProfile)
    {
        return;
    }

    close();
    d->proofProfile = profile;
}

IccProfile IccTransform::embeddedProfile() const
{
    return d->embeddedProfile;
}

IccProfile IccTransform::inputProfile() const
{
    return d->inputProfile;
}

IccProfile IccTransform::outputProfile() const
{
    return d->outputProfile;
}

IccProfile IccTransform::proofProfile() const
{
    return d->proofProfile;
}

IccProfile IccTransform::effectiveInputProfile() const
{
    return d->effectiveInputProfileConst();
}

void IccTransform::setIntent(RenderingIntent intent)
{
    if (intent == d->intent)
    {
        return;
    }

    d->intent = intent;
    close();
}

void IccTransform::setProofIntent(RenderingIntent intent)
{
    if (intent == d->proofIntent)
    {
        return;
    }

    d->proofIntent = intent;
    close();
}

void IccTransform::setUseBlackPointCompensation(bool useBPC)
{
    if (d->useBPC == useBPC)
    {
        return;
    }

    close();
    d->useBPC = useBPC;
}

void IccTransform::setCheckGamut(bool checkGamut)
{
    if (d->checkGamut == checkGamut)
    {
        return;
    }

    close();
    d->checkGamut = checkGamut;
}

void IccTransform::setCheckGamutMaskColor(const QColor& color)
{
    d->checkGamutColor = color;
}

IccTransform::RenderingIntent IccTransform::intent() const
{
    return d->intent;
}

IccTransform::RenderingIntent IccTransform::proofIntent() const
{
    return d->proofIntent;
}

bool IccTransform::isUsingBlackPointCompensation() const
{
    return d->useBPC;
}

bool IccTransform::isCheckingGamut() const
{
    return d->checkGamut;
}

QColor IccTransform::checkGamutMaskColor() const
{
    return d->checkGamutColor;
}

void IccTransform::setDoNotEmbedOutputProfile(bool doNotEmbed)
{
    d->doNotEmbed = doNotEmbed;
}

/*
void IccTransform::readFromConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QString("Color Management"));

    int intent                = group.readEntry("RenderingIntent", 0);
    bool useBPC               = group.readEntry("BPCAlgorithm", false);

    setIntent(intent);
    setUseBlackPointCompensation(useBPC);
}
*/

bool IccTransform::willHaveEffect()
{
    if (d->outputProfile.isNull())
    {
        return false;
    }

    return !d->effectiveInputProfile().isSameProfileAs(d->outputProfile);
}

static int renderingIntentToLcmsIntent(IccTransform::RenderingIntent intent)
{
    switch (intent)
    {
        case IccTransform::Perceptual:
            return INTENT_PERCEPTUAL;

        case IccTransform::RelativeColorimetric:
            return INTENT_RELATIVE_COLORIMETRIC;

        case IccTransform::Saturation:
            return INTENT_SATURATION;

        case IccTransform::AbsoluteColorimetric:
            return INTENT_ABSOLUTE_COLORIMETRIC;

        default:
            return INTENT_PERCEPTUAL;
    }
}

TransformDescription IccTransform::getDescription(const DImg& image)
{
    TransformDescription description;

    description.inputProfile  = d->effectiveInputProfile();
    description.outputProfile = d->outputProfile;
    description.intent        = renderingIntentToLcmsIntent(d->intent);

    if (d->useBPC)
    {
        description.transformFlags |= cmsFLAGS_WHITEBLACKCOMPENSATION;
    }

    LcmsLock lock;

    // Do not use TYPE_BGR_ - this implies 3 bytes per pixel, but even if !image.hasAlpha(),
    // our image data has 4 bytes per pixel with the fourth byte filled with 0xFF.
    if (image.sixteenBit())
    {
/*
        switch (dkCmsGetColorSpace(description.inputProfile))
        {
            case icSigGrayData:
                description.inputFormat = TYPE_GRAYA_16;
                break;
            case icSigCmykData:
                description.inputFormat = TYPE_CMYK_16;
                break;
            default:
                description.inputFormat = TYPE_BGRA_16;
        }
*/

        // A Dimg is always BGRA, converted by the loader
        description.inputFormat  = TYPE_BGRA_16;
        description.outputFormat = TYPE_BGRA_16;
    }
    else
    {
        description.inputFormat  = TYPE_BGRA_8;
        description.outputFormat = TYPE_BGRA_8;
    }

    return description;
}

TransformDescription IccTransform::getDescription(const QImage&)
{
    TransformDescription description;

    description.inputProfile  = d->effectiveInputProfile();
    description.outputProfile = d->outputProfile;
    description.intent        = renderingIntentToLcmsIntent(d->intent);

    if (d->useBPC)
    {
        description.transformFlags |= cmsFLAGS_WHITEBLACKCOMPENSATION;
    }

    description.inputFormat  = TYPE_BGRA_8;
    description.outputFormat = TYPE_BGRA_8;

    return description;
}

TransformDescription IccTransform::getProofingDescription(const DImg& image)
{
    TransformDescription description = getDescription(image);

    description.proofProfile = d->proofProfile;
    description.proofIntent  = renderingIntentToLcmsIntent(d->proofIntent);

    description.transformFlags |= cmsFLAGS_SOFTPROOFING;

    if (d->checkGamut)
    {
        dkCmsSetAlarmCodes(d->checkGamutColor.red(), d->checkGamutColor.green(), d->checkGamutColor.blue());
        description.transformFlags |= cmsFLAGS_GAMUTCHECK;
    }

    return description;
}

bool IccTransform::open(TransformDescription& description)
{
    if (d->handle)
    {
        if (d->currentDescription == description)
        {
            return true;
        }
        else
        {
            close();
        }
    }

    d->currentDescription = description;

    LcmsLock lock;
    d->handle = dkCmsCreateTransform(description.inputProfile,
                                     description.inputFormat,
                                     description.outputProfile,
                                     description.outputFormat,
                                     description.intent,
                                     description.transformFlags);

    if (!d->handle)
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "LCMS internal error: cannot create a color transform instance";
        return false;
    }

    return true;
}

bool IccTransform::openProofing(TransformDescription& description)
{
    if (d->handle)
    {
        if (d->currentDescription == description)
        {
            return true;
        }
        else
        {
            close();
        }
    }

    d->currentDescription = description;

    LcmsLock lock;
    d->handle = dkCmsCreateProofingTransform(description.inputProfile,
                                             description.inputFormat,
                                             description.outputProfile,
                                             description.outputFormat,
                                             description.proofProfile,
                                             description.intent,
                                             description.proofIntent,
                                             description.transformFlags);

    if (!d->handle)
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "LCMS internal error: cannot create a color transform instance";
        return false;
    }

    return true;
}

bool IccTransform::checkProfiles()
{
    if (!d->effectiveInputProfile().open())
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Cannot open embedded profile";
        return false;
    }

    if (!d->outputProfile.open())
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Cannot open output profile";
        return false;
    }

    if (!d->proofProfile.isNull())
    {
        if (!d->proofProfile.open())
        {
            qCDebug(DIGIKAM_DIMG_LOG) << "Cannot open proofing profile";
            return false;
        }
    }

    return true;
}

bool IccTransform::apply(DImg& image, DImgLoaderObserver* const observer)
{
    if (!willHaveEffect())
    {
        if (!d->outputProfile.isNull() && !d->doNotEmbed)
        {
            image.setIccProfile(d->outputProfile);
        }

        return true;
    }

    if (!checkProfiles())
    {
        return false;
    }

    TransformDescription description;

    if (d->proofProfile.isNull())
    {
        description = getDescription(image);

        if (!open(description))
        {
            return false;
        }
    }
    else
    {
        description = getProofingDescription(image);

        if (!openProofing(description))
        {
            return false;
        }
    }

    if (observer)
    {
        observer->progressInfo(&image, 0.1F);
    }

    transform(image, description, observer);

    if (!d->doNotEmbed)
    {
        image.setIccProfile(d->outputProfile);
    }

    // if this was a RAW color image, it is no more
    image.removeAttribute(QLatin1String("uncalibratedColor"));

    return true;
}

bool IccTransform::apply(QImage& qimage)
{
    if (qimage.format() != QImage::Format_RGB32  &&
        qimage.format() != QImage::Format_ARGB32 &&
        qimage.format() != QImage::Format_ARGB32_Premultiplied)
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Unsupported QImage format" << qimage.format();
        return false;
    }

    if (!willHaveEffect())
    {
        return true;
    }

    if (!checkProfiles())
    {
        return false;
    }

    TransformDescription description;
    description = getDescription(qimage);

    if (!open(description))
    {
        return false;
    }

    transform(qimage, description);

    return true;
}

void IccTransform::transform(DImg& image, const TransformDescription& description, DImgLoaderObserver* const observer)
{
    const int bytesDepth    = image.bytesDepth();
    const int pixels        = image.width() * image.height();
    // convert ten scanlines in a batch
    const int pixelsPerStep = image.width() * 10;
    uchar* data             = image.bits();

    // see dimgloader.cpp, granularity().
    int granularity         = 1;

    if (observer)
    {
        granularity = (int)((pixels / (20 * 0.9)) / observer->granularity());
    }

    int checkPoint = pixels;

    // it is safe to use the same input and output buffer if the format is the same
    if (description.inputFormat == description.outputFormat)
    {
        for (int p = pixels; p > 0; p -= pixelsPerStep)
        {
            int pixelsThisStep =  qMin(p, pixelsPerStep);
            int size           =  pixelsThisStep * bytesDepth;
            LcmsLock lock;
            dkCmsDoTransform(d->handle, data, data, pixelsThisStep);
            data               += size;

            if (observer && p <= checkPoint)
            {
                checkPoint -= granularity;
                observer->progressInfo(&image, 0.1 + 0.9 * (1.0 - float(p) / float(pixels)));
            }
        }
    }
    else
    {
        QVarLengthArray<uchar> buffer(pixelsPerStep * bytesDepth);

        for (int p = pixels; p > 0; p -= pixelsPerStep)
        {
            int pixelsThisStep  = qMin(p, pixelsPerStep);
            int size            = pixelsThisStep * bytesDepth;
            LcmsLock lock;
            memcpy(buffer.data(), data, size);
            dkCmsDoTransform(d->handle, buffer.data(), data, pixelsThisStep);
            data               += size;

            if (observer && p <= checkPoint)
            {
                checkPoint -= granularity;
                observer->progressInfo(&image, 0.1 + 0.9 * (1.0 - float(p) / float(pixels)));
            }
        }
    }
}

void IccTransform::transform(QImage& image, const TransformDescription&)
{
    const int bytesDepth    = 4;
    const int pixels        = image.width() * image.height();
    // convert ten scanlines in a batch
    const int pixelsPerStep = image.width() * 10;
    uchar* data             = image.bits();

    for (int p = pixels; p > 0; p -= pixelsPerStep)
    {
        int pixelsThisStep =  qMin(p, pixelsPerStep);
        int size           =  pixelsThisStep * bytesDepth;
        LcmsLock lock;
        dkCmsDoTransform(d->handle, data, data, pixelsThisStep);
        data               += size;
    }
}

void IccTransform::close()
{
    d->close();
}

/*
void IccTransform::closeProfiles()
{
    d->inputProfile.close();
    d->outputProfile.close();
    d->proofProfile.close();
    d->embeddedProfile.close();
}
*/

}  // namespace Digikam
