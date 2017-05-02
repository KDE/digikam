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

#include "imagegenerationfunctor.h"

// Qt includes

#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QLocale>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "galleryinfo.h"
#include "generator.h"
#include "imageelement.h"
#include "metaengine_rotation.h"
#include "drawdecoder.h"
#include "rawinfo.h"

using namespace RawEngine;

namespace Digikam
{

/**
 * Generate a thumbnail from @fullImage of @size x @size pixels
 * If square == true, crop the result to a square
 */
static QImage generateThumbnail(const QImage& fullImage, int size, bool square)
{
    QImage image = fullImage.scaled(size, size, square ? Qt::KeepAspectRatioByExpanding
                                                       : Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation);

    if (square && (image.width() != size || image.height() != size))
    {
        int sx = 0;
        int sy = 0;

        if (image.width() > size)
        {
            sx = (image.width() - size) / 2;
        }
        else
        {
            sy = (image.height() - size) / 2;
        }

        image = image.copy(sx, sy, size, size);
    }

    return image;
}

ImageGenerationFunctor::ImageGenerationFunctor(Generator* const generator,
                                               GalleryInfo* const info,
                                               const QString& destDir)
    : mGenerator(generator),
      mInfo(info),
      mDestDir(destDir)
{
}

ImageGenerationFunctor::~ImageGenerationFunctor()
{
}

void ImageGenerationFunctor::operator()(ImageElement& element)
{
    // Load image
    QString path = element.mPath;
    QFile imageFile(path);

    if (!imageFile.open(QIODevice::ReadOnly))
    {
        emitWarning(i18n("Could not read image '%1'", path));
        return;
    }

    QImage     originalImage;
    QString    imageFormat;
    QByteArray imageData;

    // Check if RAW file.
    if (DRawDecoder::isRawFile(QUrl::fromLocalFile(path)))
    {
        if (!DRawDecoder::loadRawPreview(originalImage, path))
        {
            emitWarning(i18n("Error loading RAW image '%1'", path));
            return;
        }
    }
    else
    {
        imageFormat = QString::fromLatin1(QImageReader::imageFormat(&imageFile));

        if (imageFormat.isEmpty())
        {
            emitWarning(i18n("Format of image '%1' is unknown", path));
            return;
        }

        imageFile.close();
        imageFile.open(QIODevice::ReadOnly);

        imageData   = imageFile.readAll();

        if (!originalImage.loadFromData(imageData) )
        {
            emitWarning(i18n("Error loading image '%1'", path));
            return;
        }
    }

    // Process images
    QImage fullImage = originalImage;

    if (!mInfo->useOriginalImageAsFullImage())
    {
        if (mInfo->fullResize())
        {
            int size  = mInfo->fullSize();
            fullImage = fullImage.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        if (element.mOrientation != DMetadata::ORIENTATION_UNSPECIFIED )
        {
            QMatrix matrix = MetaEngineRotation::toMatrix(element.mOrientation);
            fullImage      = fullImage.transformed(matrix);
        }
    }

    QImage thumbnail     = generateThumbnail(fullImage, mInfo->thumbnailSize(), mInfo->thumbnailSquare());

    // Save images
    QString baseFileName = Generator::webifyFileName(element.mTitle);
    baseFileName         = mUniqueNameHelper.makeNameUnique(baseFileName);

    // Save full
    QString fullFileName;

    if (mInfo->useOriginalImageAsFullImage())
    {
        fullFileName = baseFileName + QLatin1Char('.') + imageFormat.toLower();

        if (!writeDataToFile(imageData, mDestDir + QLatin1Char('/') + fullFileName))
        {
            return;
        }
    }
    else
    {
        fullFileName     = baseFileName + QLatin1Char('.') + mInfo->fullFormatString().toLower();
        QString destPath = mDestDir + QLatin1Char('/') + fullFileName;

        if (!fullImage.save(destPath, mInfo->fullFormatString().toLatin1().data(), mInfo->fullQuality()))
        {
            emitWarning(i18n("Could not save image '%1' to '%2'", path, destPath));
            return;
        }
    }

    element.mFullFileName = fullFileName;
    element.mFullSize     = fullImage.size();

    // Save original
    if (mInfo->copyOriginalImage())
    {
        QString originalFileName = QLatin1String("original_") + fullFileName;

        if (!writeDataToFile(imageData, mDestDir + QLatin1Char('/') + originalFileName))
        {
            return;
        }

        element.mOriginalFileName = originalFileName;
        element.mOriginalSize = originalImage.size();
    }

    // Save thumbnail
    QString thumbnailFileName = QLatin1String("thumb_") + baseFileName + QLatin1Char('.') +
                                mInfo->thumbnailFormatString().toLower();
    QString destPath          = mDestDir + QLatin1Char('/') + thumbnailFileName;

    if (!thumbnail.save(destPath, mInfo->thumbnailFormatString().toLatin1().data(), mInfo->thumbnailQuality()))
    {
        mGenerator->logWarningRequested(i18n("Could not save thumbnail for image '%1' to '%2'", path, destPath));
        return;
    }

    element.mThumbnailFileName = thumbnailFileName;
    element.mThumbnailSize     = thumbnail.size();
    element.mValid             = true;

    // Read Exif Metadata
    QString unavailable(i18n("unavailable"));
    DMetadata meta;
    meta.load(path);

    if (meta.hasExif() || meta.hasXmp())
    {
        // Try to use image metadata to get image info

        element.mExifImageMake = meta.getExifTagString("Exif.Image.Make");

        if (element.mExifImageMake.isEmpty())
        {
            element.mExifImageMake = meta.getXmpTagString("Xmp.tiff.Make");
        }

        element.mExifImageModel = meta.getExifTagString("Exif.Image.Model");

        if (element.mExifImageModel.isEmpty())
        {
            element.mExifImageModel = meta.getXmpTagString("Xmp.tiff.Model");
        }

        element.mExifImageOrientation = meta.getExifTagString("Exif.Image.Orientation");

        if (element.mExifImageOrientation.isEmpty())
        {
            element.mExifImageOrientation = meta.getXmpTagString("Xmp.tiff.Orientation");
        }

        element.mExifImageXResolution = meta.getExifTagString("Exif.Image.XResolution");

        if (element.mExifImageXResolution.isEmpty())
        {
            element.mExifImageXResolution = meta.getXmpTagString("Xmp.tiff.XResolution");
        }

        element.mExifImageYResolution = meta.getExifTagString("Exif.Image.YResolution");

        if (element.mExifImageYResolution.isEmpty())
        {
            element.mExifImageYResolution = meta.getXmpTagString("Xmp.tiff.YResolution");
        }

        element.mExifImageResolutionUnit = meta.getExifTagString("Exif.Image.ResolutionUnit");

        if (element.mExifImageResolutionUnit.isEmpty())
        {
            element.mExifImageResolutionUnit = meta.getXmpTagString("Xmp.tiff.ResolutionUnit");
        }

        if (meta.getImageDateTime().isValid())
        {
            element.mExifImageDateTime = QLocale().toString(meta.getImageDateTime(), QLocale::ShortFormat);
        }

        element.mExifImageYCbCrPositioning = meta.getExifTagString("Exif.Image.YCbCrPositioning");

        if (element.mExifImageYCbCrPositioning.isEmpty())
        {
            element.mExifImageYCbCrPositioning = meta.getXmpTagString("Xmp.tiff.YCbCrPositioning");
        }

        element.mExifPhotoFNumber = meta.getExifTagString("Exif.Photo.FNumber");

        if (element.mExifPhotoFNumber.isEmpty())
        {
            element.mExifPhotoFNumber = meta.getXmpTagString("Xmp.exif.FNumber");
        }

        element.mExifPhotoApertureValue = meta.getExifTagString("Exif.Photo.ApertureValue");

        if (element.mExifPhotoApertureValue.isEmpty())
        {
            element.mExifPhotoApertureValue = meta.getXmpTagString("Xmp.exif.ApertureValue");
        }

        element.mExifPhotoFocalLength = meta.getExifTagString("Exif.Photo.FocalLength");

        if (element.mExifPhotoFocalLength.isEmpty())
        {
            element.mExifPhotoFocalLength = meta.getXmpTagString("Xmp.exif.FocalLength");
        }

        element.mExifPhotoExposureTime = meta.getExifTagString("Exif.Photo.ExposureTime");

        if (element.mExifPhotoExposureTime.isEmpty())
        {
            element.mExifPhotoExposureTime = meta.getXmpTagString("Xmp.exif.ExposureTime");
        }

        element.mExifPhotoShutterSpeedValue = meta.getExifTagString("Exif.Photo.ShutterSpeedValue");

        if (element.mExifPhotoShutterSpeedValue.isEmpty())
        {
            element.mExifPhotoShutterSpeedValue = meta.getXmpTagString("Xmp.exif.ShutterSpeedValue");
        }

        element.mExifPhotoISOSpeedRatings = meta.getExifTagString("Exif.Photo.ISOSpeedRatings");

        if (element.mExifPhotoISOSpeedRatings.isEmpty())
        {
            element.mExifPhotoISOSpeedRatings = meta.getXmpTagString("Xmp.exif.ISOSpeedRatings");
        }

        element.mExifPhotoExposureProgram = meta.getExifTagString("Exif.Photo.ExposureIndex");

        if (element.mExifPhotoExposureProgram.isEmpty())
        {
            element.mExifPhotoExposureProgram = meta.getXmpTagString("Xmp.exif.ExposureIndex");
        }

        // Get GPS values
        double gpsvalue;

        if (meta.getGPSAltitude(&gpsvalue))
        {
            element.mExifGPSAltitude = QString::number(gpsvalue, 'f', 3);
        }

        if (meta.getGPSLatitudeNumber(&gpsvalue))
        {
            element.mExifGPSLatitude = QString::number(gpsvalue, 'f', 6);
        }

        if (meta.getGPSLongitudeNumber(&gpsvalue))
        {
            element.mExifGPSLongitude = QString::number(gpsvalue, 'f', 6);
        }
    }
    else
    {
        // Try to use libkdcraw interface to identify image.

        RawInfo     info;
        DRawDecoder rawdecoder;
        rawdecoder.rawFileIdentify(info, path);

        if (info.isDecodable)
        {
            if (!info.make.isEmpty())
                element.mExifImageMake = info.make;

            if (!info.model.isEmpty())
                element.mExifImageModel = info.model;

            if (info.dateTime.isValid())
                element.mExifImageDateTime = QLocale().toString(info.dateTime, QLocale::ShortFormat);

            if (info.aperture != -1.0)
                element.mExifPhotoApertureValue = QString::number(info.aperture);

            if (info.focalLength != -1.0)
                element.mExifPhotoFocalLength = QString::number(info.focalLength);

            if (info.exposureTime != -1.0)
                element.mExifPhotoExposureTime = QString::number(info.exposureTime);

            if (info.sensitivity != -1)
                element.mExifPhotoISOSpeedRatings = QString::number(info.sensitivity);
        }
    }

    if (element.mExifImageMake.isEmpty())
        element.mExifImageMake = unavailable;

    if (element.mExifImageModel.isEmpty())
        element.mExifImageModel = unavailable;

    if (element.mExifImageOrientation.isEmpty())
        element.mExifImageOrientation = unavailable;

    if (element.mExifImageXResolution.isEmpty())
        element.mExifImageXResolution = unavailable;

    if (element.mExifImageYResolution.isEmpty())
        element.mExifImageYResolution = unavailable;

    if (element.mExifImageResolutionUnit.isEmpty())
        element.mExifImageResolutionUnit = unavailable;

    if (element.mExifImageDateTime.isEmpty())
        element.mExifImageDateTime = unavailable;

    if (element.mExifImageYCbCrPositioning.isEmpty())
        element.mExifImageYCbCrPositioning = unavailable;

    if (element.mExifPhotoApertureValue.isEmpty())
        element.mExifPhotoApertureValue = unavailable;

    if (element.mExifPhotoFocalLength.isEmpty())
        element.mExifPhotoFocalLength = unavailable;

    if (element.mExifPhotoFNumber.isEmpty())
        element.mExifPhotoFNumber = unavailable;

    if (element.mExifPhotoExposureTime.isEmpty())
        element.mExifPhotoExposureTime = unavailable;

    if (element.mExifPhotoShutterSpeedValue.isEmpty())
        element.mExifPhotoShutterSpeedValue = unavailable;

    if (element.mExifPhotoISOSpeedRatings.isEmpty())
        element.mExifPhotoISOSpeedRatings = unavailable;

    if (element.mExifPhotoExposureProgram.isEmpty())
        element.mExifPhotoExposureProgram = unavailable;

    if (element.mExifGPSAltitude.isEmpty())
        element.mExifGPSAltitude = unavailable;

    if (element.mExifGPSLatitude.isEmpty())
        element.mExifGPSLatitude = unavailable;

    if (element.mExifGPSLongitude.isEmpty())
        element.mExifGPSLongitude = unavailable;
}

bool ImageGenerationFunctor::writeDataToFile(const QByteArray& data, const QString& destPath)
{
    QFile destFile(destPath);

    if (!destFile.open(QIODevice::WriteOnly))
    {
        emitWarning(i18n("Could not open file '%1' for writing", destPath));
        return false;
    }

    if (destFile.write(data) != data.size())
    {
        emitWarning(i18n("Could not save image to file '%1'", destPath));
        return false;
    }

    return true;
}

void ImageGenerationFunctor::emitWarning(const QString& message)
{
    emit (mGenerator->logWarningRequested(message));
}

} // namespace Digikam
