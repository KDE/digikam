/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-10
 * Description : album icon view tool tip
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "tooltipfiller.h"

// Qt includes

#include <QDateTime>

// KDE includes


#include <klocale.h>
#include <kfileitem.h>
#include <kglobal.h>
#include <kdeversion.h>

// Local includes

#include "albummanager.h"
#include "albumsettings.h"
#include "album.h"
#include "ditemtooltip.h"
#include "imageinfo.h"

namespace Digikam
{

QString ToolTipFiller::imageInfoTipContents(const ImageInfo& info)
{
    QString            str;
    AlbumSettings*     settings = AlbumSettings::instance();
    DToolTipStyleSheet cnt(settings->getToolTipsFont());

    ImageCommonContainer commonInfo  = info.imageCommonContainer();
    ImageMetadataContainer photoInfo = info.imageMetadataContainer();
    QString tip                      = cnt.tipHeader;

    // -- File properties ----------------------------------------------

    if (settings->getToolTipsShowFileName()  ||
        settings->getToolTipsShowFileDate()  ||
        settings->getToolTipsShowFileSize()  ||
        settings->getToolTipsShowImageType() ||
        settings->getToolTipsShowImageDim())
    {
        tip += cnt.headBeg + i18n("File Properties") + cnt.headEnd;

        if (settings->getToolTipsShowFileName())
        {
            tip += cnt.cellBeg + i18nc("filename",
                                       "Name:") + cnt.cellMid;
            tip += commonInfo.fileName + cnt.cellEnd;
        }

        if (settings->getToolTipsShowFileDate())
        {
            QDateTime modifiedDate = commonInfo.fileModificationDate;
            str = KGlobal::locale()->formatDateTime(modifiedDate, KLocale::ShortDate, true);
            tip += cnt.cellBeg + i18n("Date:") + cnt.cellMid + str + cnt.cellEnd;
        }

        if (settings->getToolTipsShowFileSize())
        {
            tip += cnt.cellBeg + i18n("Size:") + cnt.cellMid;
            QString localeFileSize = KGlobal::locale()->formatNumber(commonInfo.fileSize, 0);
            str = i18n("%1 (%2)", KIO::convertSize(commonInfo.fileSize), localeFileSize);
            tip += str + cnt.cellEnd;
        }

        QSize dims;

        if (settings->getToolTipsShowImageType())
        {
            tip += cnt.cellBeg + i18n("Type:") + cnt.cellMid + commonInfo.format + cnt.cellEnd;
        }

        if (settings->getToolTipsShowImageDim())
        {
            if (commonInfo.width == 0 || commonInfo.height == 0)
            {
                str = i18nc("unknown / invalid image dimension",
                            "Unknown");
            }
            else
            {
                QString mpixels;
                mpixels.setNum(commonInfo.width*commonInfo.height/1000000.0, 'f', 2);
                str = i18nc("width x height (megapixels Mpx)", "%1x%2 (%3Mpx)",
                            commonInfo.width, commonInfo.height, mpixels);
            }
            tip += cnt.cellBeg + i18n("Dimensions:") + cnt.cellMid + str + cnt.cellEnd;
        }
    }

    // -- Photograph Info ----------------------------------------------------

    if (settings->getToolTipsShowPhotoMake()  ||
        settings->getToolTipsShowPhotoDate()  ||
        settings->getToolTipsShowPhotoFocal() ||
        settings->getToolTipsShowPhotoExpo()  ||
        settings->getToolTipsShowPhotoMode()  ||
        settings->getToolTipsShowPhotoFlash() ||
        settings->getToolTipsShowPhotoWB())
    {
        if (!photoInfo.allFieldsNull)
        {
            QString metaStr;
            tip += cnt.headBeg + i18n("Photograph Properties") + cnt.headEnd;

            if (settings->getToolTipsShowPhotoMake())
            {
                str = QString("%1 / %2").arg(photoInfo.make.isEmpty() ? cnt.unavailable : photoInfo.make)
                                        .arg(photoInfo.model.isEmpty() ? cnt.unavailable : photoInfo.model);
                if (str.length() > cnt.maxStringLength) str = str.left(cnt.maxStringLength-3) + "...";
                metaStr += cnt.cellBeg + i18n("Make/Model:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
            }

            if (settings->getToolTipsShowPhotoDate())
            {
                if (commonInfo.creationDate.isValid())
                {
                    str = KGlobal::locale()->formatDateTime(commonInfo.creationDate, KLocale::ShortDate, true);
                    if (str.length() > cnt.maxStringLength) str = str.left(cnt.maxStringLength-3) + "...";
                    metaStr += cnt.cellBeg + i18nc("creation date of the image",
                                                   "Created:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
                }
                else
                {
                    metaStr += cnt.cellBeg + i18nc("creation date of the image",
                                                   "Created:") + cnt.cellMid + Qt::escape(cnt.unavailable) + cnt.cellEnd;
                }
            }

            if (settings->getToolTipsShowPhotoFocal())
            {
                str = photoInfo.aperture.isEmpty() ? cnt.unavailable : photoInfo.aperture;

                if (photoInfo.focalLength35.isEmpty())
                    str += QString(" / %1").arg(photoInfo.focalLength.isEmpty() ? cnt.unavailable : photoInfo.focalLength);
                else
                    str += QString(" / %1").arg(i18n("%1 (35mm: %2)",photoInfo.focalLength,photoInfo.focalLength35));

                if (str.length() > cnt.maxStringLength) str = str.left(cnt.maxStringLength-3) + "...";
                metaStr += cnt.cellBeg + i18n("Aperture/Focal:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
            }

            if (settings->getToolTipsShowPhotoExpo())
            {
                str = QString("%1 / %2").arg(photoInfo.exposureTime.isEmpty() ? cnt.unavailable : photoInfo.exposureTime)
                                        .arg(photoInfo.sensitivity.isEmpty() ? cnt.unavailable : i18n("%1 ISO",photoInfo.sensitivity));
                if (str.length() > cnt.maxStringLength) str = str.left(cnt.maxStringLength-3) + "...";
                metaStr += cnt.cellBeg + i18n("Exposure/Sensitivity:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
            }

            if (settings->getToolTipsShowPhotoMode())
            {
                if (photoInfo.exposureMode.isEmpty() && photoInfo.exposureProgram.isEmpty())
                    str = cnt.unavailable;
                else if (!photoInfo.exposureMode.isEmpty() && photoInfo.exposureProgram.isEmpty())
                    str = photoInfo.exposureMode;
                else if (photoInfo.exposureMode.isEmpty() && !photoInfo.exposureProgram.isEmpty())
                    str = photoInfo.exposureProgram;
                else
                    str = QString("%1 / %2").arg(photoInfo.exposureMode).arg(photoInfo.exposureProgram);
                if (str.length() > cnt.maxStringLength) str = str.left(cnt.maxStringLength-3) + "...";
                metaStr += cnt.cellBeg + i18n("Mode/Program:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
            }

            if (settings->getToolTipsShowPhotoFlash())
            {
                str = photoInfo.flashMode.isEmpty() ? cnt.unavailable : photoInfo.flashMode;
                if (str.length() > cnt.maxStringLength) str = str.left(cnt.maxStringLength-3) + "...";
                metaStr += cnt.cellBeg + i18nc("camera flash settings",
                                               "Flash:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
            }

            if (settings->getToolTipsShowPhotoWB())
            {
                str = photoInfo.whiteBalance.isEmpty() ? cnt.unavailable : photoInfo.whiteBalance;
                if (str.length() > cnt.maxStringLength) str = str.left(cnt.maxStringLength-3) + "...";
                metaStr += cnt.cellBeg + i18n("White Balance:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
            }

            tip += metaStr;
        }
    }

    // -- digiKam properties  ------------------------------------------

    if (settings->getToolTipsShowAlbumName() ||
        settings->getToolTipsShowComments()  ||
        settings->getToolTipsShowTags()      ||
        settings->getToolTipsShowRating())
    {
        tip += cnt.headBeg + i18n("digiKam Properties") + cnt.headEnd;

        if (settings->getToolTipsShowAlbumName())
        {
            PAlbum* album = AlbumManager::instance()->findPAlbum(info.albumId());
            if (album)
                tip += cnt.cellSpecBeg + i18n("Album:") + cnt.cellSpecMid + album->albumPath().remove(0, 1) + cnt.cellSpecEnd;
        }

        if (settings->getToolTipsShowComments())
        {
            str = info.comment();
            if (str.isEmpty()) str = QString("---");
            tip += cnt.cellSpecBeg + i18nc("caption of the file",
                                           "Caption:") + cnt.cellSpecMid +
                   cnt.breakString(str) + cnt.cellSpecEnd;
        }

        if (settings->getToolTipsShowTags())
        {
            QStringList tagPaths = AlbumManager::instance()->tagPaths(info.tagIds(), false);
            QStringList tagNames = AlbumManager::instance()->tagNames(info.tagIds());
            QString tags(i18n("Tags:"));

            if (tagPaths.isEmpty())
                tip += cnt.cellSpecBeg + tags + cnt.cellSpecMid + "---" + cnt.cellSpecEnd;
            else
            {
                QString title = tags;
                QString tagText;
                for (int i = 0; i<tagPaths.size(); i++)
                {
                    tagText = tagPaths[i];
                    if (tagText.size() > cnt.maxStringLength)
                        tagText = tagNames[i];
                    if (tagText.size() > cnt.maxStringLength)
                        tagText = cnt.elidedText(tagPaths[i], Qt::ElideLeft);
                    tip += cnt.cellSpecBeg + title + cnt.cellSpecMid + tagText + cnt.cellSpecEnd;
                    title.clear();
                }
            }
        }

        if (settings->getToolTipsShowRating())
        {
            int rating = info.rating();
            if (rating <= 0)
                str = QString("---");
            else
                str.fill( 'X', info.rating() );
            tip += cnt.cellSpecBeg + i18n("Rating:") + cnt.cellSpecMid + str + cnt.cellSpecEnd;
        }
    }

    tip += cnt.tipFooter;

    return tip;
}

QString ToolTipFiller::albumTipContents(PAlbum *album, int count)
{
    if (!album) return QString();

    QString            str;
    AlbumSettings*     settings = AlbumSettings::instance();
    DToolTipStyleSheet cnt(settings->getToolTipsFont());
    QString            tip = cnt.tipHeader;

    if (settings->getToolTipsShowAlbumTitle()      ||
        settings->getToolTipsShowAlbumDate()       ||
        settings->getToolTipsShowAlbumCollection() ||
        settings->getToolTipsShowAlbumCategory()   ||
        settings->getToolTipsShowAlbumCaption())
    {
        tip += cnt.headBeg + i18n("Album Properties") + cnt.headEnd;

        if (settings->getToolTipsShowAlbumTitle())
        {
            tip += cnt.cellBeg + i18n("Name:") + cnt.cellMid;
            tip += album->title() + cnt.cellEnd;
        }

        if (settings->getShowFolderTreeViewItemsCount())
        {
            tip += cnt.cellBeg + i18n("Items:") + cnt.cellMid;
            tip += QString::number(count) + cnt.cellEnd;
        }

        if (settings->getToolTipsShowAlbumCollection())
        {
            tip += cnt.cellBeg + i18n("Collection:") + cnt.cellMid;
            Album *col = AlbumManager::instance()->findAlbum(album->albumRootId());
            tip += col ? col->title() : QString() + cnt.cellEnd;
        }

        if (settings->getToolTipsShowAlbumDate())
        {
            QDate date = album->date();
            str        = KGlobal::locale()->formatDate(date, KLocale::ShortDate);
            tip        += cnt.cellBeg + i18n("Date:") + cnt.cellMid + str + cnt.cellEnd;
        }

        if (settings->getToolTipsShowAlbumCategory())
        {
            str = album->category();
            if (str.isEmpty()) str = QString("---");
            tip += cnt.cellSpecBeg + i18n("Category:") + cnt.cellSpecMid +
                   cnt.breakString(str) + cnt.cellSpecEnd;
        }

        if (settings->getToolTipsShowAlbumCaption())
        {
            str = album->caption();
            if (str.isEmpty()) str = QString("---");
            tip += cnt.cellSpecBeg + i18n("Caption:") + cnt.cellSpecMid +
                   cnt.breakString(str) + cnt.cellSpecEnd;
        }
    }

    tip += cnt.tipFooter;

    return tip;
}

}  // namespace Digikam
