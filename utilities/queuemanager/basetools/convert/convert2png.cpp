/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-28
 * Description : PNG image Converter batch tool.
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

#include "convert2png.moc"

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
#include "pngsettings.h"

namespace Digikam
{

Convert2PNG::Convert2PNG(QObject* const parent)
    : BatchTool("Convert2PNG", ConvertTool, parent)
{
    m_settings = 0;
    setToolTitle(i18n("Convert To PNG"));
    setToolDescription(i18n("Convert images to PNG format."));
    setToolIconName("image-png");
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
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("ImageViewer Settings");
    int compression           = group.readEntry("PNGCompression", 9);
    BatchToolSettings settings;
    settings.insert("Quality", compression);
    return settings;
}

void Convert2PNG::slotAssignSettings2Widget()
{
    m_settings->setCompressionValue(settings()["Quality"].toInt());
}

void Convert2PNG::slotSettingsChanged()
{
    BatchToolSettings settings;
    settings.insert("Quality", m_settings->getCompressionValue());
    BatchTool::slotSettingsChanged(settings);
}

QString Convert2PNG::outputSuffix() const
{
    return QString("png");
}

bool Convert2PNG::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    int PNGCompression = PNGSettings::convertCompressionForLibPng(settings()["Quality"].toInt());
    image().setAttribute("quality", PNGCompression);

    return (savefromDImg());
}

}  // namespace Digikam
