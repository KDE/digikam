/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-20
 * Description : a widget to display Standard Exif metadata
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

#include "exifwidget.h"

// Qt includes

#include <QMap>
#include <QFile>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dmetadata.h"
#include "metadatalistview.h"

namespace
{
// Standard Exif Entry list from to less important to the most important for photograph.
static const char* StandardExifEntryList[] =
{
    "Iop",
    "Thumbnail",
    "SubImage1",
    "SubImage2",
    "Image",
    "Photo",
    "GPSInfo",
    "-1"
};
}

namespace Digikam
{

ExifWidget::ExifWidget(QWidget* const parent, const QString& name)
    : MetadataWidget(parent, name)
{
    for (int i=0 ; QLatin1String(StandardExifEntryList[i]) != QLatin1String("-1") ; ++i)
    {
        m_keysFilter << QLatin1String(StandardExifEntryList[i]);
    }
}

ExifWidget::~ExifWidget()
{
}

QString ExifWidget::getMetadataTitle()
{
    return i18n("Standard EXIF Tags");
}

bool ExifWidget::loadFromURL(const QUrl& url)
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

bool ExifWidget::decodeMetadata()
{
    DMetadata data = getMetadata();

    if (!data.hasExif())
    {
        return false;
    }

    // Update all metadata contents.
    setMetadataMap(data.getExifTagsDataList(QStringList()));
    return true;
}

void ExifWidget::buildView()
{
    switch (getMode())
    {
        case CUSTOM:
            setIfdList(getMetadataMap(), m_keysFilter, getTagsFilter());
            break;

        case PHOTO:
            setIfdList(getMetadataMap(), m_keysFilter, QStringList() << QLatin1String("FULL"));
            break;

        default: // NONE
            setIfdList(getMetadataMap(), m_keysFilter, QStringList());
            break;
    }

    MetadataWidget::buildView();
}

QString ExifWidget::getTagTitle(const QString& key)
{
    DMetadata metadataIface;
    QString title = metadataIface.getExifTagTitle(key.toLatin1().constData());

    if (title.isEmpty())
    {
        return key.section(QLatin1Char('.'), -1);
    }

    return title;
}

QString ExifWidget::getTagDescription(const QString& key)
{
    DMetadata metadataIface;
    QString desc = metadataIface.getExifTagDescription(key.toLatin1().constData());

    if (desc.isEmpty())
    {
        return i18n("No description available");
    }

    return desc;
}

void ExifWidget::slotSaveMetadataToFile()
{
    QUrl url = saveMetadataToFile(i18n("EXIF File to Save"),
                                  QString(QLatin1String("*.exif|") + i18n("EXIF binary Files (*.exif)")));

    storeMetadataToFile(url, getMetadata().getExifEncoded());
}

}  // namespace Digikam
