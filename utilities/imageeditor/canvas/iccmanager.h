/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-12
 * Description : methods that implement color management tasks
 *
 * Copyright (C) 2005-2006 by F.J. Cruz <fj.cruz@supercable.es>
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes

// Local includes

#include "digikam_export.h"
#include "iccprofile.h"
#include "iccsettings.h"
#include "iccsettingscontainer.h"

namespace Digikam
{

class DImg;
class DImgLoaderObserver;
class IccManagerPriv;
class IccTransform;

class DIGIKAM_EXPORT IccManager
{
public:

    /**
     * Constructs an IccManager object.
     * The DImg will be edited. The filePath is for display only.
     */
    IccManager(DImg& image, const QString& filePath = QString(),
               const ICCSettingsContainer& settings = IccSettings::instance()->settings());
    ~IccManager();

    void setObserver(DImgLoaderObserver *observer);

    bool hasValidWorkspace() const;

    bool isUncalibratedColor() const;
    bool isMissingProfile() const;
    bool isProfileMismatch() const;

    /**
     * Transforms the image for full editing, using default settings.
     * If the default settings require showing a dialog, the image is marked as such
     * but no action is taken. See postLoadingManage.
     */
    void transformDefault();
    /**
     * Same as above, but not using default settings but the given settings.
     */
    void transform(ICCSettingsContainer::Behavior behavior,
                   IccProfile specifiedProfile = IccProfile());
    /**
     * Transforms the image for display on screen. The result is not suitable
     * for editing or storage.
     */
    void transformForDisplay();
    /**
     * Transforms the image to sRGB
     */
    void transformToSRGB();
    /**
     * Transforms the image for output to the specified output profile
     */
    void transformForOutput(const IccProfile& outputProfile);

    /**
     * Returns true if the given image is marked as needing user interaction
     * for further color management decision after loading.
     * If this returns true, use postLoadingManage() to do this.
     */
    static bool needsPostLoadingManagement(const DImg& img);
    /**
     * Carries out color management asking the user for his decision.
     * Afterwards, needsPostLoadingManagement will return false.
     */
    IccTransform postLoadingManage(QWidget *parent = 0);

    /** Returns the profile that will be used to interpret the image,
     *  using the given behavior
     */
    IccProfile imageProfile(ICCSettingsContainer::Behavior behavior,
                            IccProfile specifiedProfile = IccProfile());

protected:

    void getTransform(IccTransform& trans, ICCSettingsContainer::Behavior behavior, IccProfile specifiedProfile);
    void setIccProfile(const IccProfile& profile);

    IccManagerPriv * const d;
};

}  // namespace Digikam

#endif   // ICCMANAGER_H
