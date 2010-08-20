/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-20
 * Description : central place for Metadata settings
 *
 * Copyright (C) 2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "metadatasettings.moc"

// Qt includes

#include <QApplication>
#include <QDir>
#include <QFileInfo>

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <ksharedconfig.h>
#include <kdebug.h>

// Local includes

#include "dmetadata.h"

namespace Digikam
{

class MetadataSettings::MetadataSettingsPriv
{
public:

    MetadataSettingsPriv()
    {
    }

    MetadataSettingsContainer settings;
};

// -----------------------------------------------------------------------------------------------

class MetadataSettingsCreator
{
public:

    MetadataSettings object;
};

K_GLOBAL_STATIC(MetadataSettingsCreator, metatadaSettingsCreator)

// -----------------------------------------------------------------------------------------------

MetadataSettings* MetadataSettings::instance()
{
    return &metatadaSettingsCreator->object;
}

MetadataSettings::MetadataSettings()
                : d(new MetadataSettingsPriv)
{
    readFromConfig();
    qRegisterMetaType<MetadataSettingsContainer>("MetadataSettingsContainer");
}

MetadataSettings::~MetadataSettings()
{
    delete d;
}

MetadataSettingsContainer MetadataSettings::settings()
{
    MetadataSettingsContainer s(d->settings);
    return s;
}

void MetadataSettings::setSettings(const MetadataSettingsContainer& settings)
{
    MetadataSettingsContainer old;
    {
        old         = d->settings;
        d->settings = settings;
    }
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("Metadata Settings"));
    settings.writeToConfig(group);
    emit settingsChanged();
    emit settingsChanged(settings, old);
}

void MetadataSettings::readFromConfig()
{
    MetadataSettingsContainer old, s;
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("Metadata Settings"));
    s.readFromConfig(group);
    {
        old         = d->settings;
        d->settings = s;
    }
    emit settingsChanged();
    emit settingsChanged(s, old);
}

}  // namespace Digikam
