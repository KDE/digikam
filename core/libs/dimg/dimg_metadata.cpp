/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : digiKam 8/16 bits image management API.
 *               Metadata operations.
 *
 * Copyright (C) 2005-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dimg_p.h"

namespace Digikam
{

QByteArray DImg::getUniqueHash() const
{
    if (m_priv->attributes.contains(QLatin1String("uniqueHash")))
    {
        return m_priv->attributes[QLatin1String("uniqueHash")].toByteArray();
    }

    if (!m_priv->attributes.contains(QLatin1String("originalFilePath")))
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "DImg::getUniqueHash called without originalFilePath property set!";
        return QByteArray();
    }

    QString filePath = m_priv->attributes.value(QLatin1String("originalFilePath")).toString();

    if (filePath.isEmpty())
    {
        return QByteArray();
    }

    FileReadLocker lock(filePath);
    QByteArray hash = DImgLoader::uniqueHash(filePath, *this, false);

    // attribute is written by DImgLoader

    return hash;
}

QByteArray DImg::getUniqueHash(const QString& filePath)
{
    return DImgLoader::uniqueHash(filePath, DImg(), true);
}

QByteArray DImg::getUniqueHashV2() const
{
    if (m_priv->attributes.contains(QLatin1String("uniqueHashV2")))
    {
        return m_priv->attributes[QLatin1String("uniqueHashV2")].toByteArray();
    }

    if (!m_priv->attributes.contains(QLatin1String("originalFilePath")))
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "DImg::getUniqueHash called without originalFilePath property set!";
        return QByteArray();
    }

    QString filePath = m_priv->attributes.value(QLatin1String("originalFilePath")).toString();

    if (filePath.isEmpty())
    {
        return QByteArray();
    }

    FileReadLocker lock(filePath);

    return DImgLoader::uniqueHashV2(filePath, this);
}

QByteArray DImg::getUniqueHashV2(const QString& filePath)
{
    return DImgLoader::uniqueHashV2(filePath);
}

QByteArray DImg::createImageUniqueId() const
{
    NonDeterministicRandomData randomData(16);
    QByteArray imageUUID = randomData.toHex();
    imageUUID           += getUniqueHashV2();

    return imageUUID;
}

void DImg::prepareMetadataToSave(const QString& intendedDestPath, const QString& destMimeType,
                                 bool resetExifOrientationTag)
{
    PrepareMetadataFlags flags = PrepareMetadataFlagsAll;

    if (!resetExifOrientationTag)
    {
        flags &= ~ResetExifOrientationTag;
    }

    QUrl url = QUrl::fromLocalFile(originalFilePath());
    prepareMetadataToSave(intendedDestPath, destMimeType, url.fileName(), flags);
}

void DImg::prepareMetadataToSave(const QString& intendedDestPath, const QString& destMimeType,
                                 const QString& originalFileName, PrepareMetadataFlags flags)
{
    if (isNull())
    {
        return;
    }

    // Get image Exif/IPTC data.
    DMetadata meta(getMetadata());

    if (flags & RemoveOldMetadataPreviews || flags & CreateNewMetadataPreview)
    {
        // Clear IPTC preview
        meta.removeIptcTag("Iptc.Application2.Preview");
        meta.removeIptcTag("Iptc.Application2.PreviewFormat");
        meta.removeIptcTag("Iptc.Application2.PreviewVersion");

        // Clear Exif thumbnail
        meta.removeExifThumbnail();

        // Clear Tiff thumbnail
        MetaEngine::MetaDataMap tiffThumbTags = meta.getExifTagsDataList(QStringList() << QLatin1String("SubImage1"));

        for (MetaEngine::MetaDataMap::iterator it = tiffThumbTags.begin(); it != tiffThumbTags.end(); ++it)
        {
            meta.removeExifTag(it.key().toLatin1().constData());
        }
    }

    bool createNewPreview    = false;
    QSize previewSize;

    // Refuse preview creation for images with transparency
    // as long as we have no format to support this. See bug 286127
    bool skipPreviewCreation = hasTransparentPixels();

    if (flags & CreateNewMetadataPreview && !skipPreviewCreation)
    {
        const QSize standardPreviewSize(1280, 1280);
        previewSize = size();

        // Scale to standard preview size. Only scale down, not up
        if (width() > (uint)standardPreviewSize.width() && height() > (uint)standardPreviewSize.height())
        {
            previewSize.scale(standardPreviewSize, Qt::KeepAspectRatio);
        }

        // Only store a new preview if it is worth it - the original should be significantly larger than the preview
        createNewPreview = (2 * (uint)previewSize.width() <= width());
    }

    if (createNewPreview)
    {
        // Create the preview QImage
        QImage preview;
        {
            if (!IccManager::isSRGB(*this))
            {
                DImg previewDImg;

                if (previewSize.width() >= (int)width())
                {
                    previewDImg = copy();
                }
                else
                {
                    previewDImg = smoothScale(previewSize.width(), previewSize.height(), Qt::IgnoreAspectRatio);
                }

                IccManager manager(previewDImg);
                manager.transformToSRGB();
                preview = previewDImg.copyQImage();
            }
            else
            {
                // Ensure that preview is not upscaled
                if (previewSize.width() >= (int)width())
                {
                    preview = copyQImage();
                }
                else
                {
                    preview = smoothScale(previewSize.width(), previewSize.height(), Qt::IgnoreAspectRatio).copyQImage();
                }
            }
        }

        // Update IPTC preview.
        // see bug #130525. a JPEG segment is limited to 64K. If the IPTC byte array is
        // bigger than 64K during of image preview tag size, the target JPEG image will be
        // broken. Note that IPTC image preview tag is limited to 256K!!!
        // There is no limitation with TIFF and PNG about IPTC byte array size.
        // So for a JPEG file, we don't store the IPTC preview.
        if ((destMimeType.toUpper() != QLatin1String("JPG") && destMimeType.toUpper() != QLatin1String("JPEG") &&
             destMimeType.toUpper() != QLatin1String("JPE"))
           )
        {
            // Non JPEG file, we update IPTC preview
            meta.setItemPreview(preview);
        }

        if (destMimeType.toUpper() == QLatin1String("TIFF") || destMimeType.toUpper() == QLatin1String("TIF"))
        {
            // With TIFF file, we don't store JPEG thumbnail, we even need to erase it and store
            // a thumbnail at a special location. See bug #211758
            QImage thumb = preview.scaled(160, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            meta.setTiffThumbnail(thumb);
        }
        else
        {
            // Update Exif thumbnail.
            QImage thumb = preview.scaled(160, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            meta.setExifThumbnail(thumb);
        }
    }

    // Update Exif Image dimensions.
    meta.setItemDimensions(size());

    // Update Exif Document Name tag with the original file name.
    if (!originalFileName.isEmpty())
    {
        meta.setExifTagString("Exif.Image.DocumentName", originalFileName);
    }

    // Update Exif Orientation tag if necessary.
    if (flags & ResetExifOrientationTag)
    {
        meta.setItemOrientation(DMetadata::ORIENTATION_NORMAL);
    }

    if (!m_priv->imageHistory.isEmpty())
    {
        DImageHistory forSaving(m_priv->imageHistory);
        forSaving.adjustReferredImages();

        QUrl url         = QUrl::fromLocalFile(intendedDestPath);
        QString filePath = url.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash).toLocalFile() + QLatin1Char('/');
        QString fileName = url.fileName();

        if (!filePath.isEmpty() && !fileName.isEmpty())
        {
            forSaving.purgePathFromReferredImages(filePath, fileName);
        }

        QString imageHistoryXml = forSaving.toXml();
        meta.setItemHistory(imageHistoryXml);
    }

    if (flags & CreateNewImageHistoryUUID)
    {
        meta.setItemUniqueId(QString::fromUtf8(createImageUniqueId()));
    }

    // Store new Exif/IPTC/XMP data into image.
    setMetadata(meta.data());
}

HistoryImageId DImg::createHistoryImageId(const QString& filePath, HistoryImageId::Type type) const
{
    HistoryImageId id = DImgLoader::createHistoryImageId(filePath, *this, DMetadata(getMetadata()));
    id.setType(type);

    return id;
}

HistoryImageId DImg::addAsReferredImage(const QString& filePath, HistoryImageId::Type type)
{
    HistoryImageId id = createHistoryImageId(filePath, type);
    m_priv->imageHistory.purgePathFromReferredImages(id.path(), id.fileName());
    addAsReferredImage(id);

    return id;
}

void DImg::addAsReferredImage(const HistoryImageId& id)
{
    m_priv->imageHistory << id;
}

void DImg::insertAsReferredImage(int afterHistoryStep, const HistoryImageId& id)
{
    m_priv->imageHistory.insertReferredImage(afterHistoryStep, id);
}

void DImg::addCurrentUniqueImageId(const QString& uuid)
{
    m_priv->imageHistory.adjustCurrentUuid(uuid);
}

void DImg::addFilterAction(const Digikam::FilterAction& action)
{
    m_priv->imageHistory << action;
}

const DImageHistory& DImg::getItemHistory() const
{
    return m_priv->imageHistory;
}

DImageHistory& DImg::getItemHistory()
{
    return m_priv->imageHistory;
}

void DImg::setItemHistory(const DImageHistory& history)
{
    m_priv->imageHistory = history;
}

bool DImg::hasImageHistory() const
{
    if (m_priv->imageHistory.isEmpty())
    {
        return false;
    }
    else
    {
        return true;
    }
}

DImageHistory DImg::getOriginalImageHistory() const
{
    return attribute(QLatin1String("originalImageHistory")).value<DImageHistory>();
}

void DImg::setHistoryBranch(bool isBranch)
{
    setHistoryBranchAfter(getOriginalImageHistory(), isBranch);
}

void DImg::setHistoryBranchAfter(const DImageHistory& historyBeforeBranch, bool isBranch)
{
    int addedSteps = m_priv->imageHistory.size() - historyBeforeBranch.size();
    setHistoryBranchForLastSteps(addedSteps, isBranch);
}

void DImg::setHistoryBranchForLastSteps(int numberOfLastHistorySteps, bool isBranch)
{
    int firstStep = m_priv->imageHistory.size() - numberOfLastHistorySteps;

    if (firstStep < m_priv->imageHistory.size())
    {
        if (isBranch)
        {
            m_priv->imageHistory[firstStep].action.addFlag(FilterAction::ExplicitBranch);
        }
        else
        {
            m_priv->imageHistory[firstStep].action.removeFlag(FilterAction::ExplicitBranch);
        }
    }
}

QString DImg::colorModelToString(COLORMODEL colorModel)
{
    switch (colorModel)
    {
        case RGB:
            return i18nc("Color Model: RGB", "RGB");

        case GRAYSCALE:
            return i18nc("Color Model: Grayscale", "Grayscale");

        case MONOCHROME:
            return i18nc("Color Model: Monochrome", "Monochrome");

        case INDEXED:
            return i18nc("Color Model: Indexed", "Indexed");

        case YCBCR:
            return i18nc("Color Model: YCbCr", "YCbCr");

        case CMYK:
            return i18nc("Color Model: CMYK", "CMYK");

        case CIELAB:
            return i18nc("Color Model: CIE L*a*b*", "CIE L*a*b*");

        case COLORMODELRAW:
            return i18nc("Color Model: Uncalibrated (RAW)", "Uncalibrated (RAW)");

        case COLORMODELUNKNOWN:
        default:
            return i18nc("Color Model: Unknown", "Unknown");
    }
}

bool DImg::isAnimatedImage(const QString& filePath)
{
    QImageReader reader(filePath);
    reader.setDecideFormatFromContent(true);

    if (reader.supportsAnimation() && 
       (reader.imageCount() > 1))
    {
        qDebug(DIGIKAM_DIMG_LOG_QIMAGE) << "File \"" << filePath << "\" is an animated image ";
        return true;
    }

    return false;
}

} // namespace Digikam
