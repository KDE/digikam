/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-20
 * Description : A widget to display IPTC metadata
 * 
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qmap.h>
#include <qfile.h>

// KDE includes.

#include <klocale.h>

// Local includes.

#include "ddebug.h"
#include "dmetadata.h"
#include "iptcwidget.h"
#include "iptcwidget.moc"

namespace Digikam
{

static const char* IptcHumanList[] =
{
     "Caption",
     "City",
     "Contact",
     "Copyright",
     "Credit",
     "DateCreated",
     "Headline",
     "Keywords",
     "ProvinceState",
     "Source",
     "Urgency",
     "Writer",
     "-1"
};

static const char* StandardIptcEntryList[] =
{
     "Envelope",
     "Application2",
     "-1"
};

IptcWidget::IptcWidget(QWidget* parent, const char* name)
          : MetadataWidget(parent, name)
{
    for (int i=0 ; QString(StandardIptcEntryList[i]) != QString("-1") ; i++)
        m_keysFilter << StandardIptcEntryList[i];

    for (int i=0 ; QString(IptcHumanList[i]) != QString("-1") ; i++)
        m_tagsfilter << IptcHumanList[i];
}

IptcWidget::~IptcWidget()
{
}

QString IptcWidget::getMetadataTitle()
{
    return i18n("IPTC Records");
}

bool IptcWidget::loadFromURL(const KURL& url)
{
    setFileName(url.filename());

    if (url.isEmpty())
    {
        setMetadata();
        return false;
    }
    else
    {    
        DMetadata metadata(url.path());
        QByteArray iptcData = metadata.getIptc();

        if (iptcData.isEmpty())
        {
            setMetadata();
            return false;
        }
        else
            setMetadata(iptcData);
    }

    return true;
}

bool IptcWidget::decodeMetadata()
{
    DMetadata metaData;
    if (!metaData.setIptc(getMetadata()))
        return false;

    // Update all metadata contents.
    setMetadataMap(metaData.getIptcTagsDataList(m_keysFilter));
    return true;
}

void IptcWidget::buildView()
{
    if (getMode() == SIMPLE)
    {
        setIfdList(getMetadataMap(), m_tagsfilter);
    }
    else
    {
        setIfdList(getMetadataMap());
    }

    MetadataWidget::buildView();
}

QString IptcWidget::getTagTitle(const QString& key)
{
    DMetadata meta;
    QString title = meta.getIptcTagTitle(key.ascii());

    if (title.isEmpty())
        return key.section('.', -1);

    return title;
}

QString IptcWidget::getTagDescription(const QString& key)
{
    DMetadata meta;
    QString desc = meta.getIptcTagDescription(key.ascii());

    if (desc.isEmpty())
        return i18n("No description available");

    return desc;
}

void IptcWidget::slotSaveMetadataToFile()
{
    KURL url = saveMetadataToFile(i18n("IPTC File to Save"),
                                  QString("*.iptc|"+i18n("IPTC binary Files (*.iptc)")));
    storeMetadataToFile(url);
}

}  // namespace Digikam

