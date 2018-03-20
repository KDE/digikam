/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-06-22
 * Description : central place for DMetadata settings
 *
 * Copyright (C) 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

#include "dmetadatasettings.h"

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

class DMetadataSettings::Private
{
public:

    Private()
        : mutex(),
          configGroup(QLatin1String("DMetadata Settings"))
    {
    }

    DMetadataSettingsContainer settings;
    QMutex                     mutex;

    const QString              configGroup;

public:

    DMetadataSettingsContainer readFromConfig() const;
    void                       writeToConfig() const;
    DMetadataSettingsContainer setSettings(const DMetadataSettingsContainer& s);
};

DMetadataSettingsContainer DMetadataSettings::Private::readFromConfig() const
{
    DMetadataSettingsContainer s;
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroup);
    s.readFromConfig(group);
    return s;
}

void DMetadataSettings::Private::writeToConfig() const
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroup);
    settings.writeToConfig(group);
}

DMetadataSettingsContainer DMetadataSettings::Private::setSettings(const DMetadataSettingsContainer& s)
{
    QMutexLocker lock(&mutex);
    DMetadataSettingsContainer old;
    old      = settings;
    settings = s;
    return old;
}

// -----------------------------------------------------------------------------------------------

class DMetadataSettingsCreator
{
public:

    DMetadataSettings object;
};

Q_GLOBAL_STATIC(DMetadataSettingsCreator, dmetatadaSettingsCreator)

// -----------------------------------------------------------------------------------------------

DMetadataSettings* DMetadataSettings::instance()
{
    return &dmetatadaSettingsCreator->object;
}

DMetadataSettings::DMetadataSettings()
    : d(new Private)
{
    readFromConfig();
    qRegisterMetaType<DMetadataSettingsContainer>("DMetadataSettingsContainer");
}

DMetadataSettings::~DMetadataSettings()
{
    delete d;
}

DMetadataSettingsContainer DMetadataSettings::settings() const
{
    QMutexLocker lock(&d->mutex);
    DMetadataSettingsContainer s(d->settings);
    return s;
}

void DMetadataSettings::setSettings(const DMetadataSettingsContainer& settings)
{
    DMetadataSettingsContainer old = d->setSettings(settings);
    emit dmetadataSettingsChanged();
    emit dmetadataSettingsChanged(settings, old);
    d->writeToConfig();
}

void DMetadataSettings::readFromConfig()
{
    DMetadataSettingsContainer s   = d->readFromConfig();
    DMetadataSettingsContainer old = d->setSettings(s);
    emit dmetadataSettingsChanged();
    emit dmetadataSettingsChanged(s, old);
}

}  // namespace Digikam
