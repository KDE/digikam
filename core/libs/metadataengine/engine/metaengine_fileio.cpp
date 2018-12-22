/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-09-15
 * Description : Exiv2 library interface.
 *               File I/O methods
 *
 * Copyright (C) 2006-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

void MetaEngine::setFilePath(const QString& path)
{
    d->filePath = path;
}

QString MetaEngine::getFilePath() const
{
    return d->filePath;
}

QString MetaEngine::sidecarFilePathForFile(const QString& path)
{
    if (path.isEmpty())
    {
        return QString();
    }

    QFileInfo info(path);
    QString pathForLR = path;
    pathForLR.chop(info.suffix().size());
    pathForLR.append(QLatin1String("xmp"));

    if (QFileInfo::exists(pathForLR))
    {
        return pathForLR;
    }

    return path + QLatin1String(".xmp");
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
    return QFileInfo::exists(sidecarFilePathForFile(path));
}

bool MetaEngine::load(const QString& filePath)
{
    if (filePath.isEmpty())
    {
        return false;
    }

    d->filePath      = filePath;
    bool hasLoaded   = false;

    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        Exiv2::Image::AutoPtr image;

        image        = Exiv2::ImageFactory::open((const char*)(QFile::encodeName(filePath)).constData());

        image->readMetadata();

        // Size and mimetype ---------------------------------

        d->pixelSize = QSize(image->pixelWidth(), image->pixelHeight());
        d->mimeType  = QLatin1String(image->mimeType().c_str());

        // Image comments ---------------------------------

        d->itemComments() = image->comment();

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
        d->printExiv2ExceptionError(QString::fromUtf8("Cannot load metadata from file %1").arg(getFilePath()), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    hasLoaded |= loadFromSidecarAndMerge(filePath);

    return hasLoaded;
}

bool MetaEngine::loadFromSidecarAndMerge(const QString& filePath)
{
    if (filePath.isEmpty())
    {
        return false;
    }

    d->filePath    = filePath;
    bool hasLoaded = false;

#ifdef _XMP_SUPPORT_

    QMutexLocker lock(&s_metaEngineMutex);

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
        d->printExiv2ExceptionError(QString::fromUtf8("Cannot load XMP sidecar from file %1").arg(getFilePath()), e);
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
        case WRITE_TO_SIDECAR_ONLY:
            writeToSidecar = true;
            break;
        case WRITE_TO_FILE_ONLY:
            writeToFile    = true;
            break;
        case WRITE_TO_SIDECAR_AND_FILE:
            writeToFile    = true;
            writeToSidecar = true;
            break;
        case WRITE_TO_SIDECAR_ONLY_FOR_READ_ONLY_FILES:
            writeToFile = true;
            writeToSidecarIfFileNotPossible = true;
            break;
    }

    if (writeToFile)
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Will write Metadata to file" << finfo.absoluteFilePath();
        writtenToFile = d->saveToFile(finfo);

        if (writtenToFile)
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

bool MetaEngine::applyChanges(bool setVersion) const
{
    if (d->filePath.isEmpty())
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Failed to apply changes: file path is empty!";
        return false;
    }

    return save(d->filePath, setVersion);
}

} // namespace Digikam
