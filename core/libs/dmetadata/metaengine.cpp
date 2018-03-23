/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-09-15
 * Description : Exiv2 library interface
 *               Exiv2: http://www.exiv2.org
 *               Exif : http://www.exif.org/Exif2-2.PDF
 *               Iptc : http://www.iptc.org/std/IIM/4.1/specification/IIMV4.1.pdf
 *               Xmp  : http://www.adobe.com/devnet/xmp/pdfs/xmp_specification.pdf
 *                      http://www.iptc.org/std/Iptc4xmpCore/1.0/specification/Iptc4xmpCore_1.0-spec-XMPSchema_8.pdf
 *               Paper: http://www.metadataworkinggroup.com/pdf/mwg_guidance.pdf
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

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"

namespace Digikam
{

MetaEngine::MetaEngine()
    : d(new Private)
{
}

MetaEngine::MetaEngine(const MetaEngine& metadata)
    : d(new Private)
{
    d->copyPrivateData(metadata.d);
}

MetaEngine::MetaEngine(const MetaEngineData& data)
    : d(new Private)
{
    setData(data);
}

MetaEngine::MetaEngine(const QString& filePath)
    : d(new Private)
{
    load(filePath);
}

MetaEngine::~MetaEngine()
{
    delete d;
}

MetaEngine& MetaEngine::operator=(const MetaEngine& metadata)
{
    d->copyPrivateData(metadata.d);

    return *this;
}

//-- Statics methods ----------------------------------------------

bool MetaEngine::initializeExiv2()
{
#ifdef _XMP_SUPPORT_

    if (!Exiv2::XmpParser::initialize())
        return false;

    registerXmpNameSpace(QString::fromLatin1("http://ns.adobe.com/lightroom/1.0/"),  QString::fromLatin1("lr"));
    registerXmpNameSpace(QString::fromLatin1("http://www.digikam.org/ns/kipi/1.0/"), QString::fromLatin1("kipi"));
    registerXmpNameSpace(QString::fromLatin1("http://ns.microsoft.com/photo/1.2/"),  QString::fromLatin1("MP"));
    registerXmpNameSpace(QString::fromLatin1("http://ns.acdsee.com/iptc/1.0/"),      QString::fromLatin1("acdsee"));
    registerXmpNameSpace(QString::fromLatin1("http://www.video"),                    QString::fromLatin1("video"));

#endif // _XMP_SUPPORT_

    return true;
}

bool MetaEngine::cleanupExiv2()
{
    // Fix memory leak if Exiv2 support XMP.
#ifdef _XMP_SUPPORT_

    unregisterXmpNameSpace(QString::fromLatin1("http://ns.adobe.com/lightroom/1.0/"));
    unregisterXmpNameSpace(QString::fromLatin1("http://www.digikam.org/ns/kipi/1.0/"));
    unregisterXmpNameSpace(QString::fromLatin1("http://ns.microsoft.com/photo/1.2/"));
    unregisterXmpNameSpace(QString::fromLatin1("http://ns.acdsee.com/iptc/1.0/"));
    unregisterXmpNameSpace(QString::fromLatin1("http://www.video"));

    Exiv2::XmpParser::terminate();

#endif // _XMP_SUPPORT_

    return true;
}

bool MetaEngine::supportXmp()
{
#ifdef _XMP_SUPPORT_
    return true;
#else
    return false;
#endif // _XMP_SUPPORT_
}

bool MetaEngine::supportMetadataWritting(const QString& typeMime)
{
    if (typeMime == QString::fromLatin1("image/jpeg"))
    {
        return true;
    }
    else if (typeMime == QString::fromLatin1("image/tiff"))
    {
        return true;
    }
    else if (typeMime == QString::fromLatin1("image/png"))
    {
        return true;
    }
    else if (typeMime == QString::fromLatin1("image/jp2"))
    {
        return true;
    }
    else if (typeMime == QString::fromLatin1("image/x-raw"))
    {
        return false;
    }
    else if (typeMime == QString::fromLatin1("image/pgf"))
    {
        return true;
    }

    return false;
}

QString MetaEngine::Exiv2Version()
{
    // Since 0.14.0 release, we can extract run-time version of Exiv2.
    // else we return make version.

    return QString::fromLatin1(Exiv2::version());
}

QString MetaEngine::sidecarFilePathForFile(const QString& path)
{
    QString ret;

    if (!path.isEmpty())
    {
        ret = path + QString::fromLatin1(".xmp");
    }

    return ret;
}

QUrl MetaEngine::sidecarUrl(const QUrl& url)
{
    return sidecarUrl(url.toLocalFile());
}

QUrl MetaEngine::sidecarUrl(const QString& path)
{
    return QUrl::fromLocalFile(sidecarFilePathForFile(path));
}

QString MetaEngine::sidecarPath(const QString& path)
{
    return sidecarFilePathForFile(path);
}

bool MetaEngine::hasSidecar(const QString& path)
{
    return QFileInfo(sidecarFilePathForFile(path)).exists();
}

//-- General methods ----------------------------------------------

MetaEngineData MetaEngine::data() const
{
    MetaEngineData data;
    data.d = d->data;
    return data;
}

void MetaEngine::setData(const MetaEngineData& data)
{
    if (data.d)
    {
        d->data = data.d;
    }
    else
    {
        // MetaEngineData can have a null pointer,
        // but we never want a null pointer in Private.
        d->data->clear();
    }
}

bool MetaEngine::loadFromData(const QByteArray& imgData)
{
    if (imgData.isEmpty())
        return false;

    try
    {
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open((Exiv2::byte*)imgData.data(), imgData.size());

        d->filePath.clear();
        image->readMetadata();

        // Size and mimetype ---------------------------------

        d->pixelSize = QSize(image->pixelWidth(), image->pixelHeight());
        d->mimeType  = QString::fromLatin1(image->mimeType().c_str());

        // Image comments ---------------------------------

        d->imageComments() = image->comment();

        // Exif metadata ----------------------------------

        d->exifMetadata() = image->exifData();

        // Iptc metadata ----------------------------------

        d->iptcMetadata() = image->iptcData();

#ifdef _XMP_SUPPORT_

        // Xmp metadata -----------------------------------

        d->xmpMetadata() = image->xmpData();

#endif // _XMP_SUPPORT_

        return true;
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot load metadata using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::load(const QString& filePath)
{
    if (filePath.isEmpty())
    {
        return false;
    }

    d->filePath      = filePath;
    bool hasLoaded   = false;

    try
    {
        Exiv2::Image::AutoPtr image;

        image        = Exiv2::ImageFactory::open((const char*)(QFile::encodeName(filePath)).constData());

        image->readMetadata();

        // Size and mimetype ---------------------------------

        d->pixelSize = QSize(image->pixelWidth(), image->pixelHeight());
        d->mimeType  = QString::fromLatin1(image->mimeType().c_str());

        // Image comments ---------------------------------

        d->imageComments() = image->comment();

        // Exif metadata ----------------------------------

        d->exifMetadata() = image->exifData();

        // Iptc metadata ----------------------------------

        d->iptcMetadata() = image->iptcData();

#ifdef _XMP_SUPPORT_

        // Xmp metadata -----------------------------------
        d->xmpMetadata() = image->xmpData();

#endif // _XMP_SUPPORT_

        hasLoaded = true;
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot load metadata from file "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#ifdef _XMP_SUPPORT_
    try
    {
        if (d->useXMPSidecar4Reading)
        {
            QString xmpSidecarPath = sidecarFilePathForFile(filePath);
            QFileInfo xmpSidecarFileInfo(xmpSidecarPath);

            Exiv2::Image::AutoPtr xmpsidecar;

            if (xmpSidecarFileInfo.exists() && xmpSidecarFileInfo.isReadable())
            {
                // Read sidecar data
                xmpsidecar = Exiv2::ImageFactory::open(QFile::encodeName(xmpSidecarPath).constData());
                xmpsidecar->readMetadata();

                // Merge
                d->loadSidecarData(xmpsidecar);
                hasLoaded = true;
            }
        }
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot load XMP sidecar"), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#endif // _XMP_SUPPORT_

    return hasLoaded;
}

bool MetaEngine::save(const QString& imageFilePath, bool setVersion) const
{
    if (setVersion && !setProgramId())
    {
        return false;
    }

    // If our image is really a symlink, we should follow the symlink so that
    // when we delete the file and rewrite it, we are honoring the symlink
    // (rather than just deleting it and putting a file there).

    // However, this may be surprising to the user when they are writing sidecar
    // files.  They might expect them to show up where the symlink is.  So, we
    // shouldn't follow the link when figuring out what the filename for the
    // sidecar should be.

    // Note, we are not yet handling the case where the sidecar itself is a
    // symlink.
    QString regularFilePath = imageFilePath; // imageFilePath might be a
                                             // symlink.  Below we will change
                                             // regularFile to the pointed to
                                             // file if so.
    QFileInfo givenFileInfo(imageFilePath);

    if (givenFileInfo.isSymLink())
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "filePath" << imageFilePath << "is a symlink."
                               << "Using target" << givenFileInfo.canonicalFilePath();

        regularFilePath = givenFileInfo.canonicalFilePath();// Walk all the symlinks
    }

    // NOTE: see B.K.O #137770 & #138540 : never touch the file if is read only.
    QFileInfo finfo(regularFilePath);
    QFileInfo dinfo(finfo.path());

    if (!dinfo.isWritable())
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Dir" << dinfo.filePath() << "is read-only. Metadata not saved.";
        return false;
    }

    bool writeToFile                     = false;
    bool writeToSidecar                  = false;
    bool writeToSidecarIfFileNotPossible = false;
    bool writtenToFile                   = false;
    bool writtenToSidecar                = false;

    qCDebug(DIGIKAM_METAENGINE_LOG) << "MetaEngine::metadataWritingMode" << d->metadataWritingMode;

    switch(d->metadataWritingMode)
    {
        case WRITETOSIDECARONLY:
            writeToSidecar = true;
            break;
        case WRITETOIMAGEONLY:
            writeToFile    = true;
            break;
        case WRITETOSIDECARANDIMAGE:
            writeToFile    = true;
            writeToSidecar = true;
            break;
        case WRITETOSIDECARONLY4READONLYFILES:
            writeToFile = true;
            writeToSidecarIfFileNotPossible = true;
            break;
    }

    if (writeToFile)
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Will write Metadata to file" << finfo.absoluteFilePath();
        writtenToFile = d->saveToFile(finfo);

        if (writeToFile)
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Metadata for file" << finfo.fileName() << "written to file.";
        }
    }

    if (writeToSidecar || (writeToSidecarIfFileNotPossible && !writtenToFile))
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Will write XMP sidecar for file" << finfo.fileName();
        writtenToSidecar = d->saveToXMPSidecar(regularFilePath);

        if (writtenToSidecar)
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Metadata for file" << finfo.fileName() << "written to XMP sidecar.";
        }
    }

    return writtenToFile || writtenToSidecar;
}

bool MetaEngine::applyChanges() const
{
    if (d->filePath.isEmpty())
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Failed to apply changes: file path is empty!";
        return false;
    }

    return save(d->filePath);
}

bool MetaEngine::isEmpty() const
{
    if (!hasComments() && !hasExif() && !hasIptc() && !hasXmp())
        return true;

    return false;
}

void MetaEngine::setFilePath(const QString& path)
{
    d->filePath = path;
}

QString MetaEngine::getFilePath() const
{
    return d->filePath;
}

QSize MetaEngine::getPixelSize() const
{
    return d->pixelSize;
}

QString MetaEngine::getMimeType() const
{
    return d->mimeType;
}

void MetaEngine::setWriteRawFiles(const bool /*on*/)
{
    //d->writeRawFiles = on;
}

bool MetaEngine::writeRawFiles() const
{
    return d->writeRawFiles;
}

void MetaEngine::setUseXMPSidecar4Reading(const bool on)
{
    d->useXMPSidecar4Reading = on;
}

bool MetaEngine::useXMPSidecar4Reading() const
{
    return d->useXMPSidecar4Reading;
}

void MetaEngine::setMetadataWritingMode(const int mode)
{
    d->metadataWritingMode = mode;
}

int MetaEngine::metadataWritingMode() const
{
    return d->metadataWritingMode;
}

void MetaEngine::setUpdateFileTimeStamp(bool on)
{
    d->updateFileTimeStamp = on;
}

bool MetaEngine::updateFileTimeStamp() const
{
    return d->updateFileTimeStamp;
}

bool MetaEngine::setProgramId() const
{
    QString version(digiKamVersion());
    QLatin1String software("digiKam");
    return setImageProgramId(software, version);
}

}  // namespace Digikam
