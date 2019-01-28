/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-11-28
 * Description : PNG image Converter batch tool.
 *
 * Copyright (C) 2008-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "converttopng.h"

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

namespace DigikamBqmConvertToPngPlugin
{

ConvertToPNG::ConvertToPNG(QObject* const parent)
    : BatchTool(QLatin1String("ConvertToPNG"), ConvertTool, parent)
{
    m_changeSettings = true;
}

ConvertToPNG::~ConvertToPNG()
{
}

void ConvertToPNG::registerSettingsWidget()
{
    PNGSettings* const PNGBox = new PNGSettings();

    connect(PNGBox, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    m_settingsWidget = PNGBox;

    BatchTool::registerSettingsWidget();
}

BatchToolSettings ConvertToPNG::defaultSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("ImageViewer Settings"));
    int compression           = group.readEntry(QLatin1String("PNGCompression"), 9);
    BatchToolSettings settings;
    settings.insert(QLatin1String("Quality"), compression);
    return settings;
}

void ConvertToPNG::slotAssignSettings2Widget()
{
    m_changeSettings = false;

    PNGSettings* const PNGBox = dynamic_cast<PNGSettings*>(m_settingsWidget);

    if (PNGBox)
    {
        PNGBox->setCompressionValue(settings()[QLatin1String("Quality")].toInt());
    }

    m_changeSettings = true;
}

void ConvertToPNG::slotSettingsChanged()
{
    if (m_changeSettings)
    {
        PNGSettings* const PNGBox = dynamic_cast<PNGSettings*>(m_settingsWidget);

        if (PNGBox)
        {
            BatchToolSettings settings;
            settings.insert(QLatin1String("Quality"), PNGBox->getCompressionValue());
            BatchTool::slotSettingsChanged(settings);
        }
    }
}

QString ConvertToPNG::outputSuffix() const
{
    return QLatin1String("png");
}

bool ConvertToPNG::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    int PNGCompression = PNGSettings::convertCompressionForLibPng(settings()[QLatin1String("Quality")].toInt());
    image().setAttribute(QLatin1String("quality"), PNGCompression);

    return (savefromDImg());
}

} // namespace DigikamBqmConvertToPngPlugin
