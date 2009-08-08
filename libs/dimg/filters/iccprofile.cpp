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

#include "iccprofile.h"

// LCMS

#include <lcms.h>

// Qt includes

#include <QFile>

// KDE includes

#include <kdebug.h>
#include <kstandarddirs.h>

// Local includes

#include "dimg.h"

namespace Digikam
{

class IccProfilePriv : public QSharedData
{
public:

    IccProfilePriv()
    {
        handle      = 0;
    }

    IccProfilePriv(const IccProfilePriv& other)
                : QSharedData(other)
    {
        operator=(other);
    }

    IccProfilePriv &operator=(const IccProfilePriv& other)
    {
        data        = other.data;
        filePath    = other.filePath;
        handle      = 0;
        return *this;
    }

    ~IccProfilePriv()
    {
        close();
    }

    void close()
    {
        if (handle)
        {
            cmsCloseProfile(handle);
            handle = 0;
        }
    }

    QByteArray  data;
    QString     filePath;

    cmsHPROFILE handle;
};

IccProfile::IccProfile()
            : d(0)
{
}

IccProfile::IccProfile(const QByteArray& data)
            : d(new IccProfilePriv)
{
    d->data = data;
}

IccProfile::IccProfile(const QString& filePath)
            : d(new IccProfilePriv)
{
    d->filePath = filePath;
}

IccProfile IccProfile::sRGB()
{
    // The srgb.icm file seems to have a whitepoint of D50, see #133913
    QString filePath = KStandardDirs::locate("data", "libkdcraw/profiles/srgb-d65.icm");
    if (filePath.isNull())
    {
        kError(50003) << "The sRGB profile libkdcraw/profiles/srgb-d65.icm cannot be found. Check your installation.";
        return IccProfile();
    }
    return IccProfile(filePath);
}

IccProfile IccProfile::adobeRGB()
{
    QString filePath = KStandardDirs::locate("data", "libkdcraw/profiles/adobergb.icm");
    if (filePath.isNull())
    {
        kError(50003) << "The sRGB profile libkdcraw/profiles/adobergb.icm cannot be found. Check your installation.";
        return IccProfile();
    }
    return IccProfile(filePath);
}

IccProfile::IccProfile(const IccProfile& other)
            : d(other.d)
{
}

IccProfile::~IccProfile()
{
}

IccProfile &IccProfile::operator=(const IccProfile& other)
{
    d = other.d;
    return *this;
}

bool IccProfile::isNull() const
{
    return !d;
}

bool IccProfile::operator==(const IccProfile& other) const
{
    if (d == other.d)
        return true;
    if (d && other.d)
    {
        if (!d->filePath.isNull() || !other.d->filePath.isNull())
            return d->filePath == other.d->filePath;
        if (!d->data.isNull() || other.d->data.isNull())
            return d->data == other.d->data;
    }
    return false;
}

bool IccProfile::isSameProfileAs(IccProfile& other)
{
    if (d == other.d)
        return true;
    if (d && other.d)
    {
        // uses memcmp
        return data() == other.data();
    }
    return false;
}

QByteArray IccProfile::data()
{
    if (!d)
        return QByteArray();

    if (!d->data.isEmpty())
    {
        return d->data;
    }
    else if (!d->filePath.isNull())
    {
        QFile file(d->filePath);
        if ( !file.open(QIODevice::ReadOnly) )
            return false;
        d->data = file.readAll();
        file.close();
        return d->data;
    }
    return QByteArray();
}

bool IccProfile::open()
{
    if (!d)
        return false;

    if (d->handle)
        return true;

    if (!d->data.isEmpty())
    {
        d->handle = cmsOpenProfileFromMem(d->data.data(), (DWORD)d->data.size());
    }
    else if (!d->filePath.isNull())
    {
        // read file
        data();

        if (d->data.isEmpty())
            return false;
        d->handle = cmsOpenProfileFromMem(d->data.data(), (DWORD)d->data.size());
    }

    return d->handle;
}

void IccProfile::close()
{
    if (!d)
        return;

    d->close();
}

bool IccProfile::isOpen() const
{
    if (!d)
        return false;

    return d->handle;
}

QString IccProfile::description()
{
    if (!d)
        return QString();

    open();
    const char *desc = cmsTakeProductDesc(d->handle);
    if (desc)
        return QString::fromLatin1(desc);
    return QString();
}

bool IccProfile::writeToFile(const QString& filePath)
{
    if (!d)
        return false;

    QByteArray profile = data();
    if (!profile.isEmpty())
    {
        QFile file(filePath);
        if ( !file.open(QIODevice::WriteOnly) )
            return false;

        if (file.write(profile) == -1)
            return false;
        file.close();
        return true;
    }
    return false;
}

void *IccProfile::handle() const
{
    if (!d)
        return 0;

    return d->handle;
}


}  // namespace Digikam
