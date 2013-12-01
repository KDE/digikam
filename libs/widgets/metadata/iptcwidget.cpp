/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-20
 * Description : A widget to display IPTC metadata
 *
 * Copyright (C) 2006-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "iptcwidget.moc"

// Qt includes

#include <QMap>
#include <QFile>

// KDE includes

#include <klocale.h>

// Local includes

#include "dmetadata.h"

namespace
{
static const char* StandardIptcEntryList[] =
{
    "Envelope",
    "Application2",
    "-1"
};
}

namespace Digikam
{

IptcWidget::IptcWidget(QWidget* const parent, const char* name)
    : MetadataWidget(parent, name)
{
    for (int i=0 ; QString(StandardIptcEntryList[i]) != QString("-1") ; ++i)
    {
        m_keysFilter << StandardIptcEntryList[i];
    }
}

IptcWidget::~IptcWidget()
{
}

QString IptcWidget::getMetadataTitle()
{
    return i18n("IPTC Records");
}

bool IptcWidget::loadFromURL(const KUrl& url)
{
    setFileName(url.fileName());

    if (url.isEmpty())
    {
        setMetadata();
        return false;
    }
    else
    {
        DMetadata metadata(url.toLocalFile());

        if (!metadata.hasIptc())
        {
            setMetadata();
            return false;
        }
        else
        {
            setMetadata(metadata);
        }
    }

    return true;
}

bool IptcWidget::decodeMetadata()
{
    DMetadata data = getMetadata();

    if (!data.hasIptc())
    {
        return false;
    }

    // Update all metadata contents.
    setMetadataMap(data.getIptcTagsDataList(m_keysFilter));
    return true;
}

void IptcWidget::buildView()
{
    switch (getMode())
    {
        case CUSTOM:
            setIfdList(getMetadataMap(), m_keysFilter, getTagsFilter());
            break;

        case PHOTO:
            setIfdList(getMetadataMap(), m_keysFilter, QStringList() << QString("FULL"));
            break;

        default: // NONE
            setIfdList(getMetadataMap(), QStringList());
            break;
    }

    MetadataWidget::buildView();
}

QString IptcWidget::getTagTitle(const QString& key)
{
    DMetadata metadataIface;
    QString title = metadataIface.getIptcTagTitle(key.toAscii());

    if (title.isEmpty())
    {
        return key.section('.', -1);
    }

    return title;
}

QString IptcWidget::getTagDescription(const QString& key)
{
    DMetadata metadataIface;
    QString desc = metadataIface.getIptcTagDescription(key.toAscii());

    if (desc.isEmpty())
    {
        return i18n("No description available");
    }

    return desc;
}

void IptcWidget::slotSaveMetadataToFile()
{
    KUrl url = saveMetadataToFile(i18n("IPTC File to Save"),
                                  QString("*.iptc|"+i18n("IPTC binary Files (*.iptc)")));
    storeMetadataToFile(url, getMetadata().getIptc());
}

}  // namespace Digikam
