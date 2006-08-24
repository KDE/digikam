/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-20
 * Description : a widget to display non standard Exif metadata
 *               used by camera makers
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

#include <kdebug.h>
#include <klocale.h>

// LibExiv2 includes.

#include <exiv2/exif.hpp>
#include <exiv2/tags.hpp>
#include <exiv2/ifd.hpp>

// Local includes.

#include "dmetadata.h"
#include "makernotewidget.h"

namespace Digikam
{

// This list mix differents tags name used by camera makers.
static const char* MakerNoteHumanList[] =
{
     "AFFocusPos",
     "AFMode",
     "AFPoint",
     "AutofocusMode",
     "ColorMode",
     "ColorTemperature",
     "Contrast",
     "DigitalZoom",
     "ExposureMode",
     "ExposureProgram",
     "ExposureCompensation",
     "ExposureManualBias",
     "Flash",
     "FlashBias",
     "FlashMode",
     "FlashType",
     "FlashDevice",
     "FNumber", 
     "Focus"
     "FocusDistance",
     "FocusMode",
     "FocusSetting",
     "FocusType",
     "Hue",
     "HueAdjustment",
     "ImageStabilizer",
     "ImageStabilization",
     "InternalFlash", 
     "ISOSelection",
     "ISOSpeed",
     "Lens",
     "LensType",
     "LensRange",
     "Macro",
     "MacroFocus",
     "MeteringMode",
     "NoiseReduction",
     "OwnerName",
     "Quality",
     "Tone",
     "ToneComp",
     "Saturation",
     "Sharpness",
     "ShootingMode",
     "ShutterSpeedValue",
     "SpotMode",
     "SubjectDistance",
     "WhiteBalance",
     "WhiteBalanceBias",
     "-1"
};

static const char* ExifEntryListToIgnore[] =
{
     "GPSInfo",
     "Iop",
     "Thumbnail",
     "Image",
     "Photo",
     "-1"
};

MakerNoteWidget::MakerNoteWidget(QWidget* parent, const char* name)
               : MetadataWidget(parent, name)
{
    for (int i=0 ; QString(ExifEntryListToIgnore[i]) != QString("-1") ; i++)
        m_keysFilter << ExifEntryListToIgnore[i];

    for (int i=0 ; QString(MakerNoteHumanList[i]) != QString("-1") ; i++)
        m_tagsfilter << MakerNoteHumanList[i];
}

MakerNoteWidget::~MakerNoteWidget()
{
}

QString MakerNoteWidget::getMetadataTitle(void)
{
    return i18n("Maker Note Exif Tags");
}

bool MakerNoteWidget::loadFromURL(const KURL& url)
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

bool MakerNoteWidget::decodeMetadata()
{
    try
    {
        Exiv2::ExifData exifData;
        if (exifData.load((Exiv2::byte*)getMetadata().data(), getMetadata().size()) != 0)
            return false;

        exifData.sortByKey();
        
        QString ifDItemName;
        MetaDataMap metaDataMap;

        for (Exiv2::ExifData::iterator md = exifData.begin(); md != exifData.end(); ++md)
        {
            QString key = QString::fromLocal8Bit(md->key().c_str());
            
            // Decode the tag value with a user friendly output.
            std::ostringstream os;
            os << *md;
            QString value = QString::fromLocal8Bit(os.str().c_str());
            value.replace("\n", " ");

            // We apply a filter to get only standard Exif tags, not maker notes.
            if (!m_keysFilter.contains(key.section(".", 1, 1)))
                metaDataMap.insert(key, value);
        }
        
        // Update all metadata contents.
        setMetadataMap(metaDataMap);

        return true;
    }
    catch (Exiv2::Error& e)
    {
        kdDebug() << "Cannot parse MAKERNOTE metadata using Exiv2 ("
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
        return false;
    }
}

void MakerNoteWidget::buildView(void)
{
    if (getMode() == SIMPLE)
    {
        setIfdList(getMetadataMap(), m_tagsfilter);
    }
    else
    {
        setIfdList(getMetadataMap());
    }
}

QString MakerNoteWidget::getTagTitle(const QString& key)
{
    try 
    {
        std::string exifkey(key.ascii());
        Exiv2::ExifKey ek(exifkey); 
        return QString::fromLocal8Bit( Exiv2::ExifTags::tagTitle(ek.tag(), ek.ifdId()) );
    }
    catch (Exiv2::Error& e) 
    {
        kdDebug() << "Cannot get metadata tag title using Exiv2 ("
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
        return i18n("Unknow");
    }
}

QString MakerNoteWidget::getTagDescription(const QString& key)
{
    try 
    {
        std::string exifkey(key.ascii());
        Exiv2::ExifKey ek(exifkey); 
        return QString::fromLocal8Bit( Exiv2::ExifTags::tagDesc(ek.tag(), ek.ifdId()) );
    }
    catch (Exiv2::Error& e) 
    {
        kdDebug() << "Cannot get metadata tag description using Exiv2 ("
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
        return i18n("No description available");
    }
}

void MakerNoteWidget::slotSaveMetadataToFile(void)
{
    KURL url = saveMetadataToFile(i18n("Exif File to Save"),
                                  QString("*.dat|"+i18n("Exif binary Files (*.dat)")));
    storeMetadataToFile(url);
}

}  // namespace Digikam

#include "makernotewidget.moc"
