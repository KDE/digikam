/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-07
 * Description : a wrapper class for an ICC color profile
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

#ifndef ICCPROFILE_H
#define ICCPROFILE_H

// Qt includes

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QSharedData>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DImg;
class IccProfilePriv;

class DIGIKAM_EXPORT IccProfile
{
public:

    /**
     * Creates a null profile
     */
    IccProfile();

    /**
     * Creates a profile from the given data in memory
     */
    IccProfile(const QByteArray& data);

    /**
     * Creates a profile from the given file
     */
    IccProfile(const QString& filePath);

    /// Returns the sRGB profile (available with libkdcraw). You still need to call open().
    static IccProfile sRGB();
    /// Returns the Adobe RGB profile (available with libkdcraw). You still need to call open().
    static IccProfile adobeRGB();

    IccProfile(const IccProfile& other);
    ~IccProfile();

    IccProfile &operator=(const IccProfile& other);

    bool isNull() const;

    /**
     *  Returns true if both profiles are null, if both profiles are created from the
     *  same file profile, or if the loaded profile data is identical.
     *  Note: This will not ensure that the data is loaded. Use isSameProfile().
     *  
     */
    bool operator==(const IccProfile& other) const;
    /**
     * This method compares the actual profile data bit by bit.
     */
    bool isSameProfileAs(IccProfile& other);

    /**
     * Open this profile. Returns true if the operation succeeded
     * or the profile is already open. Returns false if the profile is null
     * or the operation failed.
     * You need to open each profile after construction.
     */
    bool open();
    /**
     * Close the profile, freeing resources. You can re-open.
     * Called automatically at destruction.
     */
    void close();
    /**
     * Returns if the profile is opened.
     */
    bool isOpen() const;

    /**
     * Reads the profile description. Opens the profile if necessary.
     */
    QString description();

    /**
     * Returns the raw profile data.
     */
    QByteArray data();

    /**
     * Writes the profile to the given file.
     */
    bool writeToFile(const QString& filePath);

    /// Access to the LCMS cmsHPROFILE handle
    void *handle() const;
    operator void*() const { return handle(); }

private:

    QSharedDataPointer<IccProfilePriv> d;
};


}  // namespace Digikam

#endif   // ICCPROFILE_H
