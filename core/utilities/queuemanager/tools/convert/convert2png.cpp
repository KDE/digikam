/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-28
 * Description : PNG image Converter batch tool.
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "convert2png.h"

// Qt includes

#include <QFileInfo>
#include <QWidget>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "dimg.h"
#include "pngsettings.h"

namespace Digikam
{

Convert2PNG::Convert2PNG(QObject* const parent)
    : BatchTool(QLatin1String("Convert2PNG"), ConvertTool, parent)
{
    m_settings = 0;
    m_changeSettings = true;

    setToolTitle(i18n("Convert To PNG"));
    setToolDescription(i18n("Convert images to PNG format."));
    setToolIconName(QLatin1String("image-png"));
}

Convert2PNG::~Convert2PNG()
{
}

void Convert2PNG::registerSettingsWidget()
{
    m_settings       = new PNGSettings();
    m_settingsWidget = m_settings;

    connect(m_settings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings Convert2PNG::defaultSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("ImageViewer Settings"));
    int compression           = group.readEntry(QLatin1String("PNGCompression"), 9);
    BatchToolSettings settings;
    settings.insert(QLatin1String("Quality"), compression);
    return settings;
}

void Convert2PNG::slotAssignSettings2Widget()
{
    m_changeSettings = false;
    m_settings->setCompressionValue(settings()[QLatin1String("Quality")].toInt());
    m_changeSettings = true;
}

void Convert2PNG::slotSettingsChanged()
{
    if (m_changeSettings)
    {
        BatchToolSettings settings;
        settings.insert(QLatin1String("Quality"), m_settings->getCompressionValue());
        BatchTool::slotSettingsChanged(settings);
    }
}

QString Convert2PNG::outputSuffix() const
{
    return QLatin1String("png");
}

bool Convert2PNG::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    int PNGCompression = PNGSettings::convertCompressionForLibPng(settings()[QLatin1String("Quality")].toInt());
    image().setAttribute(QLatin1String("quality"), PNGCompression);

    return (savefromDImg());
}

}  // namespace Digikam
