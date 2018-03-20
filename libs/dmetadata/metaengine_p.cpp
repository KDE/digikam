/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-09-15
 * Description : Exiv2 library interface.
 *               Internal private container.
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

#include "metaengine_p.h"

// C ANSI includes

extern "C"
{
#include <sys/stat.h>

#ifndef _MSC_VER
#   include <utime.h>
#else
#   include <sys/utime.h>
#endif
}

// Qt includes

#include <QTextCodec>

// Local includes

#include "digikam_debug.h"

// Pragma directives to reduce warnings from Exiv2.
#if !defined(__APPLE__) && defined(__GNUC__)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(__APPLE__) && defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace Digikam
{

MetaEngine::Private::Private()
    : data(new MetaEngineData::Private)
{
    writeRawFiles         = false;
    updateFileTimeStamp   = false;
    useXMPSidecar4Reading = false;
    metadataWritingMode   = WRITETOIMAGEONLY;
    loadedFromSidecar     = false;
    Exiv2::LogMsg::setHandler(MetaEngine::Private::printExiv2MessageHandler);
}

MetaEngine::Private::~Private()
{
}

void MetaEngine::Private::copyPrivateData(const Private* const other)
{
    data                  = other->data;
    filePath              = other->filePath;
    writeRawFiles         = other->writeRawFiles;
    updateFileTimeStamp   = other->updateFileTimeStamp;
    useXMPSidecar4Reading = other->useXMPSidecar4Reading;
    metadataWritingMode   = other->metadataWritingMode;
}

bool MetaEngine::Private::saveToXMPSidecar(const QFileInfo& finfo) const
{
    QString filePath = MetaEngine::sidecarFilePathForFile(finfo.filePath());

    if (filePath.isEmpty())
    {
        return false;
    }

    try
    {
        Exiv2::Image::AutoPtr image;
        image = Exiv2::ImageFactory::create(Exiv2::ImageType::xmp, (const char*)(QFile::encodeName(filePath).constData()));
        return saveOperations(finfo, image);
    }
    catch( Exiv2::Error& e )
    {
        printExiv2ExceptionError(QString::fromLatin1("Cannot save metadata to XMP sidecar using Exiv2 "), e);
        return false;
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
        return false;
    }
}

bool MetaEngine::Private::saveToFile(const QFileInfo& finfo) const
{
    if (!finfo.isWritable())
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "File" << finfo.fileName() << "is read only. Metadata not written.";
        return false;
    }

    QStringList rawTiffBasedSupported, rawTiffBasedNotSupported;

    // Raw files supported by Exiv2 0.21
    rawTiffBasedSupported << QString::fromLatin1("dng")
                          << QString::fromLatin1("nef")
                          << QString::fromLatin1("pef")
                          << QString::fromLatin1("orf")
                          << QString::fromLatin1("srw");

    if (Exiv2::testVersion(0,23,0))
    {
        rawTiffBasedSupported << QString::fromLatin1("cr2");
    }

    // Raw files not supported by Exiv2 0.21
    rawTiffBasedNotSupported << QString::fromLatin1("3fr")
                             << QString::fromLatin1("arw")
                             << QString::fromLatin1("dcr")
                             << QString::fromLatin1("erf")
                             << QString::fromLatin1("k25")
                             << QString::fromLatin1("kdc")
                             << QString::fromLatin1("mos")
                             << QString::fromLatin1("raw")
                             << QString::fromLatin1("sr2")
                             << QString::fromLatin1("srf")
                             << QString::fromLatin1("rw2");

    if (!Exiv2::testVersion(0,23,0))
    {
        rawTiffBasedNotSupported << QString::fromLatin1("cr2");
    }

    QString ext = finfo.suffix().toLower();

    if (!writeRawFiles && (rawTiffBasedSupported.contains(ext) || rawTiffBasedNotSupported.contains(ext)) )
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << finfo.fileName()
                                        << "is a TIFF based RAW file, "
                                        << "writing to such a file is disabled by current settings.";
        return false;
    }

/*
    if (rawTiffBasedNotSupported.contains(ext))
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << finfo.fileName()
                               << "is TIFF based RAW file not yet supported. Metadata not saved.";
        return false;
    }

    if (rawTiffBasedSupported.contains(ext) && !writeRawFiles)
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << finfo.fileName()
                               << "is TIFF based RAW file supported but writing mode is disabled. "
                               << "Metadata not saved.";
        return false;
    }

    qCDebug(DIGIKAM_METAENGINE_LOG) << "File Extension: " << ext << " is supported for writing mode";

    bool ret = false;
*/

    try
    {
        Exiv2::Image::AutoPtr image;
        image = Exiv2::ImageFactory::open((const char*)(QFile::encodeName(finfo.filePath()).constData()));
        return saveOperations(finfo, image);
    }
    catch( Exiv2::Error& e )
    {
        printExiv2ExceptionError(QString::fromLatin1("Cannot save metadata to image using Exiv2 "), e);
        return false;
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
        return false;
    }
}

bool MetaEngine::Private::saveOperations(const QFileInfo& finfo, Exiv2::Image::AutoPtr image) const
{
    try
    {
        Exiv2::AccessMode mode;
        bool wroteComment = false;
        bool wroteEXIF    = false;
        bool wroteIPTC    = false;
        bool wroteXMP     = false;

        // We need to load target file metadata to merge with new one. It's mandatory with TIFF format:
        // like all tiff file structure is based on Exif.
        image->readMetadata();

        // Image Comments ---------------------------------

        mode = image->checkMode(Exiv2::mdComment);

        if ((mode == Exiv2::amWrite) || (mode == Exiv2::amReadWrite))
        {
            image->setComment(imageComments());
            wroteComment = true;
        }

        qCDebug(DIGIKAM_METAENGINE_LOG) << "wroteComment: " << wroteComment;

        // Exif metadata ----------------------------------

        mode = image->checkMode(Exiv2::mdExif);

        if ((mode == Exiv2::amWrite) || (mode == Exiv2::amReadWrite))
        {
            if (image->mimeType() == "image/tiff")
            {
                Exiv2::ExifData orgExif = image->exifData();
                Exiv2::ExifData newExif;
                QStringList     untouchedTags;

                // With tiff image we cannot overwrite whole Exif data as well, because
                // image data are stored in Exif container. We need to take a care about
                // to not lost image data.
                untouchedTags << QString::fromLatin1("Exif.Image.ImageWidth");
                untouchedTags << QString::fromLatin1("Exif.Image.ImageLength");
                untouchedTags << QString::fromLatin1("Exif.Image.BitsPerSample");
                untouchedTags << QString::fromLatin1("Exif.Image.Compression");
                untouchedTags << QString::fromLatin1("Exif.Image.PhotometricInterpretation");
                untouchedTags << QString::fromLatin1("Exif.Image.FillOrder");
                untouchedTags << QString::fromLatin1("Exif.Image.SamplesPerPixel");
                untouchedTags << QString::fromLatin1("Exif.Image.StripOffsets");
                untouchedTags << QString::fromLatin1("Exif.Image.RowsPerStrip");
                untouchedTags << QString::fromLatin1("Exif.Image.StripByteCounts");
                untouchedTags << QString::fromLatin1("Exif.Image.XResolution");
                untouchedTags << QString::fromLatin1("Exif.Image.YResolution");
                untouchedTags << QString::fromLatin1("Exif.Image.PlanarConfiguration");
                untouchedTags << QString::fromLatin1("Exif.Image.ResolutionUnit");

                for (Exiv2::ExifData::const_iterator it = orgExif.begin(); it != orgExif.end(); ++it)
                {
                    if (untouchedTags.contains(QString::fromLatin1(it->key().c_str())))
                    {
                        newExif[it->key().c_str()] = orgExif[it->key().c_str()];
                    }
                }

                Exiv2::ExifData readedExif = exifMetadata();

                for (Exiv2::ExifData::const_iterator it = readedExif.begin(); it != readedExif.end(); ++it)
                {
                    if (!untouchedTags.contains(QString::fromLatin1(it->key().c_str())))
                    {
                        newExif[it->key().c_str()] = readedExif[it->key().c_str()];
                    }
                }

                image->setExifData(newExif);
            }
            else
            {
                image->setExifData(exifMetadata());
            }

            wroteEXIF = true;
        }

        qCDebug(DIGIKAM_METAENGINE_LOG) << "wroteEXIF: " << wroteEXIF;

        // Iptc metadata ----------------------------------

        mode = image->checkMode(Exiv2::mdIptc);

        if ((mode == Exiv2::amWrite) || (mode == Exiv2::amReadWrite))
        {
            image->setIptcData(iptcMetadata());
            wroteIPTC = true;
        }

        qCDebug(DIGIKAM_METAENGINE_LOG) << "wroteIPTC: " << wroteIPTC;

        // Xmp metadata -----------------------------------

        mode = image->checkMode(Exiv2::mdXmp);

        if ((mode == Exiv2::amWrite) || (mode == Exiv2::amReadWrite))
        {
#ifdef _XMP_SUPPORT_
            image->setXmpData(xmpMetadata());
            wroteXMP = true;
#endif
        }

        qCDebug(DIGIKAM_METAENGINE_LOG) << "wroteXMP: " << wroteXMP;

        if (!wroteComment && !wroteEXIF && !wroteIPTC && !wroteXMP)
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Writing metadata is not supported for file" << finfo.fileName();
            return false;
        }
        else if (!wroteEXIF || !wroteIPTC || !wroteXMP)
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Support for writing metadata is limited for file" << finfo.fileName();
        }

        if (!updateFileTimeStamp)
        {
            // Don't touch access and modification timestamp of file.
            struct stat    st;
            struct utimbuf ut;
            int ret = ::stat(QFile::encodeName(filePath).constData(), &st);

            if (ret == 0)
            {
                ut.modtime = st.st_mtime;
                ut.actime  = st.st_atime;
            }

            image->writeMetadata();

            if (ret == 0)
            {
                ::utime(QFile::encodeName(filePath).constData(), &ut);
            }

            qCDebug(DIGIKAM_METAENGINE_LOG) << "File time stamp restored";
        }
        else
        {
            image->writeMetadata();
        }

        return true;
    }
    catch( Exiv2::Error& e )
    {
        printExiv2ExceptionError(QString::fromLatin1("Cannot save metadata using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

void MetaEngineData::Private::clear()
{
    imageComments.clear();
    exifMetadata.clear();
    iptcMetadata.clear();
#ifdef _XMP_SUPPORT_
    xmpMetadata.clear();
#endif
}

void MetaEngine::Private::printExiv2ExceptionError(const QString& msg, Exiv2::Error& e)
{
    std::string s(e.what());
    qCCritical(DIGIKAM_METAENGINE_LOG) << msg.toLatin1().constData()
                                       << " (Error #" << e.code() << ": " << s.c_str();
}

void MetaEngine::Private::printExiv2MessageHandler(int lvl, const char* msg)
{
    qCDebug(DIGIKAM_METAENGINE_LOG) << "Exiv2 (" << lvl << ") : " << msg;
}

QString MetaEngine::Private::convertCommentValue(const Exiv2::Exifdatum& exifDatum) const
{
    try
    {
        std::string comment;
        std::string charset;

        comment = exifDatum.toString();

        // libexiv2 will prepend "charset=\"SomeCharset\" " if charset is specified
        // Before conversion to QString, we must know the charset, so we stay with std::string for a while
        if (comment.length() > 8 && comment.substr(0, 8) == "charset=")
        {
            // the prepended charset specification is followed by a blank
            std::string::size_type pos = comment.find_first_of(' ');

            if (pos != std::string::npos)
            {
                // extract string between the = and the blank
                charset = comment.substr(8, pos-8);
                // get the rest of the string after the charset specification
                comment = comment.substr(pos+1);
            }
        }

        if (charset == "\"Unicode\"")
        {
            return QString::fromUtf8(comment.data());
        }
        else if (charset == "\"Jis\"")
        {
            QTextCodec* const codec = QTextCodec::codecForName("JIS7");

            if (codec)
            {
                const char* tmp = comment.c_str();

                if (tmp)
                {
                    return codec->toUnicode(tmp);
                }
            }

            return QLatin1String("");
        }
        else if (charset == "\"Ascii\"")
        {
            return QString::fromLatin1(comment.c_str());
        }
        else
        {
            return detectEncodingAndDecode(comment);
        }
    }
    catch( Exiv2::Error& e )
    {
        printExiv2ExceptionError(QString::fromLatin1("Cannot convert Comment using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QString();
}

QString MetaEngine::Private::detectEncodingAndDecode(const std::string& value) const
{
    // For charset autodetection, we could use sophisticated code
    // (Mozilla chardet, KHTML's autodetection, QTextCodec::codecForContent),
    // but that is probably too much.
    // We check for UTF8, Local encoding and ASCII.
    // Look like KEncodingDetector class can provide a full implementation for encoding detection.

    if (value.empty())
    {
        return QString();
    }

    if (isUtf8(value.c_str()))
    {
        return QString::fromUtf8(value.c_str());
    }

    // Utf8 has a pretty unique byte pattern.
    // Thats not true for ASCII, it is not possible
    // to reliably autodetect different ISO-8859 charsets.
    // So we can use either local encoding, or latin1.

    return QString::fromLocal8Bit(value.c_str());
}

bool MetaEngine::Private::isUtf8(const char* const buffer) const
{
    int i, n;
    unsigned char c;
    bool gotone = false;

    if (!buffer)
    {
        return true;
    }

// character never appears in text
#define F 0
// character appears in plain ASCII text
#define T 1
// character appears in ISO-8859 text
#define I 2
// character appears in non-ISO extended ASCII (Mac, IBM PC)
#define X 3

    static const unsigned char text_chars[256] =
    {
        //                  BEL BS HT LF    FF CR
        F, F, F, F, F, F, F, T, T, T, T, F, T, T, F, F,  // 0x0X
        //                              ESC
        F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F,  // 0x1X
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  // 0x2X
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  // 0x3X
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  // 0x4X
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  // 0x5X
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  // 0x6X
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, F,  // 0x7X
        //            NEL
        X, X, X, X, X, T, X, X, X, X, X, X, X, X, X, X,  // 0x8X
        X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,  // 0x9X
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  // 0xaX
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  // 0xbX
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  // 0xcX
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  // 0xdX
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  // 0xeX
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I   // 0xfX
    };

    for (i = 0 ; (c = buffer[i]) ; ++i)
    {
        if ((c & 0x80) == 0)
        {
            // 0xxxxxxx is plain ASCII

            // Even if the whole file is valid UTF-8 sequences,
            // still reject it if it uses weird control characters.

            if (text_chars[c] != T)
            {
                return false;
            }
        }
        else if ((c & 0x40) == 0)
        {
            // 10xxxxxx never 1st byte
            return false;
        }
        else
        {
            // 11xxxxxx begins UTF-8
            int following = 0;

            if ((c & 0x20) == 0)
            {
                // 110xxxxx
                following = 1;
            }
            else if ((c & 0x10) == 0)
            {
                // 1110xxxx
                following = 2;
            }
            else if ((c & 0x08) == 0)
            {
                // 11110xxx
                following = 3;
            }
            else if ((c & 0x04) == 0)
            {
                // 111110xx
                following = 4;
            }
            else if ((c & 0x02) == 0)
            {
                // 1111110x
                following = 5;
            }
            else
            {
                return false;
            }

            for (n = 0 ; n < following ; ++n)
            {
                i++;

                if (!(c = buffer[i]))
                {
                    goto done;
                }

                if ((c & 0x80) == 0 || (c & 0x40))
                {
                    return false;
                }
            }

            gotone = true;
        }
    }

done:

    return gotone;   // don't claim it's UTF-8 if it's all 7-bit.
}

#undef F
#undef T
#undef I
#undef X

int MetaEngine::Private::getXMPTagsListFromPrefix(const QString& pf, MetaEngine::TagsMap& tagsMap) const
{
    int i = 0;

#ifdef _XMP_SUPPORT_

    try
    {
        QList<const Exiv2::XmpPropertyInfo*> tags;
        tags << Exiv2::XmpProperties::propertyList(pf.toLatin1().data());

        for (QList<const Exiv2::XmpPropertyInfo*>::iterator it = tags.begin(); it != tags.end(); ++it)
        {
            while ( (*it) && !QString::fromLatin1((*it)->name_).isNull() )
            {
                QString     key = QLatin1String( Exiv2::XmpKey( pf.toLatin1().data(), (*it)->name_ ).key().c_str() );
                QStringList values;
                values << QString::fromLatin1((*it)->name_)
                       << QString::fromLatin1((*it)->title_)
                       << QString::fromLatin1((*it)->desc_);
                tagsMap.insert(key, values);
                ++(*it);
                i++;
            }
        }
    }
    catch( Exiv2::Error& e )
    {
        printExiv2ExceptionError(QString::fromLatin1("Cannot get Xmp tags list using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(pf);
    Q_UNUSED(tagsMap);

#endif // _XMP_SUPPORT_

    return i;
}

#ifdef _XMP_SUPPORT_
void MetaEngine::Private::loadSidecarData(Exiv2::Image::AutoPtr xmpsidecar)
{
    // Having a sidecar is a special situation.
    // The sidecar data often "dominates", see in particular bug 309058 for important aspects:
    // If a field is removed from the sidecar, we must ignore (older) data for this field in the file.

    // First: Ignore file XMP, only use sidecar XMP
    xmpMetadata()     = xmpsidecar->xmpData();
    loadedFromSidecar = true;

    // EXIF
    // Four groups of properties are mapped between EXIF and XMP:
    // Date/Time, Description, Copyright, Creator
    // A few more tags are defined "writeback" tags in the XMP specification, the sidecar value therefore overrides the Exif value.
    // The rest is kept side-by-side.
    // (to understand, remember that the xmpsidecar's Exif data is actually XMP data mapped back to Exif)

    // Description, Copyright and Creator is dominated by the sidecar: Remove file Exif fields, if field not in XMP.
    ExifMergeHelper exifDominatedHelper;
    exifDominatedHelper << QLatin1String("Exif.Image.ImageDescription")
                        << QLatin1String("Exif.Photo.UserComment")
                        << QLatin1String("Exif.Image.Copyright")
                        << QLatin1String("Exif.Image.Artist");
    exifDominatedHelper.exclusiveMerge(xmpsidecar->exifData(), exifMetadata());
    // Date/Time and "the few more" from the XMP spec are handled as writeback
    // Note that Date/Time mapping is slightly contradictory in latest specs.
    ExifMergeHelper exifWritebackHelper;
    exifWritebackHelper << QLatin1String("Exif.Image.DateTime")
                        << QLatin1String("Exif.Image.DateTime")
                        << QLatin1String("Exif.Photo.DateTimeOriginal")
                        << QLatin1String("Exif.Photo.DateTimeDigitized")
                        << QLatin1String("Exif.Image.Orientation")
                        << QLatin1String("Exif.Image.XResolution")
                        << QLatin1String("Exif.Image.YResolution")
                        << QLatin1String("Exif.Image.ResolutionUnit")
                        << QLatin1String("Exif.Image.Software")
                        << QLatin1String("Exif.Photo.RelatedSoundFile");
    exifWritebackHelper.mergeFields(xmpsidecar->exifData(), exifMetadata());

    // IPTC
    // These fields cover almost all relevant IPTC data and are defined in the XMP specification for reconciliation.
    IptcMergeHelper iptcDominatedHelper;
    iptcDominatedHelper << QLatin1String("Iptc.Application2.ObjectName")
                        << QLatin1String("Iptc.Application2.Urgency")
                        << QLatin1String("Iptc.Application2.Category")
                        << QLatin1String("Iptc.Application2.SuppCategory")
                        << QLatin1String("Iptc.Application2.Keywords")
                        << QLatin1String("Iptc.Application2.SubLocation")
                        << QLatin1String("Iptc.Application2.SpecialInstructions")
                        << QLatin1String("Iptc.Application2.Byline")
                        << QLatin1String("Iptc.Application2.BylineTitle")
                        << QLatin1String("Iptc.Application2.City")
                        << QLatin1String("Iptc.Application2.ProvinceState")
                        << QLatin1String("Iptc.Application2.CountryCode")
                        << QLatin1String("Iptc.Application2.CountryName")
                        << QLatin1String("Iptc.Application2.TransmissionReference")
                        << QLatin1String("Iptc.Application2.Headline")
                        << QLatin1String("Iptc.Application2.Credit")
                        << QLatin1String("Iptc.Application2.Source")
                        << QLatin1String("Iptc.Application2.Copyright")
                        << QLatin1String("Iptc.Application2.Caption")
                        << QLatin1String("Iptc.Application2.Writer");
    iptcDominatedHelper.exclusiveMerge(xmpsidecar->iptcData(), iptcMetadata());

    IptcMergeHelper iptcWritebackHelper;
    iptcWritebackHelper << QLatin1String("Iptc.Application2.DateCreated")
                        << QLatin1String("Iptc.Application2.TimeCreated")
                        << QLatin1String("Iptc.Application2.DigitizationDate")
                        << QLatin1String("Iptc.Application2.DigitizationTime");
    iptcWritebackHelper.mergeFields(xmpsidecar->iptcData(), iptcMetadata());

    /*
     * TODO: Exiv2 (referring to 0.23) does not correctly synchronize all times values as given below.
     * Time values and their synchronization:
     * Original Date/Time – Creation date of the intellectual content (e.g. the photograph),
       rather than the creatio*n date of the content being shown
        Exif DateTimeOriginal (36867, 0x9003) and SubSecTimeOriginal (37521, 0x9291)
        IPTC DateCreated (IIM 2:55, 0x0237) and TimeCreated (IIM 2:60, 0x023C)
        XMP (photoshop:DateCreated)
     * Digitized Date/Time – Creation date of the digital representation
        Exif DateTimeDigitized (36868, 0x9004) and SubSecTimeDigitized (37522, 0x9292)
        IPTC DigitalCreationDate (IIM 2:62, 0x023E) and DigitalCreationTime (IIM 2:63, 0x023F)
        XMP (xmp:CreateDate)
     * Modification Date/Time – Modification date of the digital image file
        Exif DateTime (306, 0x132) and SubSecTime (37520, 0x9290)
        XMP (xmp:ModifyDate)
     */
}
#endif // _XMP_SUPPORT_

} // namespace Digikam

// Restore warnings
#if !defined(__APPLE__) && defined(__GNUC__)
#   pragma GCC diagnostic pop
#endif

#if defined(__APPLE__) && defined(__clang__)
#   pragma clang diagnostic pop
#endif
