/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-09-14
 * Description : remove metadata batch tool.
 *
 * Copyright (C) 2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "removemetadata.moc"

// Qt includes

#include <QCheckBox>
#include <QWidget>
#include <QLabel>

// KDE includes

#include <klocale.h>
#include <kvbox.h>
#include <kglobal.h>

// Local includes

#include "dimg.h"
#include "dmetadata.h"

namespace Digikam
{

RemoveMetadata::RemoveMetadata(QObject* parent)
    : BatchTool("RemoveMetadata", MetadataTool, parent)
{
    m_removeExif = 0;
    m_removeIptc = 0;
    m_removeXmp  = 0;

    setToolTitle(i18n("Remove Metadata"));
    setToolDescription(i18n("Remove Exif, Iptc, or Xmp metadata from images."));
    setToolIconName("exifinfo");
}

RemoveMetadata::~RemoveMetadata()
{
}

void RemoveMetadata::registerSettingsWidget()
{
    KVBox* const vbox = new KVBox;
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
    settings.insert("RemoveExif", false);
    settings.insert("RemoveIptc", false);
    settings.insert("RemoveXmp",  false);
    return settings;
}

void RemoveMetadata::slotAssignSettings2Widget()
{
    m_removeExif->setChecked(settings()["RemoveExif"].toBool());
    m_removeIptc->setChecked(settings()["RemoveIptc"].toBool());
    m_removeXmp->setChecked(settings()["RemoveXmp"].toBool());
}

void RemoveMetadata::slotSettingsChanged()
{
    BatchToolSettings settings;
    settings.insert("RemoveExif", m_removeExif->isChecked());
    settings.insert("RemoveIptc", m_removeIptc->isChecked());
    settings.insert("RemoveXmp",  m_removeXmp->isChecked());
    BatchTool::slotSettingsChanged(settings);
}

bool RemoveMetadata::toolOperations()
{
    bool removeExif = settings()["RemoveExif"].toBool();
    bool removeIptc = settings()["RemoveIptc"].toBool();
    bool removeXmp  = settings()["RemoveXmp"].toBool();

    if (!loadToDImg())
    {
        return false;
    }

    DMetadata meta(image().getMetadata());

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

    image().setMetadata(meta.data());

    return (savefromDImg());
}

}  // namespace Digikam
