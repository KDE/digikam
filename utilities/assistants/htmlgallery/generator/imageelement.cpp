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

#include "imageelement.h"

// Local includes

#include "xmlutils.h"

namespace Digikam
{

ImageElement::ImageElement(const DInfoInterface::DInfoMap& info)
    : mValid(false)
{
    DItemInfo item(info);
    mTitle       = item.name();
    mDescription = item.comment();
    mOrientation = (MetaEngine::ImageOrientation)(item.orientation());
    mTime        = item.dateTime();
}

ImageElement::ImageElement()
    : mValid(false)
{
}

ImageElement::~ImageElement()
{
}

void ImageElement::appendToXML(XMLWriter& xmlWriter, bool copyOriginalImage) const
{
    if (!mValid)
    {
        return;
    }

    XMLElement imageX(xmlWriter, QLatin1String("image"));
    xmlWriter.writeElement("title",       mTitle);
    xmlWriter.writeElement("description", mDescription);
    xmlWriter.writeElement("date",        mTime.toString(QLatin1String("yyyy-MM-ddThh:mm:ss")));
    appendImageElementToXML(xmlWriter, QLatin1String("full"),      mFullFileName, mFullSize);
    appendImageElementToXML(xmlWriter, QLatin1String("thumbnail"), mThumbnailFileName, mThumbnailSize);

    if (copyOriginalImage)
    {
        appendImageElementToXML(xmlWriter,
                                QLatin1String("original"),
                                mOriginalFileName, mOriginalSize);
    }

    //Exif
    // TODO put all exif tags in a sub level
    XMLElement imageExif(xmlWriter, QLatin1String("exif"));
    xmlWriter.writeElement("exifimagemake",              mExifImageMake);
    xmlWriter.writeElement("exifimagemodel",             mExifImageModel);
    xmlWriter.writeElement("exifimageorientation",       mExifImageOrientation);
    xmlWriter.writeElement("exifimagexresolution",       mExifImageXResolution);
    xmlWriter.writeElement("exifimageyresolution",       mExifImageYResolution);
    xmlWriter.writeElement("exifimageresolutionunit",    mExifImageResolutionUnit);
    xmlWriter.writeElement("exifimagedatetime",          mExifImageDateTime);
    xmlWriter.writeElement("exifimageycbcrpositioning",  mExifImageYCbCrPositioning);
    xmlWriter.writeElement("exifphotoexposuretime",      mExifPhotoExposureTime);
    xmlWriter.writeElement("exifphotofnumber",           mExifPhotoFNumber);
    xmlWriter.writeElement("exifphotoexposureprogram",   mExifPhotoExposureProgram);
    xmlWriter.writeElement("exifphotoisospeedratings",   mExifPhotoISOSpeedRatings);
    xmlWriter.writeElement("exifphotoshutterspeedvalue", mExifPhotoShutterSpeedValue);
    xmlWriter.writeElement("exifphotoaperturevalue",     mExifPhotoApertureValue);
    xmlWriter.writeElement("exifphotofocallength",       mExifPhotoFocalLength);

    // GPS
    xmlWriter.writeElement("exifgpslatitude",            mExifGPSLatitude);
    xmlWriter.writeElement("exifgpslongitude",           mExifGPSLongitude);
    xmlWriter.writeElement("exifgpsaltitude",            mExifGPSAltitude);
}

void ImageElement::appendImageElementToXML(XMLWriter& xmlWriter,
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
