/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-20
 * Description : a widget to display Standard Exif metadata
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
#include "metadatalistview.h"
#include "exifwidget.h"
#include "exifwidget.moc"

namespace Digikam
{

static const char* ExifHumanList[] =
{
     "Make",
     "Model",
     "DateTime",
     "ImageDescription",
     "Copyright",
     "ShutterSpeedValue",
     "ApertureValue",
     "ExposureProgram",
     "ExposureMode",
     "ExposureBiasValue",
     "ExposureTime",
     "WhiteBalance",
     "ISOSpeedRatings",
     "FocalLength",
     "SubjectDistance",
     "MeteringMode",
     "Contrast",
     "Saturation",
     "Sharpness",
     "LightSource",
     "Flash",
     "FNumber",
     "-1"
};

// Standard Exif Entry list from to less important to the most important for photograph.
// This will not including GPS information because they are displayed on another tab.
static const char* StandardExifEntryList[] =
{
     "Iop",
     "Thumbnail",
     "SubImage1",
     "SubImage2",
     "Image",
     "Photo",
     "-1"
};

ExifWidget::ExifWidget(QWidget* parent, const char* name)
          : MetadataWidget(parent, name)
{
    view()->setSortColumn(-1);
    
    for (int i=0 ; QString(StandardExifEntryList[i]) != QString("-1") ; i++)
        m_keysFilter << StandardExifEntryList[i];

    for (int i=0 ; QString(ExifHumanList[i]) != QString("-1") ; i++)
        m_tagsfilter << ExifHumanList[i];
}

ExifWidget::~ExifWidget()
{
}

QString ExifWidget::getMetadataTitle()
{
    return i18n("Standard EXIF Tags");
}

bool ExifWidget::loadFromURL(const KURL& url)
{
    setFileName(url.path());
    
    if (url.isEmpty())
    {
        setMetadata();
        return false;
    }
    else
    {    
        DMetadata metadata(url.path());
        QByteArray exifData = metadata.getExif();

        if (exifData.isEmpty())
        {
            setMetadata();
            return false;
        }
        else
            setMetadata(exifData);
    }

    return true;
}

bool ExifWidget::decodeMetadata()
{
    DMetadata metaData;
    if (!metaData.setExif(getMetadata()))
        return false;

    // Update all metadata contents.
    setMetadataMap(metaData.getExifTagsDataList(m_keysFilter));
    return true;
}

void ExifWidget::buildView()
{
    if (getMode() == SIMPLE)
    {
        setIfdList(getMetadataMap(), m_keysFilter, m_tagsfilter);
    }
    else
    {
        setIfdList(getMetadataMap(), m_keysFilter, QStringList());
    }

    MetadataWidget::buildView();
}

QString ExifWidget::getTagTitle(const QString& key)
{
    DMetadata meta;
    QString title = meta.getExifTagTitle(key.ascii());

    if (title.isEmpty())
        return key.section('.', -1);

    return title;
}

QString ExifWidget::getTagDescription(const QString& key)
{
    DMetadata meta;
    QString desc = meta.getExifTagDescription(key.ascii());

    if (desc.isEmpty())
        return i18n("No description available");

    return desc;
}

void ExifWidget::slotSaveMetadataToFile()
{
    KURL url = saveMetadataToFile(i18n("EXIF File to Save"),
                                  QString("*.exif|"+i18n("EXIF binary Files (*.exif)")));
    storeMetadataToFile(url);
}

}  // namespace Digikam
