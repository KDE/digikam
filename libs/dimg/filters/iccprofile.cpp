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
#include <QMutex>

// KDE includes

#include <kdebug.h>
#include <kglobal.h>
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
        type        = IccProfile::InvalidType;
    }

    IccProfilePriv(const IccProfilePriv& other)
                : QSharedData(other)
    {
        handle      = 0;
        operator=(other);
    }

    IccProfilePriv &operator=(const IccProfilePriv& other)
    {
        data        = other.data;
        filePath    = other.filePath;
        description = other.description;
        type        = other.type;
        close();
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
            LcmsLock lock();
            cmsCloseProfile(handle);
            handle = 0;
        }
    }

    QByteArray  data;
    QString     filePath;
    QString     description;
    IccProfile::ProfileType
                type;

    cmsHPROFILE handle;
};

K_GLOBAL_STATIC_WITH_ARGS(QMutex, lcmsMutex, (QMutex::Recursive))

LcmsLock::LcmsLock()
{
    lcmsMutex->lock();
}

LcmsLock::~LcmsLock()
{
    lcmsMutex->unlock();
}

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

IccProfile IccProfile::wideGamutRGB()
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
             << wideGamutRGB();
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
        LcmsLock lock();
        d->handle = cmsOpenProfileFromMem(d->data.data(), (DWORD)d->data.size());
    }
    else if (!d->filePath.isNull())
    {
        // read file
        data();

        if (d->data.isEmpty())
            return false;
        LcmsLock lock();
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

    if (!d->description.isNull())
        return d->description;

    if (!open())
        return QString();

    LcmsLock lock();
    const char *desc = cmsTakeProductDesc(d->handle);
    if (desc && desc[0] != '\0')
        d->description = QString::fromLatin1(desc);
    return d->description;
}

IccProfile::ProfileType IccProfile::type()
{
    if (!d)
        return InvalidType;

    if (d->type != InvalidType)
        return d->type;

    if (!open())
        return InvalidType;

    LcmsLock lock();
    switch ((int)cmsGetDeviceClass(d->handle))
    {
        case icSigInputClass:
        case 0x6e6b7066: // 'nkbf', proprietary in Nikon profiles
            d->type = Input;
            break;
        case icSigDisplayClass:
            d->type = Display;
            break;
        case icSigOutputClass:
            d->type = Output;
            break;
        case icSigColorSpaceClass:
            d->type = ColorSpace;
            break;
        case icSigLinkClass:
            d->type = DeviceLink;
            break;
        case icSigAbstractClass:
            d->type = Abstract;
            break;
        case icSigNamedColorClass:
            d->type = NamedColor;
            break;
        default:
            break;
    }
    return d->type;
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
