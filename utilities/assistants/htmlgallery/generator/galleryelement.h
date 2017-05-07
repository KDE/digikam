/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

    bool                         mValid;
    QString                      mTitle;
    QString                      mDescription;
    DMetadata::ImageOrientation  mOrientation;
    QDateTime                    mTime;

    QString                      mPath;

    QString                      mThumbnailFileName;
    QSize                        mThumbnailSize;
    QString                      mFullFileName;
    QSize                        mFullSize;
    QString                      mOriginalFileName;
    QSize                        mOriginalSize;

    // Exif Metadata
    QString                      mExifImageMake;
    QString                      mExifImageModel;
    QString                      mExifImageOrientation;
    QString                      mExifImageXResolution;
    QString                      mExifImageYResolution;
    QString                      mExifImageResolutionUnit;
    QString                      mExifImageDateTime;
    QString                      mExifImageYCbCrPositioning;
    QString                      mExifPhotoExposureTime;
    QString                      mExifPhotoFNumber;
    QString                      mExifPhotoExposureProgram;
    QString                      mExifPhotoISOSpeedRatings;
    QString                      mExifPhotoShutterSpeedValue;
    QString                      mExifPhotoApertureValue;
    QString                      mExifPhotoFocalLength;

    // GPS Metadata
    QString                      mExifGPSLatitude;
    QString                      mExifGPSLongitude;
    QString                      mExifGPSAltitude;
};

} // namespace Digikam

#endif // GALLERY_ELEMENT_H
