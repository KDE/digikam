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

#include "galleryelementfunctor.h"

// Qt includes

#include <QDir>
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
#include "gallerygenerator.h"
#include "galleryelement.h"
#include "metaengine_rotation.h"
#include "drawdecoder.h"
#include "rawinfo.h"

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

GalleryElementFunctor::GalleryElementFunctor(GalleryGenerator* const generator,
                                             GalleryInfo* const info,
                                             const QString& destDir)
    : m_generator(generator),
      m_info(info),
      m_destDir(destDir)
{
}

GalleryElementFunctor::~GalleryElementFunctor()
{
}

void GalleryElementFunctor::operator()(GalleryElement& element)
{
    // Load image
    QString    path = element.m_path;
    QImage     originalImage;
    QString    imageFormat;
    QByteArray imageData;

    // Check if RAW file.
    if (DRawDecoder::isRawFile(QUrl::fromLocalFile(path)))
    {
        if (!DRawDecoder::loadRawPreview(originalImage, path))
        {
            emitWarning(i18n("Error loading RAW image '%1'", QDir::toNativeSeparators(path)));
            return;
        }
    }
    else
    {
        QFile imageFile(path);

        if (!imageFile.open(QIODevice::ReadOnly))
        {
            emitWarning(i18n("Could not read image '%1'", QDir::toNativeSeparators(path)));
            return;
        }

        imageFormat = QString::fromLatin1(QImageReader::imageFormat(&imageFile));

        if (imageFormat.isEmpty())
        {
            emitWarning(i18n("Format of image '%1' is unknown", QDir::toNativeSeparators(path)));
            return;
        }

        imageData = imageFile.readAll();

        if (!originalImage.loadFromData(imageData))
        {
            emitWarning(i18n("Error loading image '%1'", QDir::toNativeSeparators(path)));
            return;
        }
    }

    // Process images
    QImage fullImage = originalImage;

    if (!m_info->useOriginalImageAsFullImage())
    {
        if (m_info->fullResize())
        {
            int size  = m_info->fullSize();
            fullImage = fullImage.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        if (element.m_orientation != DMetadata::ORIENTATION_UNSPECIFIED )
        {
            QMatrix matrix = MetaEngineRotation::toMatrix(element.m_orientation);
            fullImage      = fullImage.transformed(matrix);
        }
    }

    QImage thumbnail     = generateThumbnail(fullImage, m_info->thumbnailSize(), m_info->thumbnailSquare());

    // Save images
    QString baseFileName = GalleryGenerator::webifyFileName(element.m_title);
    baseFileName         = m_uniqueNameHelper.makeNameUnique(baseFileName);

    // Save full
    QString fullFileName;

    if (m_info->useOriginalImageAsFullImage())
    {
        fullFileName = baseFileName + QLatin1Char('.') + imageFormat.toLower();

        if (!writeDataToFile(imageData, m_destDir + QLatin1Char('/') + fullFileName))
        {
            return;
        }
    }
    else
    {
        fullFileName     = baseFileName + QLatin1Char('.') + m_info->fullFormatString().toLower();
        QString destPath = m_destDir + QLatin1Char('/') + fullFileName;

        if (!fullImage.save(destPath, m_info->fullFormatString().toLatin1().data(), m_info->fullQuality()))
        {
            emitWarning(i18n("Could not save image '%1' to '%2'",
                             QDir::toNativeSeparators(path),
                             QDir::toNativeSeparators(destPath)));
            return;
        }
    }

    element.m_fullFileName = fullFileName;
    element.m_fullSize     = fullImage.size();

    // Save original
    if (m_info->copyOriginalImage())
    {
        QString originalFileName = QLatin1String("original_") + fullFileName;

        if (!writeDataToFile(imageData, m_destDir + QLatin1Char('/') + originalFileName))
        {
            return;
        }

        element.m_originalFileName = originalFileName;
        element.m_originalSize = originalImage.size();
    }

    // Save thumbnail
    QString thumbnailFileName = QLatin1String("thumb_") + baseFileName + QLatin1Char('.') +
                                m_info->thumbnailFormatString().toLower();
    QString destPath          = m_destDir + QLatin1Char('/') + thumbnailFileName;

    if (!thumbnail.save(destPath, m_info->thumbnailFormatString().toLatin1().data(), m_info->thumbnailQuality()))
    {
        m_generator->logWarningRequested(i18n("Could not save thumbnail for image '%1' to '%2'",
                                            QDir::toNativeSeparators(path),
                                            QDir::toNativeSeparators(destPath)));
        return;
    }

    element.m_thumbnailFileName = thumbnailFileName;
    element.m_thumbnailSize     = thumbnail.size();
    element.m_valid             = true;

    // Read Exif Metadata
    QString unavailable(i18n("unavailable"));
    DMetadata meta;
    meta.load(path);

    if (meta.hasExif() || meta.hasXmp())
    {
        // Try to use image metadata to get image info

        element.m_exifImageMake = meta.getExifTagString("Exif.Image.Make");

        if (element.m_exifImageMake.isEmpty())
        {
            element.m_exifImageMake = meta.getXmpTagString("Xmp.tiff.Make");
        }

        element.m_exifImageModel = meta.getExifTagString("Exif.Image.Model");

        if (element.m_exifImageModel.isEmpty())
        {
            element.m_exifImageModel = meta.getXmpTagString("Xmp.tiff.Model");
        }

        element.m_exifImageOrientation = meta.getExifTagString("Exif.Image.Orientation");

        if (element.m_exifImageOrientation.isEmpty())
        {
            element.m_exifImageOrientation = meta.getXmpTagString("Xmp.tiff.Orientation");
        }

        element.m_exifImageXResolution = meta.getExifTagString("Exif.Image.XResolution");

        if (element.m_exifImageXResolution.isEmpty())
        {
            element.m_exifImageXResolution = meta.getXmpTagString("Xmp.tiff.XResolution");
        }

        element.m_exifImageYResolution = meta.getExifTagString("Exif.Image.YResolution");

        if (element.m_exifImageYResolution.isEmpty())
        {
            element.m_exifImageYResolution = meta.getXmpTagString("Xmp.tiff.YResolution");
        }

        element.m_exifImageResolutionUnit = meta.getExifTagString("Exif.Image.ResolutionUnit");

        if (element.m_exifImageResolutionUnit.isEmpty())
        {
            element.m_exifImageResolutionUnit = meta.getXmpTagString("Xmp.tiff.ResolutionUnit");
        }

        if (meta.getImageDateTime().isValid())
        {
            element.m_exifImageDateTime = QLocale().toString(meta.getImageDateTime(), QLocale::ShortFormat);
        }

        element.m_exifImageYCbCrPositioning = meta.getExifTagString("Exif.Image.YCbCrPositioning");

        if (element.m_exifImageYCbCrPositioning.isEmpty())
        {
            element.m_exifImageYCbCrPositioning = meta.getXmpTagString("Xmp.tiff.YCbCrPositioning");
        }

        element.m_exifPhotoFNumber = meta.getExifTagString("Exif.Photo.FNumber");

        if (element.m_exifPhotoFNumber.isEmpty())
        {
            element.m_exifPhotoFNumber = meta.getXmpTagString("Xmp.exif.FNumber");
        }

        element.m_exifPhotoApertureValue = meta.getExifTagString("Exif.Photo.ApertureValue");

        if (element.m_exifPhotoApertureValue.isEmpty())
        {
            element.m_exifPhotoApertureValue = meta.getXmpTagString("Xmp.exif.ApertureValue");
        }

        element.m_exifPhotoFocalLength = meta.getExifTagString("Exif.Photo.FocalLength");

        if (element.m_exifPhotoFocalLength.isEmpty())
        {
            element.m_exifPhotoFocalLength = meta.getXmpTagString("Xmp.exif.FocalLength");
        }

        element.m_exifPhotoExposureTime = meta.getExifTagString("Exif.Photo.ExposureTime");

        if (element.m_exifPhotoExposureTime.isEmpty())
        {
            element.m_exifPhotoExposureTime = meta.getXmpTagString("Xmp.exif.ExposureTime");
        }

        element.m_exifPhotoShutterSpeedValue = meta.getExifTagString("Exif.Photo.ShutterSpeedValue");

        if (element.m_exifPhotoShutterSpeedValue.isEmpty())
        {
            element.m_exifPhotoShutterSpeedValue = meta.getXmpTagString("Xmp.exif.ShutterSpeedValue");
        }

        element.m_exifPhotoISOSpeedRatings = meta.getExifTagString("Exif.Photo.ISOSpeedRatings");

        if (element.m_exifPhotoISOSpeedRatings.isEmpty())
        {
            element.m_exifPhotoISOSpeedRatings = meta.getXmpTagString("Xmp.exif.ISOSpeedRatings");
        }

        element.m_exifPhotoExposureProgram = meta.getExifTagString("Exif.Photo.ExposureIndex");

        if (element.m_exifPhotoExposureProgram.isEmpty())
        {
            element.m_exifPhotoExposureProgram = meta.getXmpTagString("Xmp.exif.ExposureIndex");
        }

        // Get GPS values
        double gpsvalue;

        if (meta.getGPSAltitude(&gpsvalue))
        {
            element.m_exifGPSAltitude = QString::number(gpsvalue, 'f', 3);
        }

        if (meta.getGPSLatitudeNumber(&gpsvalue))
        {
            element.m_exifGPSLatitude = QString::number(gpsvalue, 'f', 6);
        }

        if (meta.getGPSLongitudeNumber(&gpsvalue))
        {
            element.m_exifGPSLongitude = QString::number(gpsvalue, 'f', 6);
        }
    }
    else
    {
        // Try to use Raw decoder to identify image.

        RawInfo     info;
        DRawDecoder rawdecoder;
        rawdecoder.rawFileIdentify(info, path);

        if (info.isDecodable)
        {
            if (!info.make.isEmpty())
                element.m_exifImageMake = info.make;

            if (!info.model.isEmpty())
                element.m_exifImageModel = info.model;

            if (info.dateTime.isValid())
                element.m_exifImageDateTime = QLocale().toString(info.dateTime, QLocale::ShortFormat);

            if (info.aperture != -1.0)
                element.m_exifPhotoApertureValue = QString::number(info.aperture);

            if (info.focalLength != -1.0)
                element.m_exifPhotoFocalLength = QString::number(info.focalLength);

            if (info.exposureTime != -1.0)
                element.m_exifPhotoExposureTime = QString::number(info.exposureTime);

            if (info.sensitivity != -1)
                element.m_exifPhotoISOSpeedRatings = QString::number(info.sensitivity);
        }
    }

    if (element.m_exifImageMake.isEmpty())
        element.m_exifImageMake = unavailable;

    if (element.m_exifImageModel.isEmpty())
        element.m_exifImageModel = unavailable;

    if (element.m_exifImageOrientation.isEmpty())
        element.m_exifImageOrientation = unavailable;

    if (element.m_exifImageXResolution.isEmpty())
        element.m_exifImageXResolution = unavailable;

    if (element.m_exifImageYResolution.isEmpty())
        element.m_exifImageYResolution = unavailable;

    if (element.m_exifImageResolutionUnit.isEmpty())
        element.m_exifImageResolutionUnit = unavailable;

    if (element.m_exifImageDateTime.isEmpty())
        element.m_exifImageDateTime = unavailable;

    if (element.m_exifImageYCbCrPositioning.isEmpty())
        element.m_exifImageYCbCrPositioning = unavailable;

    if (element.m_exifPhotoApertureValue.isEmpty())
        element.m_exifPhotoApertureValue = unavailable;

    if (element.m_exifPhotoFocalLength.isEmpty())
        element.m_exifPhotoFocalLength = unavailable;

    if (element.m_exifPhotoFNumber.isEmpty())
        element.m_exifPhotoFNumber = unavailable;

    if (element.m_exifPhotoExposureTime.isEmpty())
        element.m_exifPhotoExposureTime = unavailable;

    if (element.m_exifPhotoShutterSpeedValue.isEmpty())
        element.m_exifPhotoShutterSpeedValue = unavailable;

    if (element.m_exifPhotoISOSpeedRatings.isEmpty())
        element.m_exifPhotoISOSpeedRatings = unavailable;

    if (element.m_exifPhotoExposureProgram.isEmpty())
        element.m_exifPhotoExposureProgram = unavailable;

    if (element.m_exifGPSAltitude.isEmpty())
        element.m_exifGPSAltitude = unavailable;

    if (element.m_exifGPSLatitude.isEmpty())
        element.m_exifGPSLatitude = unavailable;

    if (element.m_exifGPSLongitude.isEmpty())
        element.m_exifGPSLongitude = unavailable;
}

bool GalleryElementFunctor::writeDataToFile(const QByteArray& data, const QString& destPath)
{
    QFile destFile(destPath);

    if (!destFile.open(QIODevice::WriteOnly))
    {
        emitWarning(i18n("Could not open file '%1' for writing", QDir::toNativeSeparators(destPath)));
        return false;
    }

    if (destFile.write(data) != data.size())
    {
        emitWarning(i18n("Could not save image to file '%1'", QDir::toNativeSeparators(destPath)));
        return false;
    }

    return true;
}

void GalleryElementFunctor::emitWarning(const QString& message)
{
    emit (m_generator->logWarningRequested(message));
}

} // namespace Digikam
