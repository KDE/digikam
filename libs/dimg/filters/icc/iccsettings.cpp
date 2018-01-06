/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : central place for ICC settings
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

#include "iccsettings.h"
#include "digikam_config.h"

// Qt includes

#include <QApplication>
#include <QDesktopWidget>
#include <QDir>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>

// Local includes

#include "digikam_debug.h"
#include "iccprofile.h"
#include "icctransform.h"

// X11 includes

// Note must be after all other to prevent broken compilation
#ifdef HAVE_X11
#   include <climits>
#   include <X11/Xlib.h>
#   include <X11/Xatom.h>
#   include <QX11Info>
#endif /* HAVE_X11 */

namespace Digikam
{

class IccSettings::Private
{
public:

    Private()
        : configGroup(QLatin1String("Color Management"))
    {
    }

    QList<IccProfile>    scanDirectories(const QStringList& dirs);
    void                 scanDirectory(const QString& path, const QStringList& filter, QList<IccProfile>* const profiles);

    IccProfile           profileFromWindowSystem(QWidget* const widget);

    ICCSettingsContainer readFromConfig()               const;
    void                 writeToConfig()                const;
    void                 writeManagedViewToConfig()     const;
    void                 writeManagedPreviewsToConfig() const;

public:

    ICCSettingsContainer   settings;
    QMutex                 mutex;

    QList<IccProfile>      profiles;

    QHash<int, IccProfile> screenProfiles;

    const QString          configGroup;
};

// -----------------------------------------------------------------------------------------------

class IccSettingsCreator
{
public:

    IccSettings object;
};

Q_GLOBAL_STATIC(IccSettingsCreator, creator)

// -----------------------------------------------------------------------------------------------

IccSettings* IccSettings::instance()
{
    return &creator->object;
}

IccSettings::IccSettings()
    : d(new Private)
{
    IccTransform::init();
    readFromConfig();
    qRegisterMetaType<ICCSettingsContainer>("ICCSettingsContainer");
}

IccSettings::~IccSettings()
{
    delete d;
}

ICCSettingsContainer IccSettings::settings()
{
    QMutexLocker lock(&d->mutex);
    ICCSettingsContainer s(d->settings);
    return s;
}

IccProfile IccSettings::monitorProfile(QWidget* const widget)
{
    // system-wide profile set?
    IccProfile profile = d->profileFromWindowSystem(widget);

    if (!profile.isNull())
    {
        return profile;
    }

    QMutexLocker lock(&d->mutex);

    if (!d->settings.monitorProfile.isNull())
    {
        return d->settings.monitorProfile;
    }
    else
    {
        return IccProfile::sRGB();
    }
}

bool IccSettings::monitorProfileFromSystem() const
{
    // First, look into cache
    {
        QMutexLocker lock(&d->mutex);

        foreach(const IccProfile& profile, d->screenProfiles)
        {
            if (!profile.isNull())
            {
                return true;
            }
        }
    }

    // Second, check all toplevel widgets
    QList<QWidget*> topLevels = qApp->topLevelWidgets();

    foreach(QWidget* const widget, topLevels)
    {
        if (!d->profileFromWindowSystem(widget).isNull())
        {
            return true;
        }
    }

    return false;
}

/*
 * From koffice/libs/pigment/colorprofiles/KoLcmsColorProfileContainer.cpp
 * Copyright (C) 2000 Matthias Elter <elter@kde.org>
 *               2001 John Califf
 *               2004 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Adrian Page <adrian@pagenet.plus.com>
*/
IccProfile IccSettings::Private::profileFromWindowSystem(QWidget* const widget)
{
#ifdef HAVE_X11

    if (!QX11Info::isPlatformX11())
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Desktop platform is not X11";
        return IccProfile();
    }

    unsigned long appRootWindow;
    QString       atomName;

    QDesktopWidget* const desktop = QApplication::desktop();

    if (!desktop)
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "No desktop widget available for application";
        return IccProfile();
    }

    int screenNumber = desktop->screenNumber(widget);

    IccProfile profile;
    {
        QMutexLocker lock(&mutex);

        if (screenProfiles.contains(screenNumber))
        {
            return screenProfiles.value(screenNumber);
        }
    }

    if (desktop->isVirtualDesktop())
    {
        appRootWindow = QX11Info::appRootWindow(QX11Info::appScreen());
        atomName      = QString::fromLatin1("_ICC_PROFILE_%1").arg(screenNumber);
    }
    else
    {
        appRootWindow = QX11Info::appRootWindow(screenNumber);
        atomName      = QLatin1String("_ICC_PROFILE");
    }

    Atom          type;
    int           format;
    unsigned long nitems;
    unsigned long bytes_after;
    quint8*       str = 0;

    static Atom icc_atom = XInternAtom(QX11Info::display(), atomName.toLatin1().constData(), True);

    if ((icc_atom != None)                                                &&
        (XGetWindowProperty(QX11Info::display(), appRootWindow, icc_atom,
                           0, INT_MAX, False, XA_CARDINAL,
                           &type, &format, &nitems, &bytes_after,
                           (unsigned char**)& str) == Success)            &&
         nitems
       )
    {
        QByteArray bytes = QByteArray::fromRawData((char*)str, (quint32)nitems);

        if (!bytes.isEmpty())
        {
            profile = bytes;
        }

        qCDebug(DIGIKAM_DIMG_LOG) << "Found X.org XICC monitor profile " << profile.description();
    }
    else
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "No X.org XICC profile installed for screen " << screenNumber;
    }

    // insert to cache even if null
    {
        QMutexLocker lock(&mutex);
        screenProfiles.insert(screenNumber, profile);
    }

#elif defined Q_OS_WIN
    //TODO
    Q_UNUSED(widget);
#elif defined Q_OS_OSX
    //TODO
    Q_UNUSED(widget);
#else
    // Unsupported platform
    Q_UNUSED(widget);
#endif

    return IccProfile();
}

bool IccSettings::isEnabled() const
{
    return d->settings.enableCM;
}

bool IccSettings::useManagedPreviews() const
{
    return (isEnabled() && d->settings.useManagedPreviews);
}

ICCSettingsContainer IccSettings::Private::readFromConfig() const
{
    ICCSettingsContainer s;
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroup);
    s.readFromConfig(group);
    return s;
}

void IccSettings::Private::writeToConfig() const
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroup);
    settings.writeToConfig(group);
}

void IccSettings::Private::writeManagedViewToConfig() const
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroup);
    settings.writeManagedViewToConfig(group);
}

void IccSettings::Private::writeManagedPreviewsToConfig() const
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroup);
    settings.writeManagedPreviewsToConfig(group);
}

void IccSettings::readFromConfig()
{
    ICCSettingsContainer old, s;
    s = d->readFromConfig();
    {
        QMutexLocker lock(&d->mutex);
        old         = d->settings;
        d->settings = s;
    }

    emit settingsChanged();
    emit settingsChanged(s, old);
}

void IccSettings::setSettings(const ICCSettingsContainer& settings)
{
    ICCSettingsContainer old;
    {
        QMutexLocker lock(&d->mutex);

        if (settings.iccFolder != d->settings.iccFolder)
        {
            d->profiles.clear();
        }

        old         = d->settings;
        d->settings = settings;
    }

    d->writeToConfig();
    emit settingsChanged();
    emit settingsChanged(settings, old);
}

void IccSettings::setUseManagedView(bool useManagedView)
{
    ICCSettingsContainer old, current;
    {
        QMutexLocker lock(&d->mutex);
        old                        = d->settings;
        d->settings.useManagedView = useManagedView;
        current                    = d->settings;
    }

    d->writeManagedViewToConfig();

    emit settingsChanged();
    emit settingsChanged(current, old);
}

void IccSettings::setUseManagedPreviews(bool useManagedPreviews)
{
    ICCSettingsContainer old, current;
    {
        QMutexLocker lock(&d->mutex);
        old                            = d->settings;
        d->settings.useManagedPreviews = useManagedPreviews;
        current                        = d->settings;
    }

    d->writeManagedPreviewsToConfig();

    emit settingsChanged();
    emit settingsChanged(current, old);
}

void IccSettings::setIccPath(const QString& path)
{
    ICCSettingsContainer old, current;
    {
        QMutexLocker lock(&d->mutex);

        if (path == d->settings.iccFolder)
        {
            return;
        }

        d->profiles.clear();
        old                   = d->settings;
        d->settings.iccFolder = path;
        current               = d->settings;
    }

    d->writeManagedViewToConfig();

    emit settingsChanged();
    emit settingsChanged(current, old);
}

QList<IccProfile> IccSettings::Private::scanDirectories(const QStringList& dirs)
{
    QList<IccProfile> profiles;
    QStringList       filters;
    filters << QLatin1String("*.icc") << QLatin1String("*.icm");
    qCDebug(DIGIKAM_DIMG_LOG) << dirs;

    foreach(const QString& dirPath, dirs)
    {
        QDir dir(dirPath);

        if (!dir.exists())
        {
            continue;
        }

        scanDirectory(dir.path(), filters, &profiles);
    }

    return profiles;
}

void IccSettings::Private::scanDirectory(const QString& path, const QStringList& filter, QList<IccProfile>* const profiles)
{
    QDir          dir(path);
    QFileInfoList infos;
    infos << dir.entryInfoList(filter, QDir::Files | QDir::Readable);
    infos << dir.entryInfoList(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot);

    foreach(const QFileInfo& info, infos)
    {
        if (info.isFile())
        {
            //qCDebug(DIGIKAM_DIMG_LOG) << info.filePath() << (info.exists() && info.isReadable());
            IccProfile profile(info.filePath());

            if (profile.open())
            {
                *profiles << profile;

                if (info.fileName() == QLatin1String("AdobeRGB1998.icc"))
                {
                    IccProfile::considerOriginalAdobeRGB(info.filePath());
                }
            }
        }
        else if (info.isDir() && !info.isSymLink())
        {
            scanDirectory(info.filePath(), filter, profiles);
        }
    }
}

QList<IccProfile> IccSettings::allProfiles()
{
    QString extraPath;
    {
        QMutexLocker lock(&d->mutex);

        if (!d->profiles.isEmpty())
        {
            return d->profiles;
        }

        extraPath = d->settings.iccFolder;
    }

    QList<IccProfile> profiles;

    // get system paths, e.g. /usr/share/color/icc
    QStringList paths = IccProfile::defaultSearchPaths();

    // add user-specified path
    if (!extraPath.isEmpty() && !paths.contains(extraPath))
    {
        paths << extraPath;
    }

    // check search directories
    profiles << d->scanDirectories(paths);

    // load profiles that come with RawEngine
    profiles << IccProfile::defaultProfiles();

    QMutexLocker lock(&d->mutex);
    d->profiles = profiles;

    return d->profiles;
}

QList<IccProfile> IccSettings::workspaceProfiles()
{
    QList<IccProfile> profiles;

    foreach(IccProfile profile, allProfiles())  // krazy:exclude=foreach
    {
        switch (profile.type())
        {
            case IccProfile::Display:
            case IccProfile::ColorSpace:
                profiles << profile;
                break;

            default:
                break;
        }
    }

    return profiles;
}

QList<IccProfile> IccSettings::displayProfiles()
{
    QList<IccProfile> profiles;

    foreach(IccProfile profile, allProfiles())  // krazy:exclude=foreach
    {
        if (profile.type() == IccProfile::Display)
        {
            profiles << profile;
        }
    }

    return profiles;
}

QList<IccProfile> IccSettings::inputProfiles()
{
    QList<IccProfile> profiles;

    foreach(IccProfile profile, allProfiles())  // krazy:exclude=foreach
    {
        switch (profile.type())
        {
            case IccProfile::Input:
            case IccProfile::ColorSpace:
                profiles << profile;
                break;

            default:
                break;
        }
    }

    return profiles;
}

QList<IccProfile> IccSettings::outputProfiles()
{
    QList<IccProfile> profiles;

    foreach(IccProfile profile, allProfiles())  // krazy:exclude=foreach
    {
        if (profile.type() == IccProfile::Output)
        {
            profiles << profile;
        }
    }

    return profiles;
}

QList<IccProfile> IccSettings::profilesForDescription(const QString& description)
{
    QList<IccProfile> profiles;

    if (description.isEmpty())
    {
        return profiles;
    }

    foreach(IccProfile profile, allProfiles())  // krazy:exclude=foreach
    {
        if (profile.description() == description)
        {
            profiles << profile;
        }
    }

    return profiles;
}

void IccSettings::loadAllProfilesProperties()
{
    allProfiles();
    const int size = d->profiles.size();

    for (int i = 0; i < size; ++i)
    {
        IccProfile& profile = d->profiles[i];

        if (!profile.isOpen())
        {
            profile.description();
            profile.type();
            profile.close();
        }
        else
        {
            profile.description();
            profile.type();
        }
    }
}

}  // namespace Digikam
