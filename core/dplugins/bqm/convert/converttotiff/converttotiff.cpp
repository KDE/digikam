/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-11-28
 * Description : TIFF image Converter batch tool.
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

#include "converttotiff.h"

// Qt includes

#include <QFileInfo>
#include <QWidget>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "dimg.h"
#include "tiffsettings.h"

namespace DigikamBqmConvertToTiffPlugin
{

ConvertToTIFF::ConvertToTIFF(QObject* const parent)
    : BatchTool(QLatin1String("ConvertToTIFF"), ConvertTool, parent)
{
    m_changeSettings = true;
}

ConvertToTIFF::~ConvertToTIFF()
{
}

void ConvertToTIFF::registerSettingsWidget()
{
    TIFFSettings* const TIFBox = new TIFFSettings();

    connect(TIFBox, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    m_settingsWidget = TIFBox;

    BatchTool::registerSettingsWidget();
}

BatchToolSettings ConvertToTIFF::defaultSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("ImageViewer Settings"));
    bool compression          = group.readEntry(QLatin1String("TIFFCompression"), false);
    BatchToolSettings settings;
    settings.insert(QLatin1String("Quality"), compression);
    return settings;
}

void ConvertToTIFF::slotAssignSettings2Widget()
{
    m_changeSettings = false;

    TIFFSettings* const TIFBox = dynamic_cast<TIFFSettings*>(m_settingsWidget);

    if (TIFBox)
    {
        TIFBox->setCompression(settings()[QLatin1String("compress")].toBool());
    }

    m_changeSettings = true;
}

void ConvertToTIFF::slotSettingsChanged()
{
    if (m_changeSettings)
    {
        TIFFSettings* const TIFBox = dynamic_cast<TIFFSettings*>(m_settingsWidget);

        if (TIFBox)
        {
            BatchToolSettings settings;
            settings.insert(QLatin1String("compress"), TIFBox->getCompression());
            BatchTool::slotSettingsChanged(settings);
        }
    }
}

QString ConvertToTIFF::outputSuffix() const
{
    return QLatin1String("tif");
}

bool ConvertToTIFF::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    image().setAttribute(QLatin1String("compress"), settings()[QLatin1String("compress")].toBool());

    return (savefromDImg());
}

} // namespace DigikamBqmConvertToTiffPlugin
