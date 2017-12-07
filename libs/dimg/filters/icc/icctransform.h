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

#ifndef ICCTRANSFORM_H
#define ICCTRANSFORM_H

// Qt includes

#include <QByteArray>
#include <QMetaType>
#include <QString>

// Local includes

#include "dimg.h"
#include "digikam_export.h"

class QImage;

namespace Digikam
{

class DImgLoaderObserver;
class TransformDescription;

class DIGIKAM_EXPORT IccTransform
{
public:

    enum RenderingIntent
    {
        Perceptual           = 0,
        RelativeColorimetric = 1,
        Saturation           = 2,
        AbsoluteColorimetric = 3
    };

public:

    IccTransform();
    ~IccTransform();

    IccTransform(const IccTransform& other);
    IccTransform& operator=(const IccTransform& other);

    /**
     * Apply this transform with the set profiles and options to the image.
     * Optionally pass an observer to get progress information.
     */
    bool apply(DImg& image, DImgLoaderObserver* const observer = 0);

    /// Apply this transform to the QImage. This has only basic functionality.
    bool apply(QImage& qimage);

    /// Closes the transform, not the profiles. Called at desctruction.
    void close();

    /**
     * Sets the input profiles of this transform.
     * You can call both setEmbeddedProfile and setInputProfile.
     * If the image contains an embedded profile this profile is used
     * and takes precedence over the set input profile, which is used
     * without an embedded profile. If none is set, sRGB is used.
     */
    void setEmbeddedProfile(const DImg& image);
    void setInputProfile(const IccProfile& profile);

    /// Sets the output transform
    void setOutputProfile(const IccProfile& profile);

    /// Makes this transform a proofing transform, if profile is not null
    void setProofProfile(const IccProfile& profile);

    /**
     * Call this with 'true' if you do not want the output profile
     * to be set as embedded profile after apply() did a transformation.
     * Default is to set the output profile as embedded profile (false).
     */
    void setDoNotEmbedOutputProfile(bool doNotEmbed);

    /// Set options
    void setIntent(RenderingIntent intent);
    void setIntent(int intent)
    {
        setIntent((RenderingIntent)intent);
    }

    void setProofIntent(RenderingIntent intent);
    void setProofIntent(int intent)
    {
        setProofIntent((RenderingIntent)intent);
    }

    void setUseBlackPointCompensation(bool useBPC);
    void setCheckGamut(bool checkGamut);
    void setCheckGamutMaskColor(const QColor& color);

    /// Returns the contained profiles
    IccProfile embeddedProfile()         const;
    IccProfile inputProfile()            const;
    IccProfile outputProfile()           const;
    IccProfile proofProfile()            const;

    RenderingIntent intent()             const;
    RenderingIntent proofIntent()        const;
    bool isUsingBlackPointCompensation() const;
    bool isCheckingGamut()               const;
    QColor checkGamutMaskColor()         const;

    /**
     * Returns if this transformation will have an effect, i.e. if
     * effective input profile and output profile are different.
     */
    bool willHaveEffect();

    /**
     *  Returns the embedded profile; if none is set, the input profile;
     *  if none is set, sRGB.
     */
    IccProfile effectiveInputProfile() const;

    /// Initialize LittleCMS library
    static void init();

private:

    bool checkProfiles();
    TransformDescription getDescription(const DImg& image);
    TransformDescription getProofingDescription(const DImg& image);
    TransformDescription getDescription(const QImage& image);
    bool open(TransformDescription& description);
    bool openProofing(TransformDescription& description);
    void transform(DImg& img, const TransformDescription&, DImgLoaderObserver* const observer = 0);
    void transform(QImage& img, const TransformDescription&);

public:

    class Private;

private:

    QSharedDataPointer<Private> d;
};

}  // namespace Digikam

Q_DECLARE_METATYPE(Digikam::IccTransform)

#endif   // ICCTRANSFORM_H
