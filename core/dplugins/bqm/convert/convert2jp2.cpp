/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-28
 * Description : JPEG2000 image Converter batch tool.
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

#include "convert2jp2.h"

// Qt includes

#include <QFileInfo>
#include <QWidget>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "dimg.h"
#include "jp2ksettings.h"

namespace Digikam
{

Convert2JP2::Convert2JP2(QObject* const parent)
    : BatchTool(QLatin1String("Convert2JP2"), ConvertTool, parent)
{
    m_changeSettings = true;
}

Convert2JP2::~Convert2JP2()
{
}

void Convert2JP2::registerSettingsWidget()
{
    JP2KSettings* const JP2Box = new JP2KSettings();

    connect(JP2Box, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    m_settingsWidget = JP2Box;

    BatchTool::registerSettingsWidget();
}

BatchToolSettings Convert2JP2::defaultSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("ImageViewer Settings"));
    int compression           = group.readEntry(QLatin1String("JPEG2000Compression"), 75);
    bool lossLessCompression  = group.readEntry(QLatin1String("JPEG2000LossLess"),    true);
    BatchToolSettings settings;
    settings.insert(QLatin1String("quality"),  compression);
    settings.insert(QLatin1String("lossless"), lossLessCompression);

    return settings;
}

void Convert2JP2::slotAssignSettings2Widget()
{
    m_changeSettings           = false;
    JP2KSettings* const JP2Box = dynamic_cast<JP2KSettings*>(m_settingsWidget);

    if (JP2Box)
    {
        JP2Box->setCompressionValue(settings()[QLatin1String("quality")].toInt());
        JP2Box->setLossLessCompression(settings()[QLatin1String("lossless")].toBool());
    }

    m_changeSettings = true;
}

void Convert2JP2::slotSettingsChanged()
{
    if (m_changeSettings)
    {
        BatchToolSettings settings;
        JP2KSettings* const JP2Box = dynamic_cast<JP2KSettings*>(m_settingsWidget);

        if (JP2Box)
        {
            settings.insert(QLatin1String("quality"),  JP2Box->getCompressionValue());
            settings.insert(QLatin1String("lossless"), JP2Box->getLossLessCompression());
            BatchTool::slotSettingsChanged(settings);
        }
    }
}

QString Convert2JP2::outputSuffix() const
{
    return QLatin1String("jp2");
}

bool Convert2JP2::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    bool lossless = settings()[QLatin1String("lossless")].toBool();
    image().setAttribute(QLatin1String("quality"), lossless ? 100 : settings()[QLatin1String("quality")].toInt());

    return (savefromDImg());
}

} // namespace Digikam
