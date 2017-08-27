/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-09-25
 * Description : a tool to convert RAW file to DNG
 *
 * Copyright (C) 2008-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010-2011 by Jens Mueller <tschenser at gmx dot de>
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

/**
    References about DNG:
    DNG SDK tutorial:    http://www.adobeforums.com/webx/.3bc2944e
                         http://www.adobeforums.com/webx/.3c054bde
    DNG review:          http://www.barrypearson.co.uk/articles/dng/index.htm
    DNG intro:           http://www.adobe.com/digitalimag/pdfs/dng_primer.pdf
                         http://www.adobe.com/products/dng/pdfs/DNG_primer_manufacturers.pdf
    DNG Specification:   http://www.adobe.com/products/dng/pdfs/dng_spec_1_2_0_0.pdf
    TIFF/EP Spec.:       http://www.map.tu.chiba-u.ac.jp/IEC/100/TA2/recdoc/N4378.pdf
    DNG SDK reference:   http://www.thomasdideriksen.dk/misc/File%20Formats/dng_sdk_refman.pdf
    DNG SDK tarball:     ftp://ftp.adobe.com/pub/adobe/dng/dng_sdk_1_2.zip
    DNG users forum:     http://www.adobeforums.com/webx/.3bb5f0ec

    Applications using DNG SDK:
    DNG4PS2:             http://dng4ps2.chat.ru/index_en.html
    CORNERFIX:           http://sourceforge.net/projects/cornerfix
    ADOBE DNG CONVERTER: ftp://ftp.adobe.com/pub/adobe/photoshop/cameraraw/win/4.x
    DNGCONVERT:          https://github.com/jmue/dngconvert
    MOVIE2DNG:           https://elphel.svn.sourceforge.net/svnroot/elphel/tools/Movie2DNG
*/

#include "dngwriter_p.h"

// C ansi includes

extern "C"
{
#include <sys/stat.h>
#include <utime.h>
}

// Qt includes

#include <QImage>
#include <QString>
#include <QByteArray>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QTemporaryFile>

// LibDRawDecoder includes



// Local includes

#include "digikam_debug.h"
#include "dngwriterhost.h"
#include "dmetadata.h"

#define CHUNK 65536

namespace Digikam
{

DNGWriter::DNGWriter()
    : d(new Private)
{
}

DNGWriter::~DNGWriter()
{
    delete d;
}

void DNGWriter::cancel()
{
    d->cancel = true;
}

void DNGWriter::reset()
{
    d->reset();
}

void DNGWriter::setCompressLossLess(bool b)
{
    d->jpegLossLessCompression = b;
}

bool DNGWriter::compressLossLess() const
{
    return d->jpegLossLessCompression;
}

void DNGWriter::setUpdateFileDate(bool b)
{
    d->updateFileDate = b;
}

bool DNGWriter::updateFileDate() const
{
    return d->updateFileDate;
}

void DNGWriter::setBackupOriginalRawFile(bool b)
{
    d->backupOriginalRawFile = b;
}

bool DNGWriter::backupOriginalRawFile() const
{
    return d->backupOriginalRawFile;
}

void DNGWriter::setPreviewMode(int mode)
{
    d->previewMode = mode;
}

int DNGWriter::previewMode() const
{
    return d->previewMode;
}

void DNGWriter::setInputFile(const QString& filePath)
{
    d->inputFile = filePath;
}

void DNGWriter::setOutputFile(const QString& filePath)
{
    d->outputFile = filePath;
}

QString DNGWriter::inputFile() const
{
    return d->inputFile;
}

QString DNGWriter::outputFile() const
{
    return d->outputFile;
}

// FIXME: Wow, this is a LOOOOOOOONG method... We should try to refactor it.
int DNGWriter::convert()
{
    d->cancel = false;

    try
    {
        if (inputFile().isEmpty())
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: No input file to convert. Aborted..." ;
            return PROCESSFAILED;
        }

        QFileInfo inputInfo(inputFile());
        QString   dngFilePath = outputFile();

        if (dngFilePath.isEmpty())
        {
            dngFilePath = QString(inputInfo.completeBaseName() + QLatin1String(".dng"));
        }

        QFileInfo          outputInfo(dngFilePath);
        QByteArray         rawData;
        RawInfo identify;
        RawInfo identifyMake;

        // -----------------------------------------------------------------------------------------

        qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Loading RAW data from " << inputInfo.fileName() ;

        DRawDecoder rawProcessor;

        if (!rawProcessor.rawFileIdentify(identifyMake, inputFile()))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Reading RAW file failed. Aborted..." ;
            return PROCESSFAILED;
        }

        dng_rect activeArea;
        // TODO: need to get correct default crop size to avoid artifacts at the borders
        int activeWidth  = 0;
        int activeHeight = 0;
        int outputHeight = 0;
        int outputWidth  = 0;

        if ((identifyMake.orientation == 5) || (identifyMake.orientation == 6))
        {
            outputHeight = identifyMake.outputSize.width();
            outputWidth  = identifyMake.outputSize.height();
        }
        else
        {
            outputHeight = identifyMake.outputSize.height();
            outputWidth  = identifyMake.outputSize.width();
        }

        if (!rawProcessor.extractRAWData(inputFile(), rawData, identify, 0))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Loading RAW data failed. Aborted..." ;
            return FILENOTSUPPORTED;
        }

        if (d->cancel)
            return PROCESSCANCELED;

        qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Raw data loaded:" ;
        qCDebug(DIGIKAM_GENERAL_LOG) << "--- Data Size:     " << rawData.size() << " bytes";
        qCDebug(DIGIKAM_GENERAL_LOG) << "--- Date:          " << identify.dateTime.toString(Qt::ISODate);
        qCDebug(DIGIKAM_GENERAL_LOG) << "--- Make:          " << identify.make;
        qCDebug(DIGIKAM_GENERAL_LOG) << "--- Model:         " << identify.model;
        qCDebug(DIGIKAM_GENERAL_LOG) << "--- ImageSize:     " << identify.imageSize.width() << "x" << identify.imageSize.height();
        qCDebug(DIGIKAM_GENERAL_LOG) << "--- FullSize:      " << identify.fullSize.width() << "x" << identify.fullSize.height();
        qCDebug(DIGIKAM_GENERAL_LOG) << "--- OutputSize:    " << identify.outputSize.width() << "x" << identify.outputSize.height();
        qCDebug(DIGIKAM_GENERAL_LOG) << "--- Orientation:   " << identify.orientation;
        qCDebug(DIGIKAM_GENERAL_LOG) << "--- Top margin:    " << identify.topMargin;
        qCDebug(DIGIKAM_GENERAL_LOG) << "--- Left margin:   " << identify.leftMargin;
        qCDebug(DIGIKAM_GENERAL_LOG) << "--- Filter:        " << identify.filterPattern;
        qCDebug(DIGIKAM_GENERAL_LOG) << "--- Colors:        " << identify.rawColors;
        qCDebug(DIGIKAM_GENERAL_LOG) << "--- Black:         " << identify.blackPoint;
        qCDebug(DIGIKAM_GENERAL_LOG) << "--- White:         " << identify.whitePoint;
        qCDebug(DIGIKAM_GENERAL_LOG) << "--- CAM->XYZ:" ;

        QString matrixVal;

        for(int i=0; i<4; ++i)
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "                   "
                     << QString().sprintf("%03.4f  %03.4f  %03.4f", identify.cameraXYZMatrix[i][0],
                                                                    identify.cameraXYZMatrix[i][1],
                                                                    identify.cameraXYZMatrix[i][2]);
        }

        // Check if CFA layout is supported by DNG SDK.
        Private::DNGBayerPattern bayerPattern = Private::Unknown;
        uint32 filter                         = 0;
        bool fujiRotate90                     = false;

        // Standard bayer layouts
        if (identify.filterPattern == QLatin1String("GRBGGRBGGRBGGRBG"))
        {
            bayerPattern = Private::Standard;
            filter       = 0;
        }
        else if (identify.filterPattern == QLatin1String("RGGBRGGBRGGBRGGB"))
        {
            bayerPattern = Private::Standard;
            filter       = 1;
        }
        else if (identify.filterPattern == QLatin1String("BGGRBGGRBGGRBGGR"))
        {
            bayerPattern = Private::Standard;
            filter       = 2;
        }
        else if (identify.filterPattern == QLatin1String("GBRGGBRGGBRGGBRG"))
        {
            bayerPattern = Private::Standard;
            filter       = 3;
        }
        else if ((identify.filterPattern == QLatin1String("RGBGRGBGRGBGRGBG")) && (identifyMake.make == QLatin1String("FUJIFILM")))
        {
            // Fuji layouts
            bayerPattern = Private::Fuji;
            fujiRotate90 = false;
            filter       = 0;
        }
        else if ((identify.filterPattern == QLatin1String("RBGGBRGGRBGGBRGG")) && (identifyMake.make == QLatin1String("FUJIFILM")))
        {
            // Fuji layouts
            bayerPattern = Private::Fuji;
            fujiRotate90 = true;
            filter       = 0;
        }
        else if ((identify.rawColors == 3)                 &&
                 (identify.filterPattern.isEmpty())        &&
                 //(identify.filterPattern == QString("")) &&
                 ((uint32)rawData.size() == identify.outputSize.width() * identify.outputSize.height() * 3 * sizeof(uint16)))
        {
            bayerPattern = Private::LinearRaw;
        }
        else if (identify.rawColors == 4)           // Four color sensors
        {
            bayerPattern = Private::FourColor;

            if (identify.filterPattern.length() != 16)
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Bayer mosaic not supported. Aborted..." ;
                return FILENOTSUPPORTED;
            }

            for (int i = 0; i < 16; ++i)
            {
                filter = filter >> 2;

                if (identify.filterPattern[i] == QLatin1Char('G'))
                {
                    filter |= 0x00000000;
                }
                else if (identify.filterPattern[i] == QLatin1Char('M'))
                {
                    filter |= 0x40000000;
                }
                else if (identify.filterPattern[i] == QLatin1Char('C'))
                {
                    filter |= 0x80000000;
                }
                else if (identify.filterPattern[i] == QLatin1Char('Y'))
                {
                    filter |= 0xC0000000;
                }
                else
                {
                    qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Bayer mosaic not supported. Aborted..." ;
                    return FILENOTSUPPORTED;
                }
            }
        }
        else
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Bayer mosaic not supported. Aborted..." ;
            return FILENOTSUPPORTED;
        }

        if (fujiRotate90)
        {
            if (!d->fujiRotate(rawData, identify))
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Can not rotate fuji image. Aborted...";
                return PROCESSFAILED;
            }

            int tmp      = outputWidth;
            outputWidth  = outputHeight;
            outputHeight = tmp;
        }

        activeArea   = dng_rect(identify.outputSize.height(), identify.outputSize.width());
        activeWidth  = identify.outputSize.width();
        activeHeight = identify.outputSize.height();

        // Check if number of Raw Color components is supported.
        if ((identify.rawColors != 3) && (identify.rawColors != 4))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Number of Raw color components not supported. Aborted..." ;
            return PROCESSFAILED;
        }

        int width  = identify.outputSize.width();
        int height = identify.outputSize.height();

/*      // NOTE: code to hack RAW data extraction

        QString   rawdataFilePath(inputInfo.completeBaseName() + QString(".dat"));
        QFileInfo rawdataInfo(rawdataFilePath);

        QFile rawdataFile(rawdataFilePath);
        if (!rawdataFile.open(QIODevice::WriteOnly))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Cannot open file to write RAW data. Aborted..." ;
            return PROCESSFAILED;
        }
        QDataStream rawdataStream(&rawdataFile);
        rawdataStream.writeRawData(rawData.data(), rawData.size());
        rawdataFile.close();
*/
        // -----------------------------------------------------------------------------------------

        if (d->cancel)
            return PROCESSCANCELED;

        qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: DNG memory allocation and initialization" ;

        dng_memory_allocator memalloc(gDefaultDNGMemoryAllocator);

        dng_rect rect(height, width);
        DNGWriterHost host(d, &memalloc);

        host.SetSaveDNGVersion(dngVersion_SaveDefault);
        host.SetSaveLinearDNG(false);
        host.SetKeepOriginalFile(true);

        AutoPtr<dng_image> image(new dng_simple_image(rect, (bayerPattern == Private::LinearRaw) ? 3 : 1, ttShort, memalloc));

        if (d->cancel)
            return PROCESSCANCELED;

        // -----------------------------------------------------------------------------------------

        dng_pixel_buffer buffer;

        buffer.fArea       = rect;
        buffer.fPlane      = 0;
        buffer.fPlanes     = bayerPattern == Private::LinearRaw ? 3 : 1;
        buffer.fRowStep    = buffer.fPlanes * width;
        buffer.fColStep    = buffer.fPlanes;
        buffer.fPlaneStep  = 1;
        buffer.fPixelType  = ttShort;
        buffer.fPixelSize  = TagTypeSize(ttShort);
        buffer.fData       = rawData.data();
        image->Put(buffer);

        if (d->cancel)
            return PROCESSCANCELED;

        // -----------------------------------------------------------------------------------------

        qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: DNG Negative structure creation" ;

        AutoPtr<dng_negative> negative(host.Make_dng_negative());

        negative->SetDefaultScale(dng_urational(outputWidth, activeWidth), dng_urational(outputHeight, activeHeight));

        if (bayerPattern != Private::LinearRaw)
        {
            negative->SetDefaultCropOrigin(8, 8);
            negative->SetDefaultCropSize(activeWidth - 16, activeHeight - 16);
        }
        else
        {
            negative->SetDefaultCropOrigin(0, 0);
            negative->SetDefaultCropSize(activeWidth, activeHeight);
        }

        negative->SetActiveArea(activeArea);
        negative->SetModelName(identify.model.toLatin1().constData());
        negative->SetLocalName(QString::fromUtf8("%1 %2").arg(identify.make, identify.model).toLatin1().constData());
        negative->SetOriginalRawFileName(inputInfo.fileName().toLatin1().constData());
        negative->SetColorChannels(identify.rawColors);

        ColorKeyCode colorCodes[4] = { colorKeyMaxEnum, colorKeyMaxEnum, colorKeyMaxEnum, colorKeyMaxEnum };

        for(int i = 0; i < qMax(4, identify.colorKeys.length()); ++i)
        {
            if (identify.colorKeys[i] == QLatin1Char('R'))
            {
                colorCodes[i] = colorKeyRed;
            }
            else if (identify.colorKeys[i] == QLatin1Char('G'))
            {
                colorCodes[i] = colorKeyGreen;
            }
            else if (identify.colorKeys[i] == QLatin1Char('B'))
            {
                colorCodes[i] = colorKeyBlue;
            }
            else if (identify.colorKeys[i] == QLatin1Char('C'))
            {
                colorCodes[i] = colorKeyCyan;
            }
            else if (identify.colorKeys[i] == QLatin1Char('M'))
            {
                colorCodes[i] = colorKeyMagenta;
            }
            else if (identify.colorKeys[i] == QLatin1Char('Y'))
            {
                colorCodes[i] = colorKeyYellow;
            }
        }

        negative->SetColorKeys(colorCodes[0], colorCodes[1], colorCodes[2], colorCodes[3]);

        switch (bayerPattern)
        {
            case Private::Standard:
                // Standard bayer mosaicing. All work fine there.
                // Bayer CCD mask: http://en.wikipedia.org/wiki/Bayer_filter
                negative->SetBayerMosaic(filter);
                break;
            case Private::Fuji:
                // TODO: Fuji is special case. Need to setup different bayer rules here.
                // It do not work in all settings. Need indeep investiguations.
                // Fuji superCCD: http://en.wikipedia.org/wiki/Super_CCD
                negative->SetFujiMosaic(filter);
                break;
            case Private::FourColor:
                negative->SetQuadMosaic(filter);
                break;
            default:
                break;
        }

        negative->SetWhiteLevel(identify.whitePoint, 0);
        negative->SetWhiteLevel(identify.whitePoint, 1);
        negative->SetWhiteLevel(identify.whitePoint, 2);
        negative->SetWhiteLevel(identify.whitePoint, 3);

        const dng_mosaic_info* const mosaicinfo = negative->GetMosaicInfo();

        if ((mosaicinfo != NULL) && (mosaicinfo->fCFAPatternSize == dng_point(2, 2)))
        {
            negative->SetQuadBlacks(identify.blackPoint + identify.blackPointCh[0],
                                    identify.blackPoint + identify.blackPointCh[1],
                                    identify.blackPoint + identify.blackPointCh[2],
                                    identify.blackPoint + identify.blackPointCh[3]);
        }
        else
        {
            negative->SetBlackLevel(identify.blackPoint, 0);
        }

        negative->SetBaselineExposure(0.0);
        negative->SetBaselineNoise(1.0);
        negative->SetBaselineSharpness(1.0);

        dng_orientation orientation;

        switch (identify.orientation)
        {
            case RawInfo::ORIENTATION_180:
                orientation = dng_orientation::Rotate180();
                break;

            case RawInfo::ORIENTATION_Mirror90CCW:
                orientation = dng_orientation::Mirror90CCW();
                break;

            case RawInfo::ORIENTATION_90CCW:
                orientation = dng_orientation::Rotate90CCW();
                break;

            case RawInfo::ORIENTATION_90CW:
                orientation = dng_orientation::Rotate90CW();
                break;

            default:   // ORIENTATION_NONE
                orientation = dng_orientation::Normal();
                break;
        }

        negative->SetBaseOrientation(orientation);
        negative->SetAntiAliasStrength(dng_urational(100, 100));
        negative->SetLinearResponseLimit(1.0);
        negative->SetShadowScale( dng_urational(1, 1) );
        negative->SetAnalogBalance(dng_vector_3(1.0, 1.0, 1.0));

        // -------------------------------------------------------------------------------

        AutoPtr<dng_camera_profile> prof(new dng_camera_profile);
        prof->SetName(QString::fromUtf8("%1 %2").arg(identify.make, identify.model).toLatin1().constData());

        // Set Camera->XYZ Color matrix as profile.
        dng_matrix matrix;

        switch (identify.rawColors)
        {
            case 3:
                {
                    dng_matrix_3by3 camXYZ;
                    camXYZ[0][0] = identify.cameraXYZMatrix[0][0];
                    camXYZ[0][1] = identify.cameraXYZMatrix[0][1];
                    camXYZ[0][2] = identify.cameraXYZMatrix[0][2];
                    camXYZ[1][0] = identify.cameraXYZMatrix[1][0];
                    camXYZ[1][1] = identify.cameraXYZMatrix[1][1];
                    camXYZ[1][2] = identify.cameraXYZMatrix[1][2];
                    camXYZ[2][0] = identify.cameraXYZMatrix[2][0];
                    camXYZ[2][1] = identify.cameraXYZMatrix[2][1];
                    camXYZ[2][2] = identify.cameraXYZMatrix[2][2];

                    if (camXYZ.MaxEntry() == 0.0)
                    {
                        qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: camera XYZ Matrix is null : camera not supported" ;
                        return FILENOTSUPPORTED;
                    }

                    matrix = camXYZ;

                    break;
                }
            case 4:
                {
                    dng_matrix_4by3 camXYZ;
                    camXYZ[0][0] = identify.cameraXYZMatrix[0][0];
                    camXYZ[0][1] = identify.cameraXYZMatrix[0][1];
                    camXYZ[0][2] = identify.cameraXYZMatrix[0][2];
                    camXYZ[1][0] = identify.cameraXYZMatrix[1][0];
                    camXYZ[1][1] = identify.cameraXYZMatrix[1][1];
                    camXYZ[1][2] = identify.cameraXYZMatrix[1][2];
                    camXYZ[2][0] = identify.cameraXYZMatrix[2][0];
                    camXYZ[2][1] = identify.cameraXYZMatrix[2][1];
                    camXYZ[2][2] = identify.cameraXYZMatrix[2][2];
                    camXYZ[3][0] = identify.cameraXYZMatrix[3][0];
                    camXYZ[3][1] = identify.cameraXYZMatrix[3][1];
                    camXYZ[3][2] = identify.cameraXYZMatrix[3][2];

                    if (camXYZ.MaxEntry() == 0.0)
                    {
                        qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: camera XYZ Matrix is null : camera not supported" ;
                        return FILENOTSUPPORTED;
                    }

                    matrix = camXYZ;

                    break;
                }
        }

        prof->SetColorMatrix1((dng_matrix) matrix);
        prof->SetCalibrationIlluminant1(lsD65);
        negative->AddProfile(prof);

        dng_vector camNeutral(identify.rawColors);

        for (int i = 0; i < identify.rawColors; ++i)
        {
            camNeutral[i] = 1.0/identify.cameraMult[i];
        }

        negative->SetCameraNeutral(camNeutral);

        if (d->cancel)
            return PROCESSCANCELED;

        // -----------------------------------------------------------------------------------------

        qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Updating metadata to DNG Negative" ;

        dng_exif* const exif = negative->GetExif();
        exif->fModel.Set_ASCII(identify.model.toLatin1().constData());
        exif->fMake.Set_ASCII(identify.make.toLatin1().constData());

        QString   str;
        DMetadata meta;

        if (meta.load(inputFile()))
        {
            long int   num, den;
            long       val;

            // Time from original shot
            dng_date_time_info dti;
            dti.SetDateTime(d->dngDateTime(meta.getImageDateTime()));
            exif->fDateTimeOriginal = dti;

            dti.SetDateTime(d->dngDateTime(meta.getDigitizationDateTime(true)));
            exif->fDateTimeDigitized = dti;

            negative->UpdateDateTime(dti);

            // String Tags

            str = meta.getExifTagString("Exif.Image.Make");
            if (!str.isEmpty()) exif->fMake.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.Image.Model");
            if (!str.isEmpty()) exif->fModel.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.Image.Software");
            if (!str.isEmpty()) exif->fSoftware.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.Image.ImageDescription");
            if (!str.isEmpty()) exif->fImageDescription.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.Image.Artist");
            if (!str.isEmpty()) exif->fArtist.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.Image.Copyright");
            if (!str.isEmpty()) exif->fCopyright.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.Photo.UserComment");
            if (!str.isEmpty()) exif->fUserComment.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.Image.CameraSerialNumber");
            if (!str.isEmpty()) exif->fCameraSerialNumber.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.GPSInfo.GPSLatitudeRef");
            if (!str.isEmpty()) exif->fGPSLatitudeRef.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.GPSInfo.GPSLongitudeRef");
            if (!str.isEmpty()) exif->fGPSLongitudeRef.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.GPSInfo.GPSSatellites");
            if (!str.isEmpty()) exif->fGPSSatellites.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.GPSInfo.GPSStatus");
            if (!str.isEmpty()) exif->fGPSStatus.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.GPSInfo.GPSMeasureMode");
            if (!str.isEmpty()) exif->fGPSMeasureMode.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.GPSInfo.GPSSpeedRef");
            if (!str.isEmpty()) exif->fGPSSpeedRef.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.GPSInfo.GPSTrackRef");
            if (!str.isEmpty()) exif->fGPSTrackRef.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.GPSInfo.GPSSpeedRef");
            if (!str.isEmpty()) exif->fGPSSpeedRef.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.GPSInfo.GPSImgDirectionRef");
            if (!str.isEmpty()) exif->fGPSSpeedRef.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.GPSInfo.GPSMapDatum");
            if (!str.isEmpty()) exif->fGPSMapDatum.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.GPSInfo.GPSDestLatitudeRef");
            if (!str.isEmpty()) exif->fGPSDestLatitudeRef.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.GPSInfo.GPSDestLongitudeRef");
            if (!str.isEmpty()) exif->fGPSDestLongitudeRef.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.GPSInfo.GPSDestBearingRef");
            if (!str.isEmpty()) exif->fGPSDestBearingRef.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.GPSInfo.GPSDestDistanceRef");
            if (!str.isEmpty()) exif->fGPSDestDistanceRef.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.GPSInfo.GPSProcessingMethod");
            if (!str.isEmpty()) exif->fGPSProcessingMethod.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.GPSInfo.GPSAreaInformation");
            if (!str.isEmpty()) exif->fGPSAreaInformation.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.GPSInfo.GPSDateStamp");
            if (!str.isEmpty()) exif->fGPSDateStamp.Set_ASCII(str.trimmed().toLatin1().constData());

            // Rational Tags

            if (meta.getExifTagRational("Exif.Photo.ExposureTime", num, den))          exif->fExposureTime             = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.Photo.FNumber", num, den))               exif->fFNumber                  = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.Photo.ShutterSpeedValue", num, den))     exif->fShutterSpeedValue        = dng_srational(num, den);
            if (meta.getExifTagRational("Exif.Photo.ApertureValue", num, den))         exif->fApertureValue            = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.Photo.BrightnessValue", num, den))       exif->fBrightnessValue          = dng_srational(num, den);
            if (meta.getExifTagRational("Exif.Photo.ExposureBiasValue", num, den))     exif->fExposureBiasValue        = dng_srational(num, den);
            if (meta.getExifTagRational("Exif.Photo.MaxApertureValue", num, den))      exif->fMaxApertureValue         = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.Photo.FocalLength", num, den))           exif->fFocalLength              = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.Photo.DigitalZoomRatio", num, den))      exif->fDigitalZoomRatio         = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.Photo.SubjectDistance", num, den))       exif->fSubjectDistance          = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.Image.BatteryLevel", num, den))          exif->fBatteryLevelR            = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.Photo.FocalPlaneXResolution", num, den)) exif->fFocalPlaneXResolution    = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.Photo.FocalPlaneYResolution", num, den)) exif->fFocalPlaneYResolution    = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.GPSInfo.GPSAltitude", num, den))         exif->fGPSAltitude              = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.GPSInfo.GPSDOP", num, den))              exif->fGPSDOP                   = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.GPSInfo.GPSSpeed", num, den))            exif->fGPSSpeed                 = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.GPSInfo.GPSTrack", num, den))            exif->fGPSTrack                 = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.GPSInfo.GPSImgDirection", num, den))     exif->fGPSImgDirection          = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.GPSInfo.GPSDestBearing", num, den))      exif->fGPSDestBearing           = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.GPSInfo.GPSDestDistance", num, den))     exif->fGPSDestDistance          = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.GPSInfo.GPSLatitude", num, den, 0))      exif->fGPSLatitude[0]           = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.GPSInfo.GPSLatitude", num, den, 1))      exif->fGPSLatitude[1]           = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.GPSInfo.GPSLatitude", num, den, 2))      exif->fGPSLatitude[2]           = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.GPSInfo.GPSLongitude", num, den, 0))     exif->fGPSLongitude[0]          = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.GPSInfo.GPSLongitude", num, den, 1))     exif->fGPSLongitude[1]          = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.GPSInfo.GPSLongitude", num, den, 2))     exif->fGPSLongitude[2]          = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.GPSInfo.GPSTimeStamp", num, den, 0))     exif->fGPSTimeStamp[0]          = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.GPSInfo.GPSTimeStamp", num, den, 1))     exif->fGPSTimeStamp[1]          = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.GPSInfo.GPSTimeStamp", num, den, 2))     exif->fGPSTimeStamp[2]          = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.GPSInfo.GPSDestLatitude", num, den, 0))  exif->fGPSDestLatitude[0]       = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.GPSInfo.GPSDestLatitude", num, den, 1))  exif->fGPSDestLatitude[1]       = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.GPSInfo.GPSDestLatitude", num, den, 2))  exif->fGPSDestLatitude[2]       = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.GPSInfo.GPSDestLongitude", num, den, 0)) exif->fGPSDestLongitude[0]      = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.GPSInfo.GPSDestLongitude", num, den, 1)) exif->fGPSDestLongitude[1]      = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.GPSInfo.GPSDestLongitude", num, den, 2)) exif->fGPSDestLongitude[2]      = dng_urational(num, den);

            // Integer Tags

            if (meta.getExifTagLong("Exif.Photo.ExposureProgram", val))                exif->fExposureProgram          = (uint32)val;
            if (meta.getExifTagLong("Exif.Photo.ISOSpeedRatings", val))                exif->fISOSpeedRatings[0]       = (uint32)val;
            if (meta.getExifTagLong("Exif.Photo.MeteringMode", val))                   exif->fMeteringMode             = (uint32)val;
            if (meta.getExifTagLong("Exif.Photo.LightSource", val))                    exif->fLightSource              = (uint32)val;
            if (meta.getExifTagLong("Exif.Photo.Flash", val))                          exif->fFlash                    = (uint32)val;
            if (meta.getExifTagLong("Exif.Photo.SensingMethod", val))                  exif->fSensingMethod            = (uint32)val;
            if (meta.getExifTagLong("Exif.Photo.FileSource", val))                     exif->fFileSource               = (uint32)val;
            if (meta.getExifTagLong("Exif.Photo.SceneType", val))                      exif->fSceneType                = (uint32)val;
            if (meta.getExifTagLong("Exif.Photo.CustomRendered", val))                 exif->fCustomRendered           = (uint32)val;
            if (meta.getExifTagLong("Exif.Photo.ExposureMode", val))                   exif->fExposureMode             = (uint32)val;
            if (meta.getExifTagLong("Exif.Photo.WhiteBalance", val))                   exif->fWhiteBalance             = (uint32)val;
            if (meta.getExifTagLong("Exif.Photo.SceneCaptureType", val))               exif->fSceneCaptureType         = (uint32)val;
            if (meta.getExifTagLong("Exif.Photo.GainControl", val))                    exif->fGainControl              = (uint32)val;
            if (meta.getExifTagLong("Exif.Photo.Contrast", val))                       exif->fContrast                 = (uint32)val;
            if (meta.getExifTagLong("Exif.Photo.Saturation", val))                     exif->fSaturation               = (uint32)val;
            if (meta.getExifTagLong("Exif.Photo.Sharpness", val))                      exif->fSharpness                = (uint32)val;
            if (meta.getExifTagLong("Exif.Photo.SubjectDistanceRange", val))           exif->fSubjectDistanceRange     = (uint32)val;
            if (meta.getExifTagLong("Exif.Photo.FocalLengthIn35mmFilm", val))          exif->fFocalLengthIn35mmFilm    = (uint32)val;
            if (meta.getExifTagLong("Exif.Photo.ComponentsConfiguration", val))        exif->fComponentsConfiguration  = (uint32)val;
            if (meta.getExifTagLong("Exif.Photo.PixelXDimension", val))                exif->fPixelXDimension          = (uint32)val;
            if (meta.getExifTagLong("Exif.Photo.PixelYDimension", val))                exif->fPixelYDimension          = (uint32)val;
            if (meta.getExifTagLong("Exif.Photo.FocalPlaneResolutionUnit", val))       exif->fFocalPlaneResolutionUnit = (uint32)val;
            if (meta.getExifTagLong("Exif.GPSInfo.GPSAltitudeRef", val))               exif->fGPSAltitudeRef           = (uint32)val;
            if (meta.getExifTagLong("Exif.GPSInfo.GPSDifferential", val))              exif->fGPSDifferential          = (uint32)val;
            long gpsVer1 = 0;
            long gpsVer2 = 0;
            long gpsVer3 = 0;
            long gpsVer4 = 0;
            meta.getExifTagLong("Exif.GPSInfo.GPSVersionID", gpsVer1, 0);
            meta.getExifTagLong("Exif.GPSInfo.GPSVersionID", gpsVer2, 1);
            meta.getExifTagLong("Exif.GPSInfo.GPSVersionID", gpsVer3, 2);
            meta.getExifTagLong("Exif.GPSInfo.GPSVersionID", gpsVer4, 3);
            long gpsVer  = 0;
            gpsVer += gpsVer1 << 24;
            gpsVer += gpsVer2 << 16;
            gpsVer += gpsVer3 <<  8;
            gpsVer += gpsVer4;
            exif->fGPSVersionID = (uint32)gpsVer;

            // Nikon Markernotes

            if (meta.getExifTagRational("Exif.Nikon3.Lens", num, den, 0))              exif->fLensInfo[0]              = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.Nikon3.Lens", num, den, 1))              exif->fLensInfo[1]              = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.Nikon3.Lens", num, den, 2))              exif->fLensInfo[2]              = dng_urational(num, den);
            if (meta.getExifTagRational("Exif.Nikon3.Lens", num, den, 3))              exif->fLensInfo[3]              = dng_urational(num, den);
            if (meta.getExifTagLong("Exif.Nikon3.ISOSpeed", val, 1))                   exif->fISOSpeedRatings[1]       = (uint32)val;

            str = meta.getExifTagString("Exif.Nikon3.SerialNO");
            if (!str.isEmpty()) str = str.replace(QLatin1String("NO="), QLatin1String(""));
            if (!str.isEmpty()) exif->fCameraSerialNumber.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.Nikon3.SerialNumber");
            if (!str.isEmpty()) exif->fCameraSerialNumber.Set_ASCII(str.toLatin1().constData());

            if (meta.getExifTagLong("Exif.Nikon3.ShutterCount", val))                  exif->fImageNumber              = (uint32)val;
            if (meta.getExifTagLong("Exif.NikonLd1.LensIDNumber", val))                exif->fLensID.Set_ASCII((QString::fromUtf8("%1").arg(val)).toLatin1().constData());
            if (meta.getExifTagLong("Exif.NikonLd2.LensIDNumber", val))                exif->fLensID.Set_ASCII((QString::fromUtf8("%1").arg(val)).toLatin1().constData());
            if (meta.getExifTagLong("Exif.NikonLd3.LensIDNumber", val))                exif->fLensID.Set_ASCII((QString::fromUtf8("%1").arg(val)).toLatin1().constData());
            if (meta.getExifTagLong("Exif.NikonLd2.FocusDistance", val))               exif->fSubjectDistance          = dng_urational((uint32)pow(10.0, val/40.0), 100);
            if (meta.getExifTagLong("Exif.NikonLd3.FocusDistance", val))               exif->fSubjectDistance          = dng_urational((uint32)pow(10.0, val/40.0), 100);
            str = meta.getExifTagString("Exif.NikonLd1.LensIDNumber");
            if (!str.isEmpty()) exif->fLensName.Set_ASCII(str.trimmed().toLatin1().constData());
            str = meta.getExifTagString("Exif.NikonLd2.LensIDNumber");
            if (!str.isEmpty()) exif->fLensName.Set_ASCII(str.trimmed().toLatin1().constData());
            str = meta.getExifTagString("Exif.NikonLd3.LensIDNumber");
            if (!str.isEmpty()) exif->fLensName.Set_ASCII(str.trimmed().toLatin1().constData());

            // Canon Markernotes

            if (meta.getExifTagLong("Exif.Canon.SerialNumber", val))                   exif->fCameraSerialNumber.Set_ASCII((QString::fromUtf8("%1").arg(val)).toLatin1().constData());
            //if (meta.getExifTagLong("Exif.CanonCs.LensType", val))                     exif->fLensID.Set_ASCII((QString::fromUtf8("%1").arg(val)).toLatin1().constData());
            //if (meta.getExifTagLong("Exif.CanonCs.FlashActivity", val))                exif->fFlash                    = (uint32)val;
            //if (meta.getExifTagLong("Exif.CanonFi.FileNumber", val))                   exif->fImageNumber              = (uint32)val;
            //if (meta.getExifTagLong("Exif.CanonCs.MaxAperture", val))                  exif->fMaxApertureValue         = dng_urational(val, 100000);
            if (meta.getExifTagLong("Exif.CanonCs.ExposureProgram", val))
            {
                switch (val)
                {
                    case 1:
                        exif->fExposureProgram = 2;
                        break;
                    case 2:
                        exif->fExposureProgram = 4;
                        break;
                    case 3:
                        exif->fExposureProgram = 3;
                        break;
                    case 4:
                        exif->fExposureProgram = 1;
                        break;
                    default:
                        break;
                }
            }

            if (meta.getExifTagLong("Exif.CanonCs.MeteringMode", val))
            {
                switch (val)
                {
                    case 1:
                        exif->fMeteringMode = 3;
                        break;
                    case 2:
                        exif->fMeteringMode = 1;
                        break;
                    case 3:
                        exif->fMeteringMode = 5;
                        break;
                    case 4:
                        exif->fMeteringMode = 6;
                        break;
                    case 5:
                        exif->fMeteringMode = 2;
                        break;
                    default:
                        break;
                }
            }

            long canonLensUnits = 1;
            if (meta.getExifTagRational("Exif.CanonCs.Lens", num, den, 2))             canonLensUnits                  = num;
            if (meta.getExifTagRational("Exif.CanonCs.Lens", num, den, 0))             exif->fLensInfo[1]              = dng_urational(num, canonLensUnits);
            if (meta.getExifTagRational("Exif.CanonCs.Lens", num, den, 1))             exif->fLensInfo[0]              = dng_urational(num, canonLensUnits);
            if (meta.getExifTagRational("Exif.Canon.FocalLength", num, den, 1))        exif->fFocalLength              = dng_urational(num, canonLensUnits);
            long canonLensType = 65535;
            if (meta.getExifTagLong("Exif.CanonCs.LensType", canonLensType))           exif->fLensID.Set_ASCII((QString::fromUtf8("%1").arg(canonLensType)).toLatin1().constData());
            str = meta.getExifTagString("Exif.Canon.LensModel");

            if (!str.isEmpty())
            {
                exif->fLensName.Set_ASCII(str.trimmed().toLatin1().constData());
            }
            else if (canonLensType != 65535)
            {
                str = meta.getExifTagString("Exif.CanonCs.LensType");
                if (!str.isEmpty()) exif->fLensName.Set_ASCII(str.trimmed().toLatin1().constData());
            }

            str = meta.getExifTagString("Exif.Canon.OwnerName");
            if (!str.isEmpty()) exif->fOwnerName.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.Canon.FirmwareVersion");
            if (!str.isEmpty()) str = str.replace(QLatin1String("Firmware"), QLatin1String(""));
            if (!str.isEmpty()) str = str.replace(QLatin1String("Version"), QLatin1String(""));
            if (!str.isEmpty()) exif->fFirmware.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.CanonSi.ISOSpeed");
            if (!str.isEmpty()) exif->fISOSpeedRatings[1] = str.toInt();

            // Pentax Markernotes

            str = meta.getExifTagString("Exif.Pentax.LensType");

            if (str.length())
            {
                exif->fLensName.Set_ASCII(str.trimmed().toLatin1().constData());
                exif->fLensName.TrimLeadingBlanks();
                exif->fLensName.TrimTrailingBlanks();
            }

            long pentaxLensId1 = 0;
            long pentaxLensId2 = 0;

            if ((meta.getExifTagLong("Exif.Pentax.LensType", pentaxLensId1, 0)) &&
                (meta.getExifTagLong("Exif.Pentax.LensType", pentaxLensId2, 1)))
            {
                exif->fLensID.Set_ASCII(QString::fromUtf8("%1").arg(pentaxLensId1, pentaxLensId2).toLatin1().constData());
            }

           // Olympus Makernotes

            str = meta.getExifTagString("Exif.OlympusEq.SerialNumber");
            if (str.length()) exif->fCameraSerialNumber.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.OlympusEq.LensSerialNumber");
            if (str.length()) exif->fLensSerialNumber.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.OlympusEq.LensModel");
            if (str.length()) exif->fLensName.Set_ASCII(str.trimmed().toLatin1().constData());

            if (meta.getExifTagLong("Exif.OlympusEq.MinFocalLength", val))             exif->fLensInfo[0]              = dng_urational(val, 1);
            if (meta.getExifTagLong("Exif.OlympusEq.MaxFocalLength", val))             exif->fLensInfo[1]              = dng_urational(val, 1);

            // Panasonic Makernotes

            str = meta.getExifTagString("Exif.Panasonic.LensType");
            if (str.length()) exif->fLensName.Set_ASCII(str.trimmed().toLatin1().constData());

            str = meta.getExifTagString("Exif.Panasonic.LensSerialNumber");
            if (str.length()) exif->fLensSerialNumber.Set_ASCII(str.trimmed().toLatin1().constData());

            // Sony Makernotes

            if (meta.getExifTagLong("Exif.Sony2.LensID", val))
            {
                exif->fLensID.Set_ASCII(QString::fromUtf8("%1").arg(val).toLatin1().constData());
            }

            str = meta.getExifTagString("Exif.Sony2.LensID");
            if (str.length()) exif->fLensName.Set_ASCII(str.trimmed().toLatin1().constData());

            // -------------------------------------------

            if ((exif->fLensName.IsEmpty()) && (exif->fLensInfo[0].As_real64() > 0) && (exif->fLensInfo[1].As_real64() > 0))
            {
                QString     lensName;
                QTextStream stream(&lensName);
                double      dval = (double)exif->fLensInfo[0].n / (double)exif->fLensInfo[0].d;
                stream << QString::fromUtf8("%1").arg(dval, 0, 'f', 1);

                if (exif->fLensInfo[0].As_real64() != exif->fLensInfo[1].As_real64())
                {
                    dval = (double)exif->fLensInfo[1].n / (double)exif->fLensInfo[1].d;
                    stream << QString::fromUtf8("-%1").arg(dval, 0, 'f', 1);
                }

                stream << " mm";

                if (exif->fLensInfo[2].As_real64() > 0)
                {
                    dval = (double)exif->fLensInfo[2].n / (double)exif->fLensInfo[2].d;
                    stream << QString::fromUtf8(" 1/%1").arg(dval, 0, 'f', 1);
                }

                if ((exif->fLensInfo[3].As_real64() > 0) &&
                    (exif->fLensInfo[2].As_real64() != exif->fLensInfo[3].As_real64()))
                {
                    dval = (double)exif->fLensInfo[3].n / (double)exif->fLensInfo[3].d;
                    stream << QString::fromUtf8("-%1").arg(dval, 0, 'f', 1);
                }

                exif->fLensName.Set_ASCII(lensName.toLatin1().constData());
            }

            // Markernote backup.

            QByteArray mkrnts = meta.getExifTagData("Exif.Photo.MakerNote");

            if (!mkrnts.isEmpty())
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Backup Makernote (" << mkrnts.size() << " bytes)" ;

                dng_memory_allocator memalloc(gDefaultDNGMemoryAllocator);
                dng_memory_stream stream(memalloc);
                stream.Put(mkrnts.data(), mkrnts.size());
                AutoPtr<dng_memory_block> block(host.Allocate(mkrnts.size()));
                stream.SetReadPosition(0);
                stream.Get(block->Buffer(), mkrnts.size());

                if (identifyMake.make != QLatin1String("Canon"))
                {
                    negative->SetMakerNote(block);
                    negative->SetMakerNoteSafety(true);
                }

                long mknOffset       = 0;
                QString mknByteOrder = meta.getExifTagString("Exif.MakerNote.ByteOrder");

                if ((meta.getExifTagLong("Exif.MakerNote.Offset", mknOffset)) && !mknByteOrder.isEmpty())
                {
                    dng_memory_stream streamPriv(memalloc);
                    streamPriv.SetBigEndian();

                    streamPriv.Put("Adobe", 5);
                    streamPriv.Put_uint8(0x00);
                    streamPriv.Put("MakN", 4);
                    streamPriv.Put_uint32(mkrnts.size() + mknByteOrder.size() + 4);
                    streamPriv.Put(mknByteOrder.toLatin1().constData(), mknByteOrder.size());
                    streamPriv.Put_uint32(mknOffset);
                    streamPriv.Put(mkrnts.data(), mkrnts.size());
                    AutoPtr<dng_memory_block> blockPriv(host.Allocate(streamPriv.Length()));
                    streamPriv.SetReadPosition(0);
                    streamPriv.Get(blockPriv->Buffer(), streamPriv.Length());
                    negative->SetPrivateData(blockPriv);
                }
            }
        }

        if (d->backupOriginalRawFile)
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Backup Original RAW file (" << inputInfo.size() << " bytes)";

            QFileInfo originalFileInfo(inputFile());

            QFile originalFile(originalFileInfo.absoluteFilePath());

            if (!originalFile.open(QIODevice::ReadOnly))
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Cannot open original RAW file to backup in DNG. Aborted...";
                return PROCESSFAILED;
            }

            QDataStream originalDataStream(&originalFile);

            quint32 forkLength = originalFileInfo.size();
            quint32 forkBlocks = (quint32)floor((forkLength + 65535.0) / 65536.0);

            QVector<quint32> offsets;
            quint32 offset = (2 + forkBlocks) * sizeof(quint32);
            offsets.push_back(offset);

            QByteArray originalDataBlock;
            originalDataBlock.resize(CHUNK);

            QTemporaryFile compressedFile;

            if (!compressedFile.open())
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Cannot open temporary file to write Zipped Raw data. Aborted...";
                return PROCESSFAILED;
            }

            QDataStream compressedDataStream(&compressedFile);

            for (quint32 block = 0; block < forkBlocks; ++block)
            {
                int originalBlockLength = originalDataStream.readRawData(originalDataBlock.data(), CHUNK);

                QByteArray compressedDataBlock = qCompress((const uchar*)originalDataBlock.data(), originalBlockLength, -1);
                compressedDataBlock.remove(0, 4); // removes qCompress own header
                qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: compressed data block " << originalBlockLength << " -> " << compressedDataBlock.size();

                offset += compressedDataBlock.size();
                offsets.push_back(offset);

                compressedDataStream.writeRawData(compressedDataBlock.data(), compressedDataBlock.size());
            }

            dng_memory_allocator memalloc(gDefaultDNGMemoryAllocator);
            dng_memory_stream tempDataStream(memalloc);
            tempDataStream.SetBigEndian(true);
            tempDataStream.Put_uint32(forkLength);

            for (qint32 idx = 0; idx < offsets.size(); ++idx)
            {
                tempDataStream.Put_uint32(offsets[idx]);
            }

            QByteArray compressedData;
            compressedData.resize(compressedFile.size());
            compressedFile.seek(0);
            compressedDataStream.readRawData(compressedData.data(), compressedData.size());
            tempDataStream.Put(compressedData.data(), compressedData.size());

            compressedFile.remove();

            tempDataStream.Put_uint32(0);
            tempDataStream.Put_uint32(0);
            tempDataStream.Put_uint32(0);
            tempDataStream.Put_uint32(0);
            tempDataStream.Put_uint32(0);
            tempDataStream.Put_uint32(0);
            tempDataStream.Put_uint32(0);

            AutoPtr<dng_memory_block> block(host.Allocate(tempDataStream.Length()));
            tempDataStream.SetReadPosition(0);
            tempDataStream.Get(block->Buffer(), tempDataStream.Length());

            dng_md5_printer md5;
            md5.Process(block->Buffer(), block->LogicalSize());
            negative->SetOriginalRawFileData(block);
            negative->SetOriginalRawFileDigest(md5.Result());
            negative->ValidateOriginalRawFileDigest();
        }

        if (d->cancel)
            return PROCESSCANCELED;

        // -----------------------------------------------------------------------------------------

        qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Build DNG Negative" ;

        // Assign Raw image data.
        negative->SetStage1Image(image);

        // Compute linearized and range mapped image
        negative->BuildStage2Image(host);

        // Compute demosaiced image (used by preview and thumbnail)
        negative->BuildStage3Image(host);

        negative->SynchronizeMetadata();
        negative->RebuildIPTC(true, false);

        if (d->cancel)
            return PROCESSCANCELED;

        // -----------------------------------------------------------------------------------------

        dng_preview_list previewList;

        if (d->previewMode != DNGWriter::NONE)
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: DNG preview image creation" ;

            // Construct a preview image as TIFF format.
            AutoPtr<dng_image> tiffImage;
            dng_render tiff_render(host, *negative);
            tiff_render.SetFinalSpace(dng_space_sRGB::Get());
            tiff_render.SetFinalPixelType(ttByte);
            tiff_render.SetMaximumSize(d->previewMode == MEDIUM ? 1280 : width);
            tiffImage.Reset(tiff_render.Render());

            dng_image_writer tiff_writer;
            AutoPtr<dng_memory_stream> dms(new dng_memory_stream(gDefaultDNGMemoryAllocator));

            tiff_writer.WriteTIFF(host, *dms, *tiffImage.Get(), piRGB,
                                  ccUncompressed, negative.Get(), &tiff_render.FinalSpace());

            // Write TIFF preview image data to a temp JPEG file
            std::vector<char> tiff_mem_buffer(dms->Length());
            dms->SetReadPosition(0);
            dms->Get(&tiff_mem_buffer.front(), tiff_mem_buffer.size());
            dms.Reset();

            QImage pre_image;

            if (!pre_image.loadFromData((uchar*)&tiff_mem_buffer.front(), tiff_mem_buffer.size(), "TIFF"))
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Cannot load TIFF preview data in memory. Aborted..." ;
                return PROCESSFAILED;
            }

            QTemporaryFile previewFile;

            if (!previewFile.open())
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Cannot open temporary file to write JPEG preview. Aborted..." ;
                return PROCESSFAILED;
            }

            if (!pre_image.save(previewFile.fileName(), "JPEG", 90))
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Cannot save file to write JPEG preview. Aborted..." ;
                return PROCESSFAILED;
            }

            // Load JPEG preview file data in DNG preview container.
            AutoPtr<dng_jpeg_preview> jpeg_preview;
            jpeg_preview.Reset(new dng_jpeg_preview);
            jpeg_preview->fPhotometricInterpretation = piYCbCr;
            jpeg_preview->fPreviewSize.v             = pre_image.height();
            jpeg_preview->fPreviewSize.h             = pre_image.width();
            jpeg_preview->fCompressedData.Reset(host.Allocate(previewFile.size()));

            QDataStream previewStream( &previewFile );
            previewStream.readRawData(jpeg_preview->fCompressedData->Buffer_char(), previewFile.size());

            AutoPtr<dng_preview> pp( dynamic_cast<dng_preview*>(jpeg_preview.Release()) );
            previewList.Append(pp);

            previewFile.remove();
        }

        if (d->cancel)
            return PROCESSCANCELED;

        // -----------------------------------------------------------------------------------------

        qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: DNG thumbnail creation" ;

        dng_image_preview thumbnail;
        dng_render thumbnail_render(host, *negative);
        thumbnail_render.SetFinalSpace(dng_space_sRGB::Get());
        thumbnail_render.SetFinalPixelType(ttByte);
        thumbnail_render.SetMaximumSize(256);
        thumbnail.fImage.Reset(thumbnail_render.Render());

        if (d->cancel)
            return PROCESSCANCELED;

        // -----------------------------------------------------------------------------------------

        qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Creating DNG file " << outputInfo.fileName() ;

        dng_image_writer writer;
        dng_file_stream filestream(QFile::encodeName(dngFilePath).constData(), true);

        writer.WriteDNG(host, filestream, *negative.Get(), thumbnail,
                        d->jpegLossLessCompression ? ccJPEG : ccUncompressed,
                        &previewList);

        // -----------------------------------------------------------------------------------------
        // Metadata makernote cleanup using Exiv2 for some RAW file types
        // See bug #204437 and #210371, and write XMP Sidecar if necessary

        if (meta.load(dngFilePath))
        {
            if (inputInfo.suffix().toUpper() == QLatin1String("ORF"))
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: cleanup makernotes using Exiv2" ;

                meta.setWriteRawFiles(true);
                meta.removeExifTag("Exif.OlympusIp.BlackLevel", false);
            }
            else
            {
                // Don't touch DNG file and create XMP sidecar depending of host application settings.
                meta.setWriteRawFiles(false);
            }

            meta.applyChanges();
        }

        // -----------------------------------------------------------------------------------------
        // update modification time if desired

        if (d->updateFileDate)
        {
            QDateTime date = meta.getImageDateTime();

            qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Setting modification date from meta data: " << date.toString();

            // don't touch access time
            struct stat st;

            if (::stat(QFile::encodeName(dngFilePath).constData(), &st) == 0)
            {
                struct utimbuf ut;
                ut.actime  = st.st_atime;
                ut.modtime = date.toTime_t();
                utime(QFile::encodeName(dngFilePath).constData(), &ut);
            }
        }
    }

    catch (const dng_exception& exception)
    {
        int ret = exception.ErrorCode();
        qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: DNG SDK exception code (" << ret << ")" ;
        return DNGSDKINTERNALERROR;
    }

    catch (...)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: DNG SDK exception code unknow" ;
        return DNGSDKINTERNALERROR;
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: DNG conversion complete..." ;

    return PROCESSCOMPLETE;
}

}  // namespace Digikam
