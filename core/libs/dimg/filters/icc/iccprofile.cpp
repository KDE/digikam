/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-07
 * Description : a wrapper class for an ICC color profile
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

#include "iccprofile.h"
#include "digikam-lcms.h"

// Qt includes

#include <QDir>
#include <QFile>
#include <QMutex>
#include <QCryptographicHash>
#include <QStandardPaths>

// Local includes

#include "digikam_debug.h"
#include "dimg.h"

namespace Digikam
{

class IccProfile::Private : public QSharedData
{
public:

    Private() :
        type(IccProfile::InvalidType),
        handle(0)
    {
    }

    Private(const Private& other)
        : QSharedData(other)
    {
        handle   = 0;
        operator = (other);
    }

    Private& operator=(const Private& other)
    {
        data        = other.data;
        filePath    = other.filePath;
        description = other.description;
        type        = other.type;
        close();
        handle      = 0;
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
            LcmsLock lock;
            dkCmsCloseProfile(handle);
            handle = 0;
        }
    }

public:

    QByteArray              data;
    QString                 filePath;
    QString                 description;

    IccProfile::ProfileType type;

    cmsHPROFILE             handle;
};

// ----------------------------------------------------------------------------------

class IccProfileStatic
{
public:

    IccProfileStatic()
        : lcmsMutex()
    {
    }

    QMutex  lcmsMutex;
    QString adobeRGBPath;
};

Q_GLOBAL_STATIC(IccProfileStatic, static_d)

// ----------------------------------------------------------------------------------

LcmsLock::LcmsLock()
{
    static_d->lcmsMutex.lock();
}

LcmsLock::~LcmsLock()
{
    static_d->lcmsMutex.unlock();
}

// ----------------------------------------------------------------------------------

IccProfile::IccProfile()
    : d(0)
{
}

IccProfile::IccProfile(const QByteArray& data)
    : d(new Private)
{
    d->data = data;
}

IccProfile::IccProfile(const QString& filePath)
    : d(new Private)
{
    d->filePath = filePath;
}

IccProfile::IccProfile(const char* const location, const QString& relativePath)
    : d(0)
{
    QString filePath;

    // NOTE: if necessary, implement new location support here.

    if (QLatin1String(location) == QLatin1String("data"))
    {
         //qCDebug(DIGIKAM_DIMG_LOG) << "Searching ICC profile from data directory with relative path:" << relativePath;
         filePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, relativePath);
    }
    else
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Data location " << location << " to handle bundled profile is not supported.";
    }

    if (filePath.isNull())
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "The bundled profile" << relativePath << "cannot be found. Check your installation.";
        return;
    }

    d           = new Private;
    d->filePath = filePath;
}

IccProfile IccProfile::sRGB()
{
    // The srgb.icm file seems to have a whitepoint of D50, see #133913
    return IccProfile("data", QLatin1String("digikam/profiles/srgb-d65.icm"));
}

IccProfile IccProfile::adobeRGB()
{
    QString path = static_d->adobeRGBPath;

    if (path.isEmpty())
    {
        path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/profiles/compatibleWithAdobeRGB1998.icc"));
    }

    return IccProfile(path);
}

IccProfile IccProfile::wideGamutRGB()
{
    return IccProfile("data", QLatin1String("digikam/profiles/widegamut.icm"));
}

IccProfile IccProfile::proPhotoRGB()
{
    return IccProfile("data", QLatin1String("digikam/profiles/prophoto.icm"));
}

QList<IccProfile> IccProfile::defaultProfiles()
{
    QList<IccProfile> profiles;

    profiles << sRGB()
             << adobeRGB()
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

IccProfile& IccProfile::operator=(const IccProfile& other)
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
    {
        return true;
    }

    if (d && other.d)
    {
        if (!d->filePath.isNull() || !other.d->filePath.isNull())
        {
            return (d->filePath == other.d->filePath);
        }

        if (!d->data.isNull() || other.d->data.isNull())
        {
            return (d->data == other.d->data);
        }
    }

    return false;
}

bool IccProfile::isSameProfileAs(IccProfile& other)
{
    if (d == other.d)
    {
        return true;
    }

    if (d && other.d)
    {
        // uses memcmp
        return (data() == other.data());
    }

    return false;
}

QByteArray IccProfile::data()
{
    if (!d)
    {
        return QByteArray();
    }

    if (!d->data.isEmpty())
    {
        return d->data;
    }
    else if (!d->filePath.isNull())
    {
        QFile file(d->filePath);

        if (!file.open(QIODevice::ReadOnly))
        {
            return QByteArray();
        }

        d->data = file.readAll();
        file.close();
        return d->data;
    }

    return QByteArray();
}

bool IccProfile::open()
{
    if (!d)
    {
        return false;
    }

    if (d->handle)
    {
        return true;
    }

    if (!d->data.isEmpty())
    {
        LcmsLock lock;
        d->handle = dkCmsOpenProfileFromMem(d->data.data(), (DWORD)d->data.size());
    }
    else if (!d->filePath.isNull())
    {
        // read file
        data();

        if (d->data.isEmpty())
        {
            return false;
        }

        LcmsLock lock;
        d->handle = dkCmsOpenProfileFromMem(d->data.data(), (DWORD)d->data.size());
    }

    return d->handle;
}

void IccProfile::close()
{
    if (!d)
    {
        return;
    }

    d->close();
}

bool IccProfile::isOpen() const
{
    if (!d)
    {
        return false;
    }

    return d->handle;
}

QString IccProfile::filePath() const
{
    if (!d)
    {
        return QString();
    }

    return d->filePath;
}

QString IccProfile::description()
{
    if (!d)
    {
        return QString();
    }

    if (!d->description.isNull())
    {
        return d->description;
    }

    if (!open())
    {
        return QString();
    }

    LcmsLock lock;

    if ( !QString(dkCmsTakeProductDesc(d->handle)).isEmpty() )
    {
        d->description = QString(dkCmsTakeProductDesc(d->handle)).replace(QLatin1Char('\n'), QLatin1Char(' '));
    }

    return d->description;
}

IccProfile::ProfileType IccProfile::type()
{
    if (!d)
    {
        return InvalidType;
    }

    if (d->type != InvalidType)
    {
        return d->type;
    }

    if (!open())
    {
        return InvalidType;
    }

    LcmsLock lock;

    switch ((int)dkCmsGetDeviceClass(d->handle))
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
    {
        return false;
    }

    QByteArray profile = data();

    if (!profile.isEmpty())
    {
        QFile file(filePath);

        if (!file.open(QIODevice::WriteOnly))
        {
            return false;
        }

        if (file.write(profile) == -1)
        {
            return false;
        }

        file.close();
        return true;
    }

    return false;
}

void* IccProfile::handle() const
{
    if (!d)
    {
        return 0;
    }

    return d->handle;
}

QStringList IccProfile::defaultSearchPaths()
{
    QStringList paths;
    QStringList candidates;

    paths << QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QLatin1String("color/icc"), QStandardPaths::LocateDirectory);

#ifdef Q_OS_WIN

    candidates << QDir::rootPath() + QLatin1String("/Windows/Spool/Drivers/Color/");   // For Win2K and WinXP
    candidates << QDir::rootPath() + QLatin1String("/Windows/Color/");                 // For Win98 and WinMe

#elif defined (Q_OS_OSX)

    // Use a scheme highly identical to the Linux scheme, adapted for MacPorts in /opt/local, ofcial PKG installer, and the OS X standard ColorSync directories

    candidates << QLatin1String("/System/Library/ColorSync/Profiles");
    candidates << QLatin1String("/Library/ColorSync/Profiles");
    candidates << QDir::homePath() + QLatin1String("/Library/ColorSync/Profiles");

    // MacPorts installs for KDE, so we include the XDG data dirs, including /usr/share/color/icc
    QStringList dataDirs = QString::fromLocal8Bit(getenv("XDG_DATA_DIRS")).split(QLatin1Char(':'), QString::SkipEmptyParts);

    if (!dataDirs.contains(QLatin1String("/opt/local/share")))
    {
        dataDirs << QLatin1String("/opt/local/share");
    }

    if (!dataDirs.contains(QLatin1String("/opt/digikam/share")))
    {
        dataDirs << QLatin1String("/opt/digikam/share");
    }

    foreach(const QString& dataDir, dataDirs)
    {
        candidates << dataDir + QLatin1String("/color/icc");
    }

    // XDG_DATA_HOME
    QString dataHomeDir = QString::fromLocal8Bit(getenv("XDG_DATA_HOME"));

    if (!dataHomeDir.isEmpty())
    {
        candidates << dataHomeDir + QLatin1String("/color/icc");
        candidates << dataHomeDir + QLatin1String("/icc");
    }

    // home dir directories
    candidates << QDir::homePath() + QLatin1String("/.local/share/color/icc/");
    candidates << QDir::homePath() + QLatin1String("/.local/share/icc/");
    candidates << QDir::homePath() + QLatin1String("/.color/icc/");

#else // LINUX

    // XDG data dirs, including /usr/share/color/icc
    QStringList dataDirs = QString::fromLocal8Bit(getenv("XDG_DATA_DIRS")).split(QLatin1Char(':'), QString::SkipEmptyParts);

    if (!dataDirs.contains(QLatin1String("/usr/share")))
    {
        dataDirs << QLatin1String("/usr/share");
    }

    if (!dataDirs.contains(QLatin1String("/usr/local/share")))
    {
        dataDirs << QLatin1String("/usr/local/share");
    }

    foreach(const QString& dataDir, dataDirs)
    {
        candidates << dataDir + QLatin1String("/color/icc");
    }

    // XDG_DATA_HOME
    QString dataHomeDir = QString::fromLocal8Bit(getenv("XDG_DATA_HOME"));

    if (!dataHomeDir.isEmpty())
    {
        candidates << dataHomeDir + QLatin1String("/color/icc");
        candidates << dataHomeDir + QLatin1String("/icc");
    }

    // home dir directories
    candidates << QDir::homePath() + QLatin1String("/.local/share/color/icc/");
    candidates << QDir::homePath() + QLatin1String("/.local/share/icc/");
    candidates << QDir::homePath() + QLatin1String("/.color/icc/");

#endif

    foreach(const QString& candidate, candidates)
    {
        QDir dir(candidate);

        if (dir.exists() && dir.isReadable())
        {
            QString path = dir.canonicalPath();

            if (!paths.contains(path))
            {
                paths << path;
            }
        }
    }
    //qCDebug(DIGIKAM_DIMG_LOG) << candidates << '\n' << paths;

    return paths;
}

void IccProfile::considerOriginalAdobeRGB(const QString& filePath)
{
    if (!static_d->adobeRGBPath.isNull())
    {
        return;
    }

    QFile file(filePath);

    if (file.open(QIODevice::ReadOnly))
    {
        QCryptographicHash md5(QCryptographicHash::Md5);
        md5.addData(&file);

        if (md5.result().toHex() == QByteArray("dea88382d899d5f6e573b432473ae138"))
        {
            qCDebug(DIGIKAM_DIMG_LOG) << "The original Adobe RGB (1998) profile has been found at" << filePath;
            static_d->adobeRGBPath = filePath;
        }
    }
}

}  // namespace Digikam
