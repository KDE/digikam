/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-12-20
 * Description : Settings for the Showfoto tool
 *
 * Copyright (C) 2013 by Mohamed Anwer <mohammed dot ahmed dot anwer at gmail dot com>
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

#include "showfotosettings.moc"

// KDE includes

#include <kglobal.h>
#include <kglobalsettings.h>
#include <kconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "setupmisc.h"

namespace ShowFoto
{

class ShowfotoSettings::Private
{

public:

    Private() :
        drawFormatOverThumbnail(false)
    {
    }

    static const QString configGroupDefault;

    bool                 drawFormatOverThumbnail;

    KSharedConfigPtr     config;
};

const QString ShowfotoSettings::Private::configGroupDefault("ImageViewer Settings");

// -------------------------------------------------------------------------------------------------

class ShowfotoSettingsCreator
{
public:

    ShowfotoSettings object;
};

K_GLOBAL_STATIC(ShowfotoSettingsCreator, creator)

// -------------------------------------------------------------------------------------------------

ShowfotoSettings* ShowfotoSettings::instance()
{
    return &creator->object;
}

ShowfotoSettings::ShowfotoSettings()
    : QObject(), d(new Private)
{
    d->config = KGlobal::config();
    init();
    readSettings();
}

ShowfotoSettings::~ShowfotoSettings()
{
    delete d;
}

void ShowfotoSettings::init()
{
    d->drawFormatOverThumbnail = false;
}

void ShowfotoSettings::readSettings()
{
    KSharedConfigPtr config    = d->config;

    KConfigGroup group         = config->group(d->configGroupDefault);

    d->drawFormatOverThumbnail = group.readEntry("ShowMimeOverImage", false);
}

bool ShowfotoSettings::getShowFormatOverThumbnail()
{
    return d->drawFormatOverThumbnail;
}

void ShowfotoSettings::saveSettings()
{
    KSharedConfigPtr config = d->config;

    KConfigGroup group      = config->group(d->configGroupDefault);

    group.writeEntry("ShowMimeOverImage", getShowFormatOverThumbnail());
}

} // namespace Showfoto
