/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-18
 * Description : PGF image Converter batch tool.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "convert2pgf.h"

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

namespace Digikam
{

Convert2PGF::Convert2PGF(QObject* const parent)
    : BatchTool(QLatin1String("Convert2PGF"), ConvertTool, parent)
{
    m_settings = 0;
    m_changeSettings = true;

    setToolTitle(i18n("Convert To PGF"));
    setToolDescription(i18n("Convert images to PGF format."));
    setToolIconName(QLatin1String("image-jpeg"));
}

Convert2PGF::~Convert2PGF()
{
}

void Convert2PGF::registerSettingsWidget()
{
    m_settings       = new PGFSettings();
    m_settingsWidget = m_settings;

    connect(m_settings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings Convert2PGF::defaultSettings()
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

void Convert2PGF::slotAssignSettings2Widget()
{
    m_changeSettings = false;
    m_settings->setCompressionValue(settings()[QLatin1String("quality")].toInt());
    m_settings->setLossLessCompression(settings()[QLatin1String("lossless")].toBool());
    m_changeSettings = true;
}

void Convert2PGF::slotSettingsChanged()
{
    if (m_changeSettings)
    {
        BatchToolSettings settings;
        settings.insert(QLatin1String("quality"),  m_settings->getCompressionValue());
        settings.insert(QLatin1String("lossless"), m_settings->getLossLessCompression());
        BatchTool::slotSettingsChanged(settings);
    }
}

QString Convert2PGF::outputSuffix() const
{
    return QLatin1String("pgf");
}

bool Convert2PGF::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    bool lossless = settings()[QLatin1String("lossless")].toBool();
    image().setAttribute(QLatin1String("quality"), lossless ? 0 : settings()[QLatin1String("quality")].toInt());

    return (savefromDImg());
}

}  // namespace Digikam
