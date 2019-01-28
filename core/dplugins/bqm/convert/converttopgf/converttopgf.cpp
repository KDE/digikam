/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-18
 * Description : PGF image Converter batch tool.
 *
 * Copyright (C) 2009-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "converttopgf.h"

// Qt includes

#include <QFileInfo>
#include <QWidget>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "dimg.h"
#include "pgfsettings.h"

namespace DigikamBqmConvertToPgfPlugin
{

ConvertToPGF::ConvertToPGF(QObject* const parent)
    : BatchTool(QLatin1String("ConvertToPGF"), ConvertTool, parent)
{
    m_changeSettings = true;
}

ConvertToPGF::~ConvertToPGF()
{
}

void ConvertToPGF::registerSettingsWidget()
{
    PGFSettings* const PGFBox = new PGFSettings();

    connect(PGFBox, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    m_settingsWidget          = PGFBox;

    BatchTool::registerSettingsWidget();
}

BatchToolSettings ConvertToPGF::defaultSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("ImageViewer Settings"));
    int compression           = group.readEntry(QLatin1String("PGFCompression"), 3);
    bool lossLessCompression  = group.readEntry(QLatin1String("PGFLossLess"),    true);
    BatchToolSettings settings;
    settings.insert(QLatin1String("quality"),  compression);
    settings.insert(QLatin1String("lossless"), lossLessCompression);
    return settings;
}

void ConvertToPGF::slotAssignSettings2Widget()
{
    m_changeSettings = false;

    PGFSettings* const PGFBox = dynamic_cast<PGFSettings*>(m_settingsWidget);

    if (PGFBox)
    {
        PGFBox->setCompressionValue(settings()[QLatin1String("quality")].toInt());
        PGFBox->setLossLessCompression(settings()[QLatin1String("lossless")].toBool());
    }

    m_changeSettings = true;
}

void ConvertToPGF::slotSettingsChanged()
{
    if (m_changeSettings)
    {
        PGFSettings* const PGFBox = dynamic_cast<PGFSettings*>(m_settingsWidget);

        if (PGFBox)
        {
            BatchToolSettings settings;
            settings.insert(QLatin1String("quality"),  PGFBox->getCompressionValue());
            settings.insert(QLatin1String("lossless"), PGFBox->getLossLessCompression());
            BatchTool::slotSettingsChanged(settings);
        }
    }
}

QString ConvertToPGF::outputSuffix() const
{
    return QLatin1String("pgf");
}

bool ConvertToPGF::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    bool lossless = settings()[QLatin1String("lossless")].toBool();
    image().setAttribute(QLatin1String("quality"), lossless ? 0 : settings()[QLatin1String("quality")].toInt());

    return (savefromDImg());
}

} // namespace DigikamBqmConvertToPgfPlugin
