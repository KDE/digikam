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

#include <QDir>
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

IccProfile::IccProfile(const char *location, const QString& relativePath)
            : d(0)
{
    QString filePath = KStandardDirs::locate(location, relativePath);
    if (filePath.isNull())
    {
        kError(50003) << "The sRGB profile" << relativePath << "cannot be found. Check your installation.";
        return;
    }
    d = new IccProfilePriv;
    d->filePath = filePath;
}


IccProfile IccProfile::sRGB()
{
    // The srgb.icm file seems to have a whitepoint of D50, see #133913
    return IccProfile("data", "libkdcraw/profiles/srgb-d65.icm");
}

IccProfile IccProfile::adobeRGB()
{
    return IccProfile("data", "libkdcraw/profiles/adobergb.icm");
}

IccProfile IccProfile::wideGamuteRGB()
{
    return IccProfile("data", "libkdcraw/profiles/widegamut.icm");
}

IccProfile IccProfile::proPhotoRGB()
{
    return IccProfile("data", "libkdcraw/profiles/prophoto.icm");
}

IccProfile IccProfile::appleRGB()
{
    return IccProfile("data", "libkdcraw/profiles/applergb.icm");
}

QList<IccProfile> IccProfile::defaultProfiles()
{
    QList<IccProfile> profiles;
    profiles << sRGB()
             << adobeRGB()
             << appleRGB()
             << proPhotoRGB()
             << wideGamuteRGB();
    return profiles;
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

QString IccProfile::filePath() const
{
    if (!d)
        return QString();

    return d->filePath;
}

QString IccProfile::description()
{
    if (!d)
        return QString();

    open();
    const char *desc = cmsTakeProductDesc(d->handle);
    if (desc && desc[0] != '\0')
        return QString::fromLatin1(desc);
    return QString();
}

IccProfile::ProfileType IccProfile::type()
{
    if (!d)
        return InvalidType;

    if (!open())
        return InvalidType;

    switch ((int)cmsGetDeviceClass(d->handle))
    {
        case icSigInputClass:
        case 0x6e6b7066: // 'nkbf', proprietary in Nikon profiles
            return Input;
        case icSigDisplayClass:
            return Display;
        case icSigOutputClass:
            return Output;
        case icSigColorSpaceClass:
            return ColorSpace;
        case icSigLinkClass:
            return DeviceLink;
        case icSigAbstractClass:
            return Abstract;
        case icSigNamedColorClass:
            return NamedColor;
        default:
            return InvalidType;
    }
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

QStringList IccProfile::defaultSearchPaths()
{
    QStringList paths;
    QStringList candidates;

    paths << KGlobal::dirs()->findDirs("data", "color/icc");

    #ifdef Q_WS_WIN
    //TODO
    #elif defined (Q_WS_MAC)
    //TODO
    #else

        // XDG data dirs, including /usr/share/color/icc
        QStringList dataDirs = QString::fromLocal8Bit(getenv("XDG_DATA_DIRS")).split(":", QString::SkipEmptyParts);
        if (!dataDirs.contains(QLatin1String("/usr/share")))
            dataDirs << "/usr/share";
        if (!dataDirs.contains(QLatin1String("/usr/local/share")))
            dataDirs << "/usr/local/share";
        foreach (const QString &dataDir, dataDirs)
            candidates << dataDir + "/color/icc";

        // XDG_DATA_HOME
        QString dataHomeDir = QString::fromLocal8Bit(getenv("XDG_DATA_HOME"));
        candidates << dataHomeDir + "/color/icc";
        candidates << dataHomeDir + "/icc";

        // home dir directories
        candidates << QDir::homePath() + "/.local/share/color/icc/";
        candidates << QDir::homePath() + "/.local/share/icc/";
        candidates << QDir::homePath() + "/.color/icc/";
    #endif

    foreach (const QString &candidate, candidates)
    {
        QDir dir(candidate);
        if (dir.exists() && dir.isReadable() && !paths.contains(candidate))
        {
            paths << candidate;
        }
    }
    //kDebug(50003) << candidates << '\n' << paths;

    return paths;
}



}  // namespace Digikam
