/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-28
 * Description : TIFF image Converter batch tool.
 *
 * Copyright (C) 2008-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "convert2tiff.moc"

// Qt includes

#include <QFileInfo>
#include <QWidget>

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kiconloader.h>
#include <klocale.h>

// Local includes

#include "dimg.h"
#include "tiffsettings.h"

namespace Digikam
{

Convert2TIFF::Convert2TIFF(QObject* parent)
    : BatchTool("Convert2TIFF", ConvertTool, parent)
{
    setToolTitle(i18n("Convert To TIFF"));
    setToolDescription(i18n("Convert images to TIFF format."));
    setToolIcon(KIcon(SmallIcon("image-tiff")));

    m_settings = new TIFFSettings();
    setSettingsWidget(m_settings);

    connect(m_settings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));
}

Convert2TIFF::~Convert2TIFF()
{
}

BatchToolSettings Convert2TIFF::defaultSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("ImageViewer Settings");
    bool compression          = group.readEntry("TIFFCompression", false);
    BatchToolSettings settings;
    settings.insert("Quality", compression);
    return settings;
}

void Convert2TIFF::slotAssignSettings2Widget()
{
    m_settings->setCompression(settings()["compress"].toBool());
}

void Convert2TIFF::slotSettingsChanged()
{
    BatchToolSettings settings;
    settings.insert("compress", m_settings->getCompression());
    BatchTool::slotSettingsChanged(settings);
}

QString Convert2TIFF::outputSuffix() const
{
    return QString("tif");
}

bool Convert2TIFF::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    image().setAttribute("compress", settings()["compress"].toBool());

    return (savefromDImg());
}

}  // namespace Digikam
