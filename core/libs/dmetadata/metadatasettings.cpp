/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-20
 * Description : central place for Metadata settings
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "metadatasettings.h"

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

class MetadataSettings::Private
{
public:

    Private()
        : mutex(),
          configGroup(QLatin1String("Metadata Settings"))
    {
    }

    MetadataSettingsContainer settings;
    QMutex                    mutex;

    const QString             configGroup;

public:

    MetadataSettingsContainer readFromConfig() const;
    void                      writeToConfig() const;
    MetadataSettingsContainer setSettings(const MetadataSettingsContainer& s);
};

MetadataSettingsContainer MetadataSettings::Private::readFromConfig() const
{
    MetadataSettingsContainer s;
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroup);
    s.readFromConfig(group);
    return s;
}

void MetadataSettings::Private::writeToConfig() const
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroup);
    settings.writeToConfig(group);
}

MetadataSettingsContainer MetadataSettings::Private::setSettings(const MetadataSettingsContainer& s)
{
    QMutexLocker lock(&mutex);
    MetadataSettingsContainer old;
    old      = settings;
    settings = s;
    return old;
}

// -----------------------------------------------------------------------------------------------

class MetadataSettingsCreator
{
public:

    MetadataSettings object;
};

Q_GLOBAL_STATIC(MetadataSettingsCreator, metatadaSettingsCreator)

// -----------------------------------------------------------------------------------------------

MetadataSettings* MetadataSettings::instance()
{
    return &metatadaSettingsCreator->object;
}

MetadataSettings::MetadataSettings()
    : d(new Private)
{
    readFromConfig();
    qRegisterMetaType<MetadataSettingsContainer>("MetadataSettingsContainer");
}

MetadataSettings::~MetadataSettings()
{
    delete d;
}

MetadataSettingsContainer MetadataSettings::settings() const
{
    QMutexLocker lock(&d->mutex);
    MetadataSettingsContainer s(d->settings);
    return s;
}

bool MetadataSettings::exifRotate() const
{
    return d->settings.exifRotate;
}

void MetadataSettings::setSettings(const MetadataSettingsContainer& settings)
{
    MetadataSettingsContainer old = d->setSettings(settings);
    emit settingsChanged();
    emit settingsChanged(settings, old);
    d->writeToConfig();
}

void MetadataSettings::readFromConfig()
{
    MetadataSettingsContainer s   = d->readFromConfig();
    MetadataSettingsContainer old = d->setSettings(s);
    emit settingsChanged();
    emit settingsChanged(s, old);
}

}  // namespace Digikam
