/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-28
 * Description : TIFF image Converter batch tool.
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

#include "convert2tiff.h"

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

namespace Digikam
{

Convert2TIFF::Convert2TIFF(QObject* const parent)
    : BatchTool(QLatin1String("Convert2TIFF"), ConvertTool, parent)
{
    m_settings = 0;

    setToolTitle(i18n("Convert To TIFF"));
    setToolDescription(i18n("Convert images to TIFF format."));
    setToolIconName(QLatin1String("image-tiff"));
}

Convert2TIFF::~Convert2TIFF()
{
}

void Convert2TIFF::registerSettingsWidget()
{
    m_settings       = new TIFFSettings();
    m_settingsWidget = m_settings;

    connect(m_settings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings Convert2TIFF::defaultSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("ImageViewer Settings"));
    bool compression          = group.readEntry(QLatin1String("TIFFCompression"), false);
    BatchToolSettings settings;
    settings.insert(QLatin1String("Quality"), compression);
    return settings;
}

void Convert2TIFF::slotAssignSettings2Widget()
{
    m_settings->setCompression(settings()[QLatin1String("compress")].toBool());
}

void Convert2TIFF::slotSettingsChanged()
{
    BatchToolSettings settings;
    settings.insert(QLatin1String("compress"), m_settings->getCompression());
    BatchTool::slotSettingsChanged(settings);
}

QString Convert2TIFF::outputSuffix() const
{
    return QLatin1String("tif");
}

bool Convert2TIFF::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    image().setAttribute(QLatin1String("compress"), settings()[QLatin1String("compress")].toBool());

    return (savefromDImg());
}

}  // namespace Digikam
