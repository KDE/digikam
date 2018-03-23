/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-09-14
 * Description : remove metadata batch tool.
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "removemetadata.h"

// Qt includes

#include <QCheckBox>
#include <QWidget>
#include <QLabel>
#include <QFile>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "dimg.h"
#include "dmetadata.h"

namespace Digikam
{

RemoveMetadata::RemoveMetadata(QObject* const parent)
    : BatchTool(QLatin1String("RemoveMetadata"), MetadataTool, parent)
{
    m_removeExif = 0;
    m_removeIptc = 0;
    m_removeXmp  = 0;

    setToolTitle(i18n("Remove Metadata"));
    setToolDescription(i18n("Remove Exif, Iptc, or Xmp metadata from images."));
    setToolIconName(QLatin1String("format-text-code"));
}

RemoveMetadata::~RemoveMetadata()
{
}

void RemoveMetadata::registerSettingsWidget()
{
    DVBox* const vbox = new DVBox;
    m_removeExif      = new QCheckBox(i18n("Remove Exif"), vbox);
    m_removeIptc      = new QCheckBox(i18n("Remove Iptc"), vbox);
    m_removeXmp       = new QCheckBox(i18n("Remove Xmp"), vbox);
    QLabel* space     = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    m_settingsWidget = vbox;

    connect(m_removeExif, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(m_removeIptc, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(m_removeXmp, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings RemoveMetadata::defaultSettings()
{
    BatchToolSettings settings;
    settings.insert(QLatin1String("RemoveExif"), false);
    settings.insert(QLatin1String("RemoveIptc"), false);
    settings.insert(QLatin1String("RemoveXmp"),  false);
    return settings;
}

void RemoveMetadata::slotAssignSettings2Widget()
{
    m_removeExif->setChecked(settings()[QLatin1String("RemoveExif")].toBool());
    m_removeIptc->setChecked(settings()[QLatin1String("RemoveIptc")].toBool());
    m_removeXmp->setChecked(settings()[QLatin1String("RemoveXmp")].toBool());
}

void RemoveMetadata::slotSettingsChanged()
{
    BatchToolSettings settings;
    settings.insert(QLatin1String("RemoveExif"), m_removeExif->isChecked());
    settings.insert(QLatin1String("RemoveIptc"), m_removeIptc->isChecked());
    settings.insert(QLatin1String("RemoveXmp"),  m_removeXmp->isChecked());
    BatchTool::slotSettingsChanged(settings);
}

bool RemoveMetadata::toolOperations()
{
    if (!isLastChainedTool())
    {
        setErrorDescription(i18n("Remove Metadata: Not the last tool in the list."));
        return false;
    }

    bool ret = true;
    DMetadata meta;

    if (image().isNull())
    {
        QFile::remove(outputUrl().toLocalFile());
        ret = QFile::copy(inputUrl().toLocalFile(), outputUrl().toLocalFile());

        if (!ret || !meta.load(outputUrl().toLocalFile()))
        {
            return ret;
        }
    }
    else
    {
        ret = savefromDImg();
        meta.setData(image().getMetadata());
    }

    bool removeExif = settings()[QLatin1String("RemoveExif")].toBool();
    bool removeIptc = settings()[QLatin1String("RemoveIptc")].toBool();
    bool removeXmp  = settings()[QLatin1String("RemoveXmp")].toBool();

    if (removeExif)
    {
        meta.clearExif();
    }

    if (removeIptc)
    {
        meta.clearIptc();
    }

    if (removeXmp)
    {
        meta.clearXmp();
    }

    if (ret && (removeExif || removeIptc || removeXmp))
    {
        ret = meta.save(outputUrl().toLocalFile(), false);
    }

    return ret;
}

} // namespace Digikam
