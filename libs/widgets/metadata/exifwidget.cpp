/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-20
 * Description : a widget to display Standard Exif metadata
 * 
 * Copyright 2006 by Gilles Caulier
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

// C++ includes.

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <string>

// Qt includes.

#include <qmap.h>
#include <qfile.h>

// KDE includes.

#include <klocale.h>

// LibExiv2 includes.

#include <exiv2/exif.hpp>
#include <exiv2/tags.hpp>
#include <exiv2/ifd.hpp>

// Local includes.

#include "ddebug.h"
#include "dmetadata.h"
#include "metadatalistview.h"
#include "exifwidget.h"

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
// This will not including GPS informations because they are displayed on another tab.
static const char* StandardExifEntryList[] =
{
     "Iop",
     "Thumbnail",
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

QString ExifWidget::getMetadataTitle(void)
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
    try
    {
        Exiv2::ExifData exifData;
        if (exifData.load((Exiv2::byte*)getMetadata().data(), getMetadata().size()) != 0)
            return false;

        exifData.sortByKey();
        
        QString     ifDItemName;
        MetaDataMap metaDataMap;

        for (Exiv2::ExifData::iterator md = exifData.begin(); md != exifData.end(); ++md)
        {
            QString key = QString::fromAscii(md->key().c_str());

            // Decode the tag value with a user friendly output.
            QString tagValue;
            if (key == "Exif.Photo.UserComment")
            {
                tagValue = DMetadata::convertCommentValue(*md);
            }
            else
            {
                std::ostringstream os;
                os << *md;

                // Exif tag contents can be an i18n strings, no only simple ascii.
                tagValue = QString::fromLocal8Bit(os.str().c_str());
            }
            tagValue.replace("\n", " ");

            // We apply a filter to get only standard Exif tags, not maker notes.
            if (m_keysFilter.contains(key.section(".", 1, 1)))
                metaDataMap.insert(key, tagValue);
        }

        // Update all metadata contents.
        setMetadataMap(metaDataMap);

        return true;
    }
    catch (Exiv2::Error& e)
    {
        DDebug() << "Cannot parse EXIF metadata using Exiv2 ("
                  << QString::fromAscii(e.what().c_str())
                  << ")" << endl;
        return false;
    }
}

void ExifWidget::buildView(void)
{
    
    if (getMode() == SIMPLE)
    {
        setIfdList(getMetadataMap(), m_keysFilter, m_tagsfilter);
    }
    else
    {
        setIfdList(getMetadataMap(), m_keysFilter, QStringList());
    }
}

QString ExifWidget::getTagTitle(const QString& key)
{
    try 
    {
        std::string exifkey(key.ascii());
        Exiv2::ExifKey ek(exifkey); 
        return QString::fromLocal8Bit( Exiv2::ExifTags::tagTitle(ek.tag(), ek.ifdId()) );
    }
    catch (Exiv2::Error& e) 
    {
        DDebug() << "Cannot get metadata tag title using Exiv2 ("
                  << QString::fromAscii(e.what().c_str())
                  << ")" << endl;
        return i18n("Unknown");
    }
}

QString ExifWidget::getTagDescription(const QString& key)
{
    try 
    {
        std::string exifkey(key.ascii());
        Exiv2::ExifKey ek(exifkey); 
        return QString::fromLocal8Bit( Exiv2::ExifTags::tagDesc(ek.tag(), ek.ifdId()) );
    }
    catch (Exiv2::Error& e) 
    {
        DDebug() << "Cannot get metadata tag description using Exiv2 ("
                  << QString::fromAscii(e.what().c_str())
                  << ")" << endl;
        return i18n("No description available");
    }
}

void ExifWidget::slotSaveMetadataToFile(void)
{
    KURL url = saveMetadataToFile(i18n("EXIF File to Save"),
                                  QString("*.dat|"+i18n("EXIF binary Files (*.dat)")));
    storeMetadataToFile(url);
}

}  // namespace Digikam

#include "exifwidget.moc"
