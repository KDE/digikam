/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-12
 * Description : methods that implement color management tasks
 *
 * Copyright (C) 2005-2006 by F.J. Cruz <fj dot cruz at supercable dot es>
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef ICCMANAGER_H
#define ICCMANAGER_H

// Local includes

#include "digikam_export.h"
#include "iccprofile.h"
#include "iccsettings.h"
#include "iccsettingscontainer.h"

namespace Digikam
{

class DImg;
class DImgLoaderObserver;
class IccTransform;

class DIGIKAM_EXPORT IccManager
{
public:

    /**
     * Constructs an IccManager object.
     * The DImg will be edited. The filePath is for display only.
     */
    explicit IccManager(DImg& image, const ICCSettingsContainer& settings = IccSettings::instance()->settings());
    ~IccManager();

    void setObserver(DImgLoaderObserver* const observer);

    DImg image()                    const;
    ICCSettingsContainer settings() const;
    DImgLoaderObserver* observer()  const;

    bool hasValidWorkspace()   const;
    bool isUncalibratedColor() const;
    bool isMissingProfile()    const;
    bool isProfileMismatch()   const;

    /**
     * Transforms the image for full editing, using default settings.
     * If the default settings require showing a dialog, the image is marked as such
     * but no action is taken. See IccPostLoadingManager.
     */
    void transformDefault();

    /**
     * Same as above, but not using default settings but the given settings.
     */
    void transform(ICCSettingsContainer::Behavior behavior,
                   const IccProfile& specifiedProfile = IccProfile());
    /**
     * Transforms the image to sRGB
     */
    void transformToSRGB();

    /**
     * Returns true if a call to transformToSRGB() would have an effect.
     */
    static bool isSRGB(const DImg& img);

    /**
     * Transforms the image for output to the specified output profile
     */
    void transformForOutput(const IccProfile& outputProfile);

    /**
     * Transforms the image for display on screen. The result is not suitable
     * for editing or storage.
     * You can specify the widget in which the image will be displayed, or specify
     * the display profile yourself.
     * You can retrieve the profile with displayProfile() and pass it to transformForDisplay()
     * later (in a thread), or you can get a transform from displayTransform and apply it yourself.
     */
    void transformForDisplay();
    void transformForDisplay(QWidget* const widget);
    void transformForDisplay(const IccProfile& displayProfile);

    static IccProfile displayProfile(QWidget* const displayingWidget = 0);
    IccTransform displayTransform(QWidget* const displayingWidget = 0);
    IccTransform displayTransform(const IccProfile& displayProfile);

    /**
     * Returns a display transform, with soft-proofing enabled for the given device profile.
     */
    IccTransform displaySoftProofingTransform(const IccProfile& deviceProfile, QWidget* const displayingWidget = 0);
    IccTransform displaySoftProofingTransform(const IccProfile& deviceProfile, const IccProfile& displayProfile);

    /**
     * Returns true if the given image is marked as needing user interaction
     * for further color management decision after loading.
     * If this returns true, use IccPostLoadingManager to do this.
     */
    static bool needsPostLoadingManagement(const DImg& img);

    /** Returns the profile that will be used to interpret the image,
     *  using the given behavior
     */
    IccProfile imageProfile(ICCSettingsContainer::Behavior behavior,
                            const IccProfile& specifiedProfile = IccProfile());

    /** Transforms the given QImage from the given inputProfile to sRGB. */
    static void transformToSRGB(QImage& qimage, const IccProfile& inputProfile);

    /** Transforms the given QImage from sRGB to given display profile. */
    static void transformForDisplay(QImage& qimage, const IccProfile& displayProfile1 = displayProfile());

protected:

    void getTransform(IccTransform& trans, ICCSettingsContainer::Behavior behavior, const IccProfile& specifiedProfile);
    void setIccProfile(const IccProfile& profile);
    ICCSettingsContainer::Behavior safestBestBehavior() const;

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif   // ICCMANAGER_H
