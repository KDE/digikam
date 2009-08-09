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

#include <QMutex>
#include <QMutexLocker>

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <ksharedconfig.h>

// Local includes


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
    readFromConfig();
}

ICCSettingsContainer IccSettings::settings()
{
    QMutexLocker lock(&d->mutex);
    ICCSettingsContainer s(d->settings);
    return s;
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


}  // namespace Digikam

