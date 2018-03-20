/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-09-15
 * Description : Exiv2 library interface.
 *               Common metadata image information manipulation methods
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "metaengine.h"
#include "metaengine_p.h"

// Qt includes

#include <QBuffer>

// Local includes

#include "metaengine_rotation.h"
#include "digikam_debug.h"

namespace Digikam
{

bool MetaEngine::setImageProgramId(const QString& program, const QString& version) const
{
    try
    {
        QString software(program);
        software.append(QString::fromLatin1("-"));
        software.append(version);

        // Set program info into Exif.Image.ProcessingSoftware tag (only available with Exiv2 >= 0.14.0).

        d->exifMetadata()["Exif.Image.ProcessingSoftware"] = std::string(software.toLatin1().constData());

        // See B.K.O #142564: Check if Exif.Image.Software already exist. If yes, do not touch this tag.

        if (!d->exifMetadata().empty())
        {
            Exiv2::ExifData exifData(d->exifMetadata());
            Exiv2::ExifKey key("Exif.Image.Software");
            Exiv2::ExifData::const_iterator it = exifData.findKey(key);

            if (it == exifData.end())
                d->exifMetadata()["Exif.Image.Software"] = std::string(software.toLatin1().constData());
        }

        // set program info into XMP tags.

#ifdef _XMP_SUPPORT_

        if (!d->xmpMetadata().empty())
        {
            // Only create Xmp.xmp.CreatorTool if it do not exist.
            Exiv2::XmpData xmpData(d->xmpMetadata());
            Exiv2::XmpKey key("Xmp.xmp.CreatorTool");
            Exiv2::XmpData::const_iterator it = xmpData.findKey(key);

            if (it == xmpData.end())
                setXmpTagString("Xmp.xmp.CreatorTool", software);
        }

        setXmpTagString("Xmp.tiff.Software", software);

#endif // _XMP_SUPPORT_

        // Set program info into IPTC tags.

        d->iptcMetadata()["Iptc.Application2.Program"]        = std::string(program.toLatin1().constData());
        d->iptcMetadata()["Iptc.Application2.ProgramVersion"] = std::string(version.toLatin1().constData());
        return true;
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot set Program identity into image using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

QSize MetaEngine::getImageDimensions() const
{
    try
    {
        long width=-1, height=-1;

        // Try to get Exif.Photo tags

        Exiv2::ExifData exifData(d->exifMetadata());
        Exiv2::ExifKey key("Exif.Photo.PixelXDimension");
        Exiv2::ExifData::const_iterator it = exifData.findKey(key);

        if (it != exifData.end() && it->count())
            width = it->toLong();

        Exiv2::ExifKey key2("Exif.Photo.PixelYDimension");
        Exiv2::ExifData::const_iterator it2 = exifData.findKey(key2);

        if (it2 != exifData.end() && it2->count())
            height = it2->toLong();

        if (width != -1 && height != -1)
            return QSize(width, height);

        // Try to get Exif.Image tags

        width  = -1;
        height = -1;

        Exiv2::ExifKey key3("Exif.Image.ImageWidth");
        Exiv2::ExifData::const_iterator it3 = exifData.findKey(key3);

        if (it3 != exifData.end() && it3->count())
            width = it3->toLong();

        Exiv2::ExifKey key4("Exif.Image.ImageLength");
        Exiv2::ExifData::const_iterator it4 = exifData.findKey(key4);

        if (it4 != exifData.end() && it4->count())
            height = it4->toLong();

        if (width != -1 && height != -1)
            return QSize(width, height);

#ifdef _XMP_SUPPORT_

        // Try to get Xmp.tiff tags

        width    = -1;
        height   = -1;
        bool wOk = false;
        bool hOk = false;

        QString str = getXmpTagString("Xmp.tiff.ImageWidth");

        if (!str.isEmpty())
            width = str.toInt(&wOk);

        str = getXmpTagString("Xmp.tiff.ImageLength");

        if (!str.isEmpty())
            height = str.toInt(&hOk);

        if (wOk && hOk)
            return QSize(width, height);

        // Try to get Xmp.exif tags

        width  = -1;
        height = -1;
        wOk    = false;
        hOk    = false;

        str = getXmpTagString("Xmp.exif.PixelXDimension");

        if (!str.isEmpty())
            width = str.toInt(&wOk);

        str = getXmpTagString("Xmp.exif.PixelYDimension");

        if (!str.isEmpty())
            height = str.toInt(&hOk);

        if (wOk && hOk)
            return QSize(width, height);

#endif // _XMP_SUPPORT_

    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot parse image dimensions tag using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QSize();
}

bool MetaEngine::setImageDimensions(const QSize& size) const
{
    try
    {
        // Set Exif values.

        // NOTE: see B.K.O #144604: need to cast to record an unsigned integer value.
        d->exifMetadata()["Exif.Image.ImageWidth"]      = static_cast<uint32_t>(size.width());
        d->exifMetadata()["Exif.Image.ImageLength"]     = static_cast<uint32_t>(size.height());
        d->exifMetadata()["Exif.Photo.PixelXDimension"] = static_cast<uint32_t>(size.width());
        d->exifMetadata()["Exif.Photo.PixelYDimension"] = static_cast<uint32_t>(size.height());

        // Set Xmp values.

#ifdef _XMP_SUPPORT_

        setXmpTagString("Xmp.tiff.ImageWidth",      QString::number(size.width()));
        setXmpTagString("Xmp.tiff.ImageLength",     QString::number(size.height()));
        setXmpTagString("Xmp.exif.PixelXDimension", QString::number(size.width()));
        setXmpTagString("Xmp.exif.PixelYDimension", QString::number(size.height()));

#endif // _XMP_SUPPORT_

        return true;
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot set image dimensions using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

MetaEngine::ImageOrientation MetaEngine::getImageOrientation() const
{
    try
    {
        Exiv2::ExifData exifData(d->exifMetadata());
        Exiv2::ExifData::iterator it;
        long orientation;
        ImageOrientation imageOrient = ORIENTATION_NORMAL;

        // -- Standard Xmp tag --------------------------------

#ifdef _XMP_SUPPORT_

        bool ok = false;
        QString str = getXmpTagString("Xmp.tiff.Orientation");

        if (!str.isEmpty())
        {
            orientation = str.toLong(&ok);

            if (ok)
            {
                qCDebug(DIGIKAM_METAENGINE_LOG) << "Orientation => Xmp.tiff.Orientation => " << (int)orientation;
                return (ImageOrientation)orientation;
            }
        }

#endif // _XMP_SUPPORT_

        // Because some camera set a wrong standard exif orientation tag,
        // We need to check makernote tags in first!

        // -- Minolta Cameras ----------------------------------

        Exiv2::ExifKey minoltaKey1("Exif.MinoltaCs7D.Rotation");
        it = exifData.findKey(minoltaKey1);

        if (it != exifData.end() && it->count())
        {
            orientation = it->toLong();
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Orientation => Exif.MinoltaCs7D.Rotation => " << (int)orientation;

            switch(orientation)
            {
                case 76:
                    imageOrient = ORIENTATION_ROT_90;
                    break;
                case 82:
                    imageOrient = ORIENTATION_ROT_270;
                    break;
            }

            return imageOrient;
        }

        Exiv2::ExifKey minoltaKey2("Exif.MinoltaCs5D.Rotation");
        it = exifData.findKey(minoltaKey2);

        if (it != exifData.end() && it->count())
        {
            orientation = it->toLong();
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Orientation => Exif.MinoltaCs5D.Rotation => " << (int)orientation;

            switch(orientation)
            {
                case 76:
                    imageOrient = ORIENTATION_ROT_90;
                    break;
                case 82:
                    imageOrient = ORIENTATION_ROT_270;
                    break;
            }

            return imageOrient;
        }

        // -- Standard Exif tag --------------------------------

        Exiv2::ExifKey keyStd("Exif.Image.Orientation");
        it = exifData.findKey(keyStd);

        if (it != exifData.end() && it->count())
        {
            orientation = it->toLong();
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Orientation => Exif.Image.Orientation => " << (int)orientation;
            return (ImageOrientation)orientation;
        }

    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot parse Exif Orientation tag using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return ORIENTATION_UNSPECIFIED;
}

bool MetaEngine::setImageOrientation(ImageOrientation orientation) const
{
    try
    {
        if (orientation < ORIENTATION_UNSPECIFIED || orientation > ORIENTATION_ROT_270)
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Image orientation value is not correct!";
            return false;
        }

        // Set Exif values.

        d->exifMetadata()["Exif.Image.Orientation"] = static_cast<uint16_t>(orientation);
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Exif.Image.Orientation tag set to: " << (int)orientation;

        // Set Xmp values.

#ifdef _XMP_SUPPORT_

        setXmpTagString("Xmp.tiff.Orientation", QString::number((int)orientation));

#endif // _XMP_SUPPORT_

        // -- Minolta/Sony Cameras ----------------------------------

        // Minolta and Sony camera store image rotation in Makernote.
        // We remove these information to prevent duplicate values.

        Exiv2::ExifData::iterator it;

        Exiv2::ExifKey minoltaKey1("Exif.MinoltaCs7D.Rotation");
        it = d->exifMetadata().findKey(minoltaKey1);

        if (it != d->exifMetadata().end())
        {
            d->exifMetadata().erase(it);
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Removing Exif.MinoltaCs7D.Rotation tag";
        }

        Exiv2::ExifKey minoltaKey2("Exif.MinoltaCs5D.Rotation");
        it = d->exifMetadata().findKey(minoltaKey2);

        if (it != d->exifMetadata().end())
        {
            d->exifMetadata().erase(it);
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Removing Exif.MinoltaCs5D.Rotation tag";
        }

        // -- Exif embedded thumbnail ----------------------------------

        Exiv2::ExifKey thumbKey("Exif.Thumbnail.Orientation");
        it = d->exifMetadata().findKey(thumbKey);

        if (it != d->exifMetadata().end() && it->count())
        {
            MetaEngineRotation operation((MetaEngine::ImageOrientation)it->toLong());
            operation *= orientation;
            (*it) = static_cast<uint16_t>(operation.exifOrientation());
        }

        return true;
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot set Exif Orientation tag using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

MetaEngine::ImageColorWorkSpace MetaEngine::getImageColorWorkSpace() const
{
    // Check Exif values.

    long exifColorSpace = -1;

    if (!getExifTagLong("Exif.Photo.ColorSpace", exifColorSpace))
    {
#ifdef _XMP_SUPPORT_
        QVariant var = getXmpTagVariant("Xmp.exif.ColorSpace");
        if (!var.isNull())
            exifColorSpace = var.toInt();
#endif // _XMP_SUPPORT_
    }

    if (exifColorSpace == 1)
    {
        return WORKSPACE_SRGB; // as specified by standard
    }
    else if (exifColorSpace == 2)
    {
        return WORKSPACE_ADOBERGB; // not in the standard!
    }
    else
    {
        if (exifColorSpace == 65535)
        {
            // A lot of cameras set the Exif.Iop.InteroperabilityIndex,
            // as documented for ExifTool
            QString interopIndex = getExifTagString("Exif.Iop.InteroperabilityIndex");

            if (!interopIndex.isNull())
            {
                if (interopIndex == QString::fromLatin1("R03"))
                    return WORKSPACE_ADOBERGB;
                else if (interopIndex == QString::fromLatin1("R98"))
                    return WORKSPACE_SRGB;
            }
        }

        // Note: Text EXIF ColorSpace tag may just not be present (NEF files)

        // Nikon camera set Exif.Photo.ColorSpace to uncalibrated or just skip this field,
        // then add additional information into the makernotes.
        // Exif.Nikon3.ColorSpace: 1 => sRGB, 2 => AdobeRGB
        long nikonColorSpace;

        if (getExifTagLong("Exif.Nikon3.ColorSpace", nikonColorSpace))
        {
            if (nikonColorSpace == 1)
                return WORKSPACE_SRGB;
            else if (nikonColorSpace == 2)
                return WORKSPACE_ADOBERGB;
        }
        // Exif.Nikon3.ColorMode is set to "MODE2" for AdobeRGB, but there are sometimes two ColorMode fields
        // in a NEF, with the first one "COLOR" and the second one "MODE2"; but in this case, ColorSpace (above) was set.
        if (getExifTagString("Exif.Nikon3.ColorMode").contains(QString::fromLatin1("MODE2")))
            return WORKSPACE_ADOBERGB;

        //TODO: This makernote tag (0x00b4) must be added to libexiv2
        /*
        long canonColorSpace;
        if (getExifTagLong("Exif.Canon.ColorSpace", canonColorSpace))
        {
            if (canonColorSpace == 1)
                return WORKSPACE_SRGB;
            else if (canonColorSpace == 2)
                return WORKSPACE_ADOBERGB;
        }
        */

        // TODO : add more Makernote parsing here ...

        if (exifColorSpace == 65535)
            return WORKSPACE_UNCALIBRATED;
    }

    return WORKSPACE_UNSPECIFIED;
}

bool MetaEngine::setImageColorWorkSpace(ImageColorWorkSpace workspace) const
{
    try
    {
        // Set Exif value.

        d->exifMetadata()["Exif.Photo.ColorSpace"] = static_cast<uint16_t>(workspace);

        // Set Xmp value.

#ifdef _XMP_SUPPORT_

        setXmpTagString("Xmp.exif.ColorSpace", QString::number((int)workspace));

#endif // _XMP_SUPPORT_

        return true;
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot set Exif color workspace tag using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

QDateTime MetaEngine::getImageDateTime() const
{
    try
    {
        // In first, trying to get Date & time from Exif tags.

        if (!d->exifMetadata().empty())
        {
            Exiv2::ExifData exifData(d->exifMetadata());
            {
                Exiv2::ExifKey key("Exif.Photo.DateTimeOriginal");
                Exiv2::ExifData::const_iterator it = exifData.findKey(key);

                if (it != exifData.end())
                {
                    QDateTime dateTime = QDateTime::fromString(QString::fromLatin1(it->toString().c_str()), Qt::ISODate);

                    if (dateTime.isValid())
                    {
                        qCDebug(DIGIKAM_METAENGINE_LOG) << "DateTime => Exif.Photo.DateTimeOriginal => " << dateTime;
                        return dateTime;
                    }
                }
            }
            {
                Exiv2::ExifKey key("Exif.Photo.DateTimeDigitized");
                Exiv2::ExifData::const_iterator it = exifData.findKey(key);

                if (it != exifData.end())
                {
                    QDateTime dateTime = QDateTime::fromString(QString::fromLatin1(it->toString().c_str()), Qt::ISODate);

                    if (dateTime.isValid())
                    {
                        qCDebug(DIGIKAM_METAENGINE_LOG) << "DateTime => Exif.Photo.DateTimeDigitized => " << dateTime;
                        return dateTime;
                    }
                }
            }
            {
                Exiv2::ExifKey key("Exif.Image.DateTime");
                Exiv2::ExifData::const_iterator it = exifData.findKey(key);

                if (it != exifData.end())
                {
                    QDateTime dateTime = QDateTime::fromString(QString::fromLatin1(it->toString().c_str()), Qt::ISODate);

                    if (dateTime.isValid())
                    {
                        qCDebug(DIGIKAM_METAENGINE_LOG) << "DateTime => Exif.Image.DateTime => " << dateTime;
                        return dateTime;
                    }
                }
            }
        }

        // In second, trying to get Date & time from Xmp tags.

#ifdef _XMP_SUPPORT_

        if (!d->xmpMetadata().empty())
        {
            Exiv2::XmpData xmpData(d->xmpMetadata());
            {
                Exiv2::XmpKey key("Xmp.exif.DateTimeOriginal");
                Exiv2::XmpData::const_iterator it = xmpData.findKey(key);

                if (it != xmpData.end())
                {
                    QDateTime dateTime = QDateTime::fromString(QString::fromLatin1(it->toString().c_str()), Qt::ISODate);

                    if (dateTime.isValid())
                    {
                        qCDebug(DIGIKAM_METAENGINE_LOG) << "DateTime => Xmp.exif.DateTimeOriginal => " << dateTime;
                        return dateTime;
                    }
                }
            }
            {
                Exiv2::XmpKey key("Xmp.exif.DateTimeDigitized");
                Exiv2::XmpData::const_iterator it = xmpData.findKey(key);

                if (it != xmpData.end())
                {
                    QDateTime dateTime = QDateTime::fromString(QString::fromLatin1(it->toString().c_str()), Qt::ISODate);

                    if (dateTime.isValid())
                    {
                        qCDebug(DIGIKAM_METAENGINE_LOG) << "DateTime => Xmp.exif.DateTimeDigitized => " << dateTime;
                        return dateTime;
                    }
                }
            }
            {
                Exiv2::XmpKey key("Xmp.photoshop.DateCreated");
                Exiv2::XmpData::const_iterator it = xmpData.findKey(key);

                if (it != xmpData.end())
                {
                    QDateTime dateTime = QDateTime::fromString(QString::fromLatin1(it->toString().c_str()), Qt::ISODate);

                    if (dateTime.isValid())
                    {
                        qCDebug(DIGIKAM_METAENGINE_LOG) << "DateTime => Xmp.photoshop.DateCreated => " << dateTime;
                        return dateTime;
                    }
                }
            }
            {
                Exiv2::XmpKey key("Xmp.xmp.CreateDate");
                Exiv2::XmpData::const_iterator it = xmpData.findKey(key);

                if (it != xmpData.end())
                {
                    QDateTime dateTime = QDateTime::fromString(QString::fromLatin1(it->toString().c_str()), Qt::ISODate);

                    if (dateTime.isValid())
                    {
                        qCDebug(DIGIKAM_METAENGINE_LOG) << "DateTime => Xmp.xmp.CreateDate => " << dateTime;
                        return dateTime;
                    }
                }
            }
            {
                Exiv2::XmpKey key("Xmp.tiff.DateTime");
                Exiv2::XmpData::const_iterator it = xmpData.findKey(key);

                if (it != xmpData.end())
                {
                    QDateTime dateTime = QDateTime::fromString(QString::fromLatin1(it->toString().c_str()), Qt::ISODate);

                    if (dateTime.isValid())
                    {
                        qCDebug(DIGIKAM_METAENGINE_LOG) << "DateTime => Xmp.tiff.DateTime => " << dateTime;
                        return dateTime;
                    }
                }
            }
            {
                Exiv2::XmpKey key("Xmp.xmp.ModifyDate");
                Exiv2::XmpData::const_iterator it = xmpData.findKey(key);

                if (it != xmpData.end())
                {
                    QDateTime dateTime = QDateTime::fromString(QString::fromLatin1(it->toString().c_str()), Qt::ISODate);

                    if (dateTime.isValid())
                    {
                        qCDebug(DIGIKAM_METAENGINE_LOG) << "DateTime => Xmp.xmp.ModifyDate => " << dateTime;
                        return dateTime;
                    }
                }
            }
            {
                Exiv2::XmpKey key("Xmp.xmp.MetadataDate");
                Exiv2::XmpData::const_iterator it = xmpData.findKey(key);

                if (it != xmpData.end())
                {
                    QDateTime dateTime = QDateTime::fromString(QString::fromLatin1(it->toString().c_str()), Qt::ISODate);

                    if (dateTime.isValid())
                    {
                        qCDebug(DIGIKAM_METAENGINE_LOG) << "DateTime => Xmp.xmp.MetadataDate => " << dateTime;
                        return dateTime;
                    }
                }
            }

            // Video files support

            {
                Exiv2::XmpKey key("Xmp.video.DateTimeOriginal");
                Exiv2::XmpData::const_iterator it = xmpData.findKey(key);

                if (it != xmpData.end())
                {
                    QDateTime dateTime = QDateTime::fromString(QString::fromLatin1(it->toString().c_str()), Qt::ISODate);

                    if (dateTime.isValid())
                    {
                        qCDebug(DIGIKAM_METAENGINE_LOG) << "DateTime => Xmp.video.DateTimeOriginal => " << dateTime;
                        return dateTime;
                    }
                }
            }
            {
                Exiv2::XmpKey key("Xmp.video.DateUTC");
                Exiv2::XmpData::const_iterator it = xmpData.findKey(key);

                if (it != xmpData.end())
                {
                    QDateTime dateTime = QDateTime::fromString(QString::fromLatin1(it->toString().c_str()), Qt::ISODate);

                    if (dateTime.isValid())
                    {
                        qCDebug(DIGIKAM_METAENGINE_LOG) << "DateTime => Xmp.video.DateUTC => " << dateTime;
                        return dateTime;
                    }
                }
            }
            {
                Exiv2::XmpKey key("Xmp.video.ModificationDate");
                Exiv2::XmpData::const_iterator it = xmpData.findKey(key);

                if (it != xmpData.end())
                {
                    QDateTime dateTime = QDateTime::fromString(QString::fromLatin1(it->toString().c_str()), Qt::ISODate);

                    if (dateTime.isValid())
                    {
                        qCDebug(DIGIKAM_METAENGINE_LOG) << "DateTime => Xmp.video.ModificationDate => " << dateTime;
                        return dateTime;
                    }
                }
            }
            {
                Exiv2::XmpKey key("Xmp.video.DateTimeDigitized");
                Exiv2::XmpData::const_iterator it = xmpData.findKey(key);

                if (it != xmpData.end())
                {
                    QDateTime dateTime = QDateTime::fromString(QString::fromLatin1(it->toString().c_str()), Qt::ISODate);

                    if (dateTime.isValid())
                    {
                        qCDebug(DIGIKAM_METAENGINE_LOG) << "DateTime => Xmp.video.DateTimeDigitized => " << dateTime;
                        return dateTime;
                    }
                }
            }
        }

#endif // _XMP_SUPPORT_

        // In third, trying to get Date & time from Iptc tags.

        if (!d->iptcMetadata().empty())
        {
            Exiv2::IptcData iptcData(d->iptcMetadata());

            // Try creation Iptc date & time entries.

            Exiv2::IptcKey keyDateCreated("Iptc.Application2.DateCreated");
            Exiv2::IptcData::const_iterator it = iptcData.findKey(keyDateCreated);

            if (it != iptcData.end())
            {
                QString IptcDateCreated(QString::fromLatin1(it->toString().c_str()));
                Exiv2::IptcKey keyTimeCreated("Iptc.Application2.TimeCreated");
                Exiv2::IptcData::const_iterator it2 = iptcData.findKey(keyTimeCreated);

                if (it2 != iptcData.end())
                {
                    QString IptcTimeCreated(QString::fromLatin1(it2->toString().c_str()));
                    QDate date = QDate::fromString(IptcDateCreated, Qt::ISODate);
                    QTime time = QTime::fromString(IptcTimeCreated, Qt::ISODate);
                    QDateTime dateTime = QDateTime(date, time);

                    if (dateTime.isValid())
                    {
                        qCDebug(DIGIKAM_METAENGINE_LOG) << "DateTime => Iptc.Application2.DateCreated => " << dateTime;
                        return dateTime;
                    }
                }
            }

            // Try digitization Iptc date & time entries.

            Exiv2::IptcKey keyDigitizationDate("Iptc.Application2.DigitizationDate");
            Exiv2::IptcData::const_iterator it3 = iptcData.findKey(keyDigitizationDate);

            if (it3 != iptcData.end())
            {
                QString IptcDateDigitization(QString::fromLatin1(it3->toString().c_str()));
                Exiv2::IptcKey keyDigitizationTime("Iptc.Application2.DigitizationTime");
                Exiv2::IptcData::const_iterator it4 = iptcData.findKey(keyDigitizationTime);

                if (it4 != iptcData.end())
                {
                    QString IptcTimeDigitization(QString::fromLatin1(it4->toString().c_str()));
                    QDate date = QDate::fromString(IptcDateDigitization, Qt::ISODate);
                    QTime time = QTime::fromString(IptcTimeDigitization, Qt::ISODate);
                    QDateTime dateTime = QDateTime(date, time);

                    if (dateTime.isValid())
                    {
                        qCDebug(DIGIKAM_METAENGINE_LOG) << "DateTime => Iptc.Application2.DigitizationDate => " << dateTime;
                        return dateTime;
                    }
                }
            }
        }
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot parse Exif date & time tag using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QDateTime();
}

bool MetaEngine::setImageDateTime(const QDateTime& dateTime, bool setDateTimeDigitized) const
{
    if (!dateTime.isValid())
        return false;

    try
    {
        // In first we write date & time into Exif.

        // DateTimeDigitized is set by slide scanners etc. when a picture is digitized.
        // DateTimeOriginal specifies the date/time when the picture was taken.
        // For digital cameras, these dates should be both set, and identical.
        // Reference: http://www.exif.org/Exif2-2.PDF, chapter 4.6.5, table 4, section F.

        const std::string& exifdatetime(dateTime.toString(QString::fromLatin1("yyyy:MM:dd hh:mm:ss")).toLatin1().constData());
        d->exifMetadata()["Exif.Image.DateTime"]         = exifdatetime;
        d->exifMetadata()["Exif.Photo.DateTimeOriginal"] = exifdatetime;

        if (setDateTimeDigitized)
            d->exifMetadata()["Exif.Photo.DateTimeDigitized"] = exifdatetime;

#ifdef _XMP_SUPPORT_

        // In second we write date & time into Xmp.

        const std::string& xmpdatetime(dateTime.toString(Qt::ISODate).toLatin1().constData());

        Exiv2::Value::AutoPtr xmpTxtVal = Exiv2::Value::create(Exiv2::xmpText);
        xmpTxtVal->read(xmpdatetime);
        d->xmpMetadata().add(Exiv2::XmpKey("Xmp.exif.DateTimeOriginal"),  xmpTxtVal.get());
        d->xmpMetadata().add(Exiv2::XmpKey("Xmp.photoshop.DateCreated"),  xmpTxtVal.get());
        d->xmpMetadata().add(Exiv2::XmpKey("Xmp.tiff.DateTime"),          xmpTxtVal.get());
        d->xmpMetadata().add(Exiv2::XmpKey("Xmp.xmp.CreateDate"),         xmpTxtVal.get());
        d->xmpMetadata().add(Exiv2::XmpKey("Xmp.xmp.MetadataDate"),       xmpTxtVal.get());
        d->xmpMetadata().add(Exiv2::XmpKey("Xmp.xmp.ModifyDate"),         xmpTxtVal.get());
        d->xmpMetadata().add(Exiv2::XmpKey("Xmp.video.DateTimeOriginal"), xmpTxtVal.get());
        d->xmpMetadata().add(Exiv2::XmpKey("Xmp.video.DateUTC"),          xmpTxtVal.get());
        d->xmpMetadata().add(Exiv2::XmpKey("Xmp.video.ModificationDate"), xmpTxtVal.get());

        if (setDateTimeDigitized)
        {
            d->xmpMetadata().add(Exiv2::XmpKey("Xmp.exif.DateTimeDigitized"),  xmpTxtVal.get());
            d->xmpMetadata().add(Exiv2::XmpKey("Xmp.video.DateTimeDigitized"), xmpTxtVal.get());
        }

        // Tag not updated:
        // "Xmp.dc.DateTime" is a sequence of date relevant of dublin core change.
        //                   This is not the picture date as well

#endif // _XMP_SUPPORT_

        // In third we write date & time into Iptc.

        const std::string& iptcdate(dateTime.date().toString(Qt::ISODate).toLatin1().constData());
        const std::string& iptctime(dateTime.time().toString(Qt::ISODate).toLatin1().constData());
        d->iptcMetadata()["Iptc.Application2.DateCreated"] = iptcdate;
        d->iptcMetadata()["Iptc.Application2.TimeCreated"] = iptctime;

        if (setDateTimeDigitized)
        {
            d->iptcMetadata()["Iptc.Application2.DigitizationDate"] = iptcdate;
            d->iptcMetadata()["Iptc.Application2.DigitizationTime"] = iptctime;
        }

        return true;
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot set Date & Time into image using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

QDateTime MetaEngine::getDigitizationDateTime(bool fallbackToCreationTime) const
{
    try
    {
        // In first, trying to get Date & time from Exif tags.

        if (!d->exifMetadata().empty())
        {
            // Try Exif date time digitized.

            Exiv2::ExifData exifData(d->exifMetadata());
            Exiv2::ExifKey key("Exif.Photo.DateTimeDigitized");
            Exiv2::ExifData::const_iterator it = exifData.findKey(key);

            if (it != exifData.end())
            {
                QDateTime dateTime = QDateTime::fromString(QString::fromLatin1(it->toString().c_str()), Qt::ISODate);

                if (dateTime.isValid())
                {
                    qCDebug(DIGIKAM_METAENGINE_LOG) << "DateTime (Exif digitalized): " << dateTime.toString().toLatin1().constData();
                    return dateTime;
                }
            }
        }

        // In second, we trying XMP

#ifdef _XMP_SUPPORT_

        if (!d->xmpMetadata().empty())
        {
            Exiv2::XmpData xmpData(d->xmpMetadata());
            {
                Exiv2::XmpKey key("Xmp.exif.DateTimeDigitized");
                Exiv2::XmpData::const_iterator it = xmpData.findKey(key);

                if (it != xmpData.end())
                {
                    QDateTime dateTime = QDateTime::fromString(QString::fromLatin1(it->toString().c_str()), Qt::ISODate);

                    if (dateTime.isValid())
                    {
                        qCDebug(DIGIKAM_METAENGINE_LOG) << "DateTime (XMP-Exif digitalized): " << dateTime.toString().toLatin1().constData();
                        return dateTime;
                    }
                }
            }
            {
                Exiv2::XmpKey key("Xmp.video.DateTimeDigitized");
                Exiv2::XmpData::const_iterator it = xmpData.findKey(key);

                if (it != xmpData.end())
                {
                    QDateTime dateTime = QDateTime::fromString(QString::fromLatin1(it->toString().c_str()), Qt::ISODate);

                    if (dateTime.isValid())
                    {
                        qCDebug(DIGIKAM_METAENGINE_LOG) << "DateTime (XMP-Video digitalized): " << dateTime.toString().toLatin1().constData();
                        return dateTime;
                    }
                }
            }
        }

#endif // _XMP_SUPPORT_

        // In third, trying to get Date & time from Iptc tags.

        if (!d->iptcMetadata().empty())
        {
            // Try digitization Iptc date time entries.

            Exiv2::IptcData iptcData(d->iptcMetadata());
            Exiv2::IptcKey keyDigitizationDate("Iptc.Application2.DigitizationDate");
            Exiv2::IptcData::const_iterator it = iptcData.findKey(keyDigitizationDate);

            if (it != iptcData.end())
            {
                QString IptcDateDigitization(QString::fromLatin1(it->toString().c_str()));

                Exiv2::IptcKey keyDigitizationTime("Iptc.Application2.DigitizationTime");
                Exiv2::IptcData::const_iterator it2 = iptcData.findKey(keyDigitizationTime);

                if (it2 != iptcData.end())
                {
                    QString IptcTimeDigitization(QString::fromLatin1(it2->toString().c_str()));

                    QDate date = QDate::fromString(IptcDateDigitization, Qt::ISODate);
                    QTime time = QTime::fromString(IptcTimeDigitization, Qt::ISODate);
                    QDateTime dateTime = QDateTime(date, time);

                    if (dateTime.isValid())
                    {
                        qCDebug(DIGIKAM_METAENGINE_LOG) << "Date (IPTC digitalized): " << dateTime.toString().toLatin1().constData();
                        return dateTime;
                    }
                }
            }
        }
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot parse Exif digitization date & time tag using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    if (fallbackToCreationTime)
        return getImageDateTime();
    else
        return QDateTime();
}

bool MetaEngine::getImagePreview(QImage& preview) const
{
    try
    {
        // In first we trying to get from Iptc preview tag.
        if (preview.loadFromData(getIptcTagData("Iptc.Application2.Preview")) )
            return true;

        // TODO : Added here Makernotes preview extraction when Exiv2 will be fixed for that.
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot get image preview using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::setImagePreview(const QImage& preview) const
{
    if (preview.isNull())
    {
        removeIptcTag("Iptc.Application2.Preview");
        removeIptcTag("Iptc.Application2.PreviewFormat");
        removeIptcTag("Iptc.Application2.PreviewVersion");
        return true;
    }

    try
    {
        QByteArray data;
        QBuffer buffer(&data);
        buffer.open(QIODevice::WriteOnly);

        // A little bit compressed preview jpeg image to limit IPTC size.
        preview.save(&buffer, "JPEG");
        qCDebug(DIGIKAM_METAENGINE_LOG) << "JPEG image preview size: (" << preview.width() << " x "
                 << preview.height() << ") pixels - " << data.size() << " bytes";

        Exiv2::DataValue val;
        val.read((Exiv2::byte *)data.data(), data.size());
        d->iptcMetadata()["Iptc.Application2.Preview"] = val;

        // See http://www.iptc.org/std/IIM/4.1/specification/IIMV4.1.pdf Appendix A for details.
        d->iptcMetadata()["Iptc.Application2.PreviewFormat"]  = 11;  // JPEG
        d->iptcMetadata()["Iptc.Application2.PreviewVersion"] = 1;

        return true;
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot get image preview using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

}  // namespace Digikam
