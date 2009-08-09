/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-11-18
 * Description : a class to apply ICC color correction to image.
 *
 * Copyright (C) 2005-2006 by F.J. Cruz <fj.cruz@supercable.es>
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com> 
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

#include <QtCore/QByteArray>
#include <QtCore/QString>

// Local includes

#include "dimg.h"
#include "digikam_export.h"

namespace Digikam
{

class IccTransformPriv;
class TransformDescription;

class DIGIKAM_EXPORT IccTransform
{
public:

    IccTransform();
    ~IccTransform();

    IccTransform(const IccTransform& other);
    IccTransform &operator=(const IccTransform& other);

    /// Apply this transform with the set profiles and options to the image
    bool apply(DImg& image);

    /// Closes the transform, not the profiles. Called at desctruction.
    void close();

    enum RenderingIntent
    {
        Perceptual = 0,
        RelativeColorimetric = 1,
        Saturation = 2,
        AbsoluteColorimetric = 3
    };

    /**
     * Sets the input profiles of this transform.
     * You can call both setEmbeddedProfile and setInputProfile.
     * If the image contains an embedded profile this profile is used
     * and takes precedence over the set input profile, which is used
     * without an embedded profile. If none is set, sRGB is used.
     */
    void setEmbeddedProfile(const DImg& image);
    void setInputProfile(const IccProfile &profile);
    /// Sets the output transform
    void setOutputProfile(const IccProfile &profile);
    /// Makes this transform a proofing transform, if profile is not null
    void setProofProfile(const IccProfile &profile);

    /// Set options
    void setIntent(RenderingIntent intent);
    void setIntent(int intent) { setIntent((RenderingIntent)intent); }
    void setUseBlackPointCompensation(bool useBPC);
    void setCheckGamut(bool checkGamut);

    /// Read intent and BPC from config
    void readFromConfig();

    /// Returns the contained profiles
    IccProfile embeddedProfile() const;
    IccProfile inputProfile() const;
    IccProfile outputProfile() const;
    IccProfile proofProfile() const;

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

private:

    TransformDescription getDescription(const DImg& image);
    TransformDescription getProofingDescription(const DImg& image);
    bool open(TransformDescription &description);
    bool openProofing(TransformDescription &description);
    void transform(const DImg& img, const TransformDescription&);

private:

    QSharedDataPointer<IccTransformPriv> d;

};

}  // namespace Digikam

#endif   // ICCTRANSFORM_H
