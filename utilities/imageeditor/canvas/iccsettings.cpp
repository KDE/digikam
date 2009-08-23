/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : central place for ICC settings
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

#include "iccsettings.h"
#include "iccsettings.moc"

// Qt includes

#include <QDir>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kglobal.h>
#include <ksharedconfig.h>

// Local includes

#include "iccprofile.h"
#include "icctransform.h"

namespace Digikam
{

class IccSettingsPriv
{
public:

    IccSettingsPriv()
    {
    }

    ICCSettingsContainer    settings;
    QMutex                  mutex;

    QList<IccProfile>       profiles;

    QList<IccProfile> scanDirectories(const QStringList& dirs);
    void scanDirectory(const QString& path, const QStringList& filter, QList<IccProfile> *profiles);
};

class IccSettingsCreator { public: IccSettings object; };
K_GLOBAL_STATIC(IccSettingsCreator, creator)

IccSettings *IccSettings::instance()
{
    return &creator->object;
}

IccSettings::IccSettings()
            : d(new IccSettingsPriv)
{
    IccTransform::init();
    readFromConfig();
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

IccProfile IccSettings::monitorProfile(QWidget *widget)
{
    //TODO: X.org Icc profile specification
    Q_UNUSED(widget)
    QMutexLocker lock(&d->mutex);
    if (!d->settings.monitorProfile.isNull())
        return d->settings.monitorProfile;
    else
        return IccProfile::sRGB();
}

bool IccSettings::isEnabled()
{
    return d->settings.enableCM;
}

void IccSettings::readFromConfig()
{
    ICCSettingsContainer s;
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(QString("Color Management"));
    s.readFromConfig(group);
    {
        QMutexLocker lock(&d->mutex);
        d->settings = s;
    }
    emit settingsChanged();
}

void IccSettings::setSettings(const ICCSettingsContainer& settings)
{
    {
        QMutexLocker lock(&d->mutex);
        if (settings.iccFolder != d->settings.iccFolder)
            d->profiles.clear();
        d->settings = settings;
    }
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(QString("Color Management"));
    settings.writeToConfig(group);
    emit settingsChanged();
}

void IccSettings::setUseManagedView(bool useManagedView)
{
    {
        QMutexLocker lock(&d->mutex);
        d->settings.useManagedView = useManagedView;
    }
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(QString("Color Management"));
    d->settings.writeManagedViewToConfig(group);
    emit settingsChanged();
}

void IccSettings::setIccPath(const QString& path)
{
    {
        QMutexLocker lock(&d->mutex);
        if (path == d->settings.iccFolder)
            return;
        d->profiles.clear();
        d->settings.iccFolder = path;
    }
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(QString("Color Management"));
    d->settings.writeManagedViewToConfig(group);
    emit settingsChanged();
}

QList<IccProfile> IccSettingsPriv::scanDirectories(const QStringList& dirs)
{
    QList<IccProfile> profiles;

    QStringList filters;
    filters << "*.icc" << "*.icm";
    kDebug() << dirs;
    foreach (const QString &dirPath, dirs)
    {
        QDir dir(dirPath);
        if (!dir.exists())
            continue;
        scanDirectory(dir.path(), filters, &profiles);
    }

    return profiles;
}

void IccSettingsPriv::scanDirectory(const QString& path, const QStringList& filter, QList<IccProfile> *profiles)
{
    QDir dir(path);
    QFileInfoList infos;
    infos << dir.entryInfoList(filter, QDir::Files | QDir::Readable);
    infos << dir.entryInfoList(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot);
    foreach (const QFileInfo &info, infos)
    {
        if (info.isFile())
        {
            //kDebug(50003) << info.filePath() << (info.exists() && info.isReadable());
            IccProfile profile(info.filePath());
            if (profile.open())
            {
                *profiles << profile;
                if (info.fileName() == "AdobeRGB1998.icc")
                    IccProfile::considerOriginalAdobeRGB(info.filePath());
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
            return d->profiles;
        extraPath = d->settings.iccFolder;
    }

    QList<IccProfile> profiles;
    // get system paths, e.g. /usr/share/color/icc
    QStringList paths = IccProfile::defaultSearchPaths();
    // add user-specified path
    if (!extraPath.isEmpty())
        paths << extraPath;
    // check search directories
    profiles << d->scanDirectories(paths);
    // load profiles that come with libkdcraw
    profiles << IccProfile::defaultProfiles();

    QMutexLocker lock(&d->mutex);
    d->profiles = profiles;

    return d->profiles;
}

QList<IccProfile> IccSettings::workspaceProfiles()
{
    QList<IccProfile> profiles;
    foreach (IccProfile profile, allProfiles())
    {
        switch (profile.type())
        {
            case IccProfile::Display:
            case IccProfile::ColorSpace:
                profiles << profile;
            default:
                break;
        }
    }
    return profiles;
}

QList<IccProfile> IccSettings::displayProfiles()
{
    QList<IccProfile> profiles;
    foreach (IccProfile profile, allProfiles())
    {
        if (profile.type() == IccProfile::Display)
            profiles << profile;
    }
    return profiles;
}

QList<IccProfile> IccSettings::inputProfiles()
{
    QList<IccProfile> profiles;
    foreach (IccProfile profile, allProfiles())
    {
        switch (profile.type())
        {
            case IccProfile::Input:
            case IccProfile::ColorSpace:
                profiles << profile;
            default:
                break;
        }
    }
    return profiles;
}

QList<IccProfile> IccSettings::outputProfiles()
{
    QList<IccProfile> profiles;
    foreach (IccProfile profile, allProfiles())
    {
        if (profile.type() == IccProfile::Output)
            profiles << profile;
    }
    return profiles;
}

void IccSettings::loadAllProfilesProperties()
{
    allProfiles();
    const int size = d->profiles.size();
    for (int i=0; i<size; ++i)
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

