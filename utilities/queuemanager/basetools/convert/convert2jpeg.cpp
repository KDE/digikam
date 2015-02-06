/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-24
 * Description : JPEG image Converter batch tool.
 *
 * Copyright (C) 2008-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "convert2jpeg.moc"

// Qt includes

#include <QFileInfo>
#include <QWidget>

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocale.h>
#include <kglobal.h>

// Local includes

#include "dimg.h"
#include "jpegsettings.h"

namespace Digikam
{

Convert2JPEG::Convert2JPEG(QObject* const parent)
    : BatchTool("Convert2JPEG", ConvertTool, parent)
{
    m_settings = 0;
    m_changeSettings = true;

    setToolTitle(i18n("Convert To JPEG"));
    setToolDescription(i18n("Convert images to JPEG format."));
    setToolIconName("image-jpeg");
}

Convert2JPEG::~Convert2JPEG()
{
}

void Convert2JPEG::registerSettingsWidget()
{
    m_settings       = new JPEGSettings;
    m_settingsWidget = m_settings;

    connect(m_settings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings Convert2JPEG::defaultSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("ImageViewer Settings");
    int compression           = group.readEntry("JPEGCompression", 75);
    int subSampling           = group.readEntry("JPEGSubSampling", 1);  // Medium subsampling
    BatchToolSettings settings;
    settings.insert("Quality",     compression);
    settings.insert("SubSampling", subSampling);
    return settings;
}

void Convert2JPEG::slotAssignSettings2Widget()
{
    m_changeSettings = false;
    m_settings->setCompressionValue(settings()["Quality"].toInt());
    m_settings->setSubSamplingValue(settings()["SubSampling"].toInt());
    m_changeSettings = true;
}

void Convert2JPEG::slotSettingsChanged()
{
    if (m_changeSettings)
    {
        BatchToolSettings settings;
        settings.insert("Quality",     m_settings->getCompressionValue());
        settings.insert("SubSampling", m_settings->getSubSamplingValue());
        BatchTool::slotSettingsChanged(settings);
    }
}

QString Convert2JPEG::outputSuffix() const
{
    return QString("jpg");
}

bool Convert2JPEG::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    int JPEGCompression = JPEGSettings::convertCompressionForLibJpeg(settings()["Quality"].toInt());
    image().setAttribute("quality",     JPEGCompression);
    image().setAttribute("subsampling", settings()["SubSampling"].toInt());

    return (savefromDImg());
}

}  // namespace Digikam
