/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-24
 * Description : JPEG image Converter batch tool.
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

#include "convert2jpeg.h"

// Qt includes

#include <QFileInfo>
#include <QWidget>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "dimg.h"
#include "jpegsettings.h"

namespace Digikam
{

Convert2JPEG::Convert2JPEG(QObject* const parent)
    : BatchTool(QLatin1String("Convert2JPEG"), ConvertTool, parent)
{
    m_changeSettings = true;
}

Convert2JPEG::~Convert2JPEG()
{
}

void Convert2JPEG::registerSettingsWidget()
{
    JPEGSettings* const JPGBox = new JPEGSettings;

    connect(JPGBox, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    m_settingsWidget = JPGBox;

    BatchTool::registerSettingsWidget();
}

BatchToolSettings Convert2JPEG::defaultSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("ImageViewer Settings"));
    int compression           = group.readEntry(QLatin1String("JPEGCompression"), 75);
    int subSampling           = group.readEntry(QLatin1String("JPEGSubSampling"), 1);  // Medium subsampling
    BatchToolSettings settings;
    settings.insert(QLatin1String("Quality"),     compression);
    settings.insert(QLatin1String("SubSampling"), subSampling);
    return settings;
}

void Convert2JPEG::slotAssignSettings2Widget()
{
    m_changeSettings = false;

    JPEGSettings* const JPGBox = dynamic_cast<JPEGSettings*>(m_settingsWidget);

    if (JPGBox)
    {
        JPGBox->setCompressionValue(settings()[QLatin1String("Quality")].toInt());
        JPGBox->setSubSamplingValue(settings()[QLatin1String("SubSampling")].toInt());
    }

    m_changeSettings = true;
}

void Convert2JPEG::slotSettingsChanged()
{
    if (m_changeSettings)
    {
        JPEGSettings* const JPGBox = dynamic_cast<JPEGSettings*>(m_settingsWidget);

        if (JPGBox)
        {
            BatchToolSettings settings;
            settings.insert(QLatin1String("Quality"),     JPGBox->getCompressionValue());
            settings.insert(QLatin1String("SubSampling"), JPGBox->getSubSamplingValue());
            BatchTool::slotSettingsChanged(settings);
        }
    }
}

QString Convert2JPEG::outputSuffix() const
{
    return QLatin1String("jpg");
}

bool Convert2JPEG::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    int JPEGCompression = JPEGSettings::convertCompressionForLibJpeg(settings()[QLatin1String("Quality")].toInt());
    image().setAttribute(QLatin1String("quality"),     JPEGCompression);
    image().setAttribute(QLatin1String("subsampling"), settings()[QLatin1String("SubSampling")].toInt());

    return (savefromDImg());
}

} // namespace Digikam
