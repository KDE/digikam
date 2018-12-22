/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-20
 * Description : central place for MetaEngine settings
 *
 * Copyright (C) 2010-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "metaenginesettings.h"

// Qt includes

#include <QDir>
#include <QFileInfo>
#include <QMutex>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN MetaEngineSettings::Private
{
public:

    explicit Private()
        : mutex(),
          configGroup(QLatin1String("Metadata Settings"))
    {
    }

    MetaEngineSettingsContainer settings;
    QMutex                      mutex;

    const QString               configGroup;

public:

    MetaEngineSettingsContainer readFromConfig() const;
    void                        writeToConfig() const;
    MetaEngineSettingsContainer setSettings(const MetaEngineSettingsContainer& s);
};

MetaEngineSettingsContainer MetaEngineSettings::Private::readFromConfig() const
{
    MetaEngineSettingsContainer s;
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroup);
    s.readFromConfig(group);
    return s;
}

void MetaEngineSettings::Private::writeToConfig() const
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroup);
    settings.writeToConfig(group);
}

MetaEngineSettingsContainer MetaEngineSettings::Private::setSettings(const MetaEngineSettingsContainer& s)
{
    QMutexLocker lock(&mutex);
    MetaEngineSettingsContainer old;
    old      = settings;
    settings = s;
    return old;
}

// -----------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN MetaEngineSettingsCreator
{
public:

    MetaEngineSettings object;
};

Q_GLOBAL_STATIC(MetaEngineSettingsCreator, metaEngineSettingsCreator)

// -----------------------------------------------------------------------------------------------

MetaEngineSettings* MetaEngineSettings::instance()
{
    return &metaEngineSettingsCreator->object;
}

MetaEngineSettings::MetaEngineSettings()
    : d(new Private)
{
    readFromConfig();
    qRegisterMetaType<MetaEngineSettingsContainer>("MetaEngineSettingsContainer");
}

MetaEngineSettings::~MetaEngineSettings()
{
    delete d;
}

MetaEngineSettingsContainer MetaEngineSettings::settings() const
{
    QMutexLocker lock(&d->mutex);
    MetaEngineSettingsContainer s(d->settings);
    return s;
}

bool MetaEngineSettings::exifRotate() const
{
    return d->settings.exifRotate;
}

void MetaEngineSettings::setSettings(const MetaEngineSettingsContainer& settings)
{
    MetaEngineSettingsContainer old = d->setSettings(settings);
    emit settingsChanged();
    emit settingsChanged(settings, old);
    d->writeToConfig();
}

void MetaEngineSettings::readFromConfig()
{
    MetaEngineSettingsContainer s   = d->readFromConfig();
    MetaEngineSettingsContainer old = d->setSettings(s);
    emit settingsChanged();
    emit settingsChanged(s, old);
}

} // namespace Digikam
