/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// Local includes

#include "galleryelement.h"
#include "galleryxmlutils.h"
#include "metaengine.h"

namespace Digikam
{

GalleryElement::GalleryElement(const DInfoInterface::DInfoMap& info)
    : m_valid(false)
{
    DItemInfo item(info);
    m_title       = item.name();
    m_description = item.comment();
    m_orientation = (MetaEngine::ImageOrientation)(item.orientation());
    m_time        = item.dateTime();
}

GalleryElement::GalleryElement()
    : m_valid(false),
      m_orientation(MetaEngine::ORIENTATION_UNSPECIFIED)
{
}

GalleryElement::~GalleryElement()
{
}

void GalleryElement::appendToXML(XMLWriter& xmlWriter, bool copyOriginalImage) const
{
    if (!m_valid)
    {
        return;
    }

    XMLElement imageX(xmlWriter, QLatin1String("image"));
    xmlWriter.writeElement("title",       m_title);
    xmlWriter.writeElement("description", m_description);
    xmlWriter.writeElement("date",        m_time.toString(QLatin1String("yyyy-MM-ddThh:mm:ss")));
    appendImageElementToXML(xmlWriter, QLatin1String("full"),      m_fullFileName, m_fullSize);
    appendImageElementToXML(xmlWriter, QLatin1String("thumbnail"), m_thumbnailFileName, m_thumbnailSize);

    if (copyOriginalImage)
    {
        appendImageElementToXML(xmlWriter,
                                QLatin1String("original"),
                                m_originalFileName, m_originalSize);
    }

    //Exif
    // TODO put all exif tags in a sub level
    XMLElement imageExif(xmlWriter, QLatin1String("exif"));
    xmlWriter.writeElement("exifimagemake",              m_exifImageMake);
    xmlWriter.writeElement("exifimagemodel",             m_exifImageModel);
    xmlWriter.writeElement("exifimageorientation",       m_exifImageOrientation);
    xmlWriter.writeElement("exifimagexresolution",       m_exifImageXResolution);
    xmlWriter.writeElement("exifimageyresolution",       m_exifImageYResolution);
    xmlWriter.writeElement("exifimageresolutionunit",    m_exifImageResolutionUnit);
    xmlWriter.writeElement("exifimagedatetime",          m_exifImageDateTime);
    xmlWriter.writeElement("exifimageycbcrpositioning",  m_exifImageYCbCrPositioning);
    xmlWriter.writeElement("exifphotoexposuretime",      m_exifPhotoExposureTime);
    xmlWriter.writeElement("exifphotofnumber",           m_exifPhotoFNumber);
    xmlWriter.writeElement("exifphotoexposureprogram",   m_exifPhotoExposureProgram);
    xmlWriter.writeElement("exifphotoisospeedratings",   m_exifPhotoISOSpeedRatings);
    xmlWriter.writeElement("exifphotoshutterspeedvalue", m_exifPhotoShutterSpeedValue);
    xmlWriter.writeElement("exifphotoaperturevalue",     m_exifPhotoApertureValue);
    xmlWriter.writeElement("exifphotofocallength",       m_exifPhotoFocalLength);

    // GPS
    xmlWriter.writeElement("exifgpslatitude",            m_exifGPSLatitude);
    xmlWriter.writeElement("exifgpslongitude",           m_exifGPSLongitude);
    xmlWriter.writeElement("exifgpsaltitude",            m_exifGPSAltitude);
}

void GalleryElement::appendImageElementToXML(XMLWriter& xmlWriter,
                                           const QString& elementName,
                                           const QString& fileName,
                                           const QSize& size) const
{
    XMLAttributeList attrList;
    attrList.append(QLatin1String("fileName"), fileName);
    attrList.append(QLatin1String("width"),    size.width());
    attrList.append(QLatin1String("height"),   size.height());
    XMLElement elem(xmlWriter, elementName, &attrList);
}

} // namespace Digikam
