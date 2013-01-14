/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-28
 * Description : JPEG2000 image Converter batch tool.
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

#include "convert2jp2.moc"

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
#include "jp2ksettings.h"

namespace Digikam
{

Convert2JP2::Convert2JP2(QObject* const parent)
    : BatchTool("Convert2JP2", ConvertTool, parent)
{
    m_settings = 0;

    setToolTitle(i18n("Convert To JP2"));
    setToolDescription(i18n("Convert images to JPEG-2000 format."));
    setToolIconName("image-jp2");
}

Convert2JP2::~Convert2JP2()
{
}

void Convert2JP2::registerSettingsWidget()
{
    m_settings       = new JP2KSettings();
    m_settingsWidget = m_settings;

    connect(m_settings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings Convert2JP2::defaultSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("ImageViewer Settings");
    int compression           = group.readEntry("JPEG2000Compression", 75);
    bool lossLessCompression  = group.readEntry("JPEG2000LossLess",    true);
    BatchToolSettings settings;
    settings.insert("quality",  compression);
    settings.insert("lossless", lossLessCompression);
    return settings;
}

void Convert2JP2::slotAssignSettings2Widget()
{
    m_settings->setCompressionValue(settings()["quality"].toInt());
    m_settings->setLossLessCompression(settings()["lossless"].toBool());
}

void Convert2JP2::slotSettingsChanged()
{
    BatchToolSettings settings;
    settings.insert("quality",  m_settings->getCompressionValue());
    settings.insert("lossless", m_settings->getLossLessCompression());
    BatchTool::slotSettingsChanged(settings);
}

QString Convert2JP2::outputSuffix() const
{
    return QString("jp2");
}

bool Convert2JP2::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    bool lossless = settings()["lossless"].toBool();
    image().setAttribute("quality", lossless ? 100 : settings()["quality"].toInt());

    return (savefromDImg());
}

}  // namespace Digikam
