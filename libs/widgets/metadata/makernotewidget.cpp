/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-20
 * Description : a widget to display non standard Exif metadata
 *               used by camera makers
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "makernotewidget.h"

// Qt includes

#include <QMap>
#include <QFile>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dmetadata.h"

namespace Digikam
{

static const char* ExifEntryListToIgnore[] =
{
    "GPSInfo",
    "Iop",
    "Thumbnail",
    "SubImage1",
    "SubImage2",
    "Image",
    "Photo",
    "-1"
};

MakerNoteWidget::MakerNoteWidget(QWidget* const parent, const QString& name)
    : MetadataWidget(parent, name)
{
    for (int i=0 ; QLatin1String(ExifEntryListToIgnore[i]) != QLatin1String("-1") ; ++i)
    {
        m_keysFilter << QLatin1String(ExifEntryListToIgnore[i]);
    }
}

MakerNoteWidget::~MakerNoteWidget()
{
}

QString MakerNoteWidget::getMetadataTitle()
{
    return i18n("MakerNote EXIF Tags");
}

bool MakerNoteWidget::loadFromURL(const QUrl& url)
{
    setFileName(url.toLocalFile());

    if (url.isEmpty())
    {
        setMetadata();
        return false;
    }
    else
    {
        DMetadata metadata(url.toLocalFile());

        if (!metadata.hasExif())
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

bool MakerNoteWidget::decodeMetadata()
{
    DMetadata data = getMetadata();

    if (!data.hasExif())
    {
        return false;
    }

    // Update all metadata contents.
    setMetadataMap(data.getExifTagsDataList(m_keysFilter, true));
    return true;
}

void MakerNoteWidget::buildView()
{
    switch (getMode())
    {
        case CUSTOM:
            setIfdList(getMetadataMap(), getTagsFilter());
            break;

        case PHOTO:
            setIfdList(getMetadataMap(), QStringList() << QLatin1String("FULL"));
            break;

        default: // NONE
            setIfdList(getMetadataMap(), QStringList());
            break;
    }

    MetadataWidget::buildView();
}

QString MakerNoteWidget::getTagTitle(const QString& key)
{
    DMetadata metadataIface;
    QString title = metadataIface.getExifTagTitle(key.toLatin1().constData());

    if (title.isEmpty())
    {
        return key.section(QLatin1Char('.'), -1);
    }

    return title;
}

QString MakerNoteWidget::getTagDescription(const QString& key)
{
    DMetadata metadataIface;
    QString desc = metadataIface.getExifTagDescription(key.toLatin1().constData());

    if (desc.isEmpty())
    {
        return i18n("No description available");
    }

    return desc;
}

void MakerNoteWidget::slotSaveMetadataToFile()
{
    QUrl url = saveMetadataToFile(i18n("EXIF File to Save"),
                                  QString(QLatin1String("*.exif|") + i18n("EXIF binary Files (*.exif)")));

    storeMetadataToFile(url, getMetadata().getExifEncoded());
}

}  // namespace Digikam
