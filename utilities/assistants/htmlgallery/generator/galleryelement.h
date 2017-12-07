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

#ifndef GALLERY_ELEMENT_H
#define GALLERY_ELEMENT_H

// Qt includes

#include <QSize>
#include <QString>

// Local includes

#include "dmetadata.h"
#include "dinfointerface.h"

namespace Digikam
{

class XMLWriter;

/**
 * This class stores all the necessary information to produce an XML
 * description of a gallery element.
 */
class GalleryElement
{
public:

    explicit GalleryElement(const DInfoInterface::DInfoMap& info);
    GalleryElement();
    ~GalleryElement();

    void appendToXML(XMLWriter& xmlWriter, bool copyOriginalImage) const;

    void appendImageElementToXML(XMLWriter& xmlWriter, const QString& elementName,
                                 const QString& fileName, const QSize& size) const;

public:

    bool                         m_valid;
    QString                      m_title;
    QString                      m_description;
    DMetadata::ImageOrientation  m_orientation;
    QDateTime                    m_time;

    QString                      m_path;

    QString                      m_thumbnailFileName;
    QSize                        m_thumbnailSize;
    QString                      m_fullFileName;
    QSize                        m_fullSize;
    QString                      m_originalFileName;
    QSize                        m_originalSize;

    // Exif Metadata
    QString                      m_exifImageMake;
    QString                      m_exifImageModel;
    QString                      m_exifImageOrientation;
    QString                      m_exifImageXResolution;
    QString                      m_exifImageYResolution;
    QString                      m_exifImageResolutionUnit;
    QString                      m_exifImageDateTime;
    QString                      m_exifImageYCbCrPositioning;
    QString                      m_exifPhotoExposureTime;
    QString                      m_exifPhotoFNumber;
    QString                      m_exifPhotoExposureProgram;
    QString                      m_exifPhotoISOSpeedRatings;
    QString                      m_exifPhotoShutterSpeedValue;
    QString                      m_exifPhotoApertureValue;
    QString                      m_exifPhotoFocalLength;

    // GPS Metadata
    QString                      m_exifGPSLatitude;
    QString                      m_exifGPSLongitude;
    QString                      m_exifGPSAltitude;
};

} // namespace Digikam

#endif // GALLERY_ELEMENT_H
