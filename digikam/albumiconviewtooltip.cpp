/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-10
 * Description : album icon view tool tip
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "albumiconviewtooltip.h"

// Qt includes.

#include <QPixmap>
#include <QPainter>
#include <QTextDocument>
#include <QDateTime>

// KDE includes.

#include <kdebug.h>
#include <klocale.h>
#include <kfileitem.h>
#include <kglobal.h>
#include <kdeversion.h>

// Local includes.

#include "albumiconview.h"
#include "albumiconitem.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "album.h"

namespace Digikam
{

class AlbumIconViewToolTipPriv
{
public:

    AlbumIconViewToolTipPriv()
    {
        view     = 0;
        iconItem = 0;
    }

    AlbumIconView *view;

    AlbumIconItem *iconItem;
};

AlbumIconViewToolTip::AlbumIconViewToolTip(AlbumIconView* view)
                    : DItemToolTip(), d(new AlbumIconViewToolTipPriv)
{
    d->view = view;
}

AlbumIconViewToolTip::~AlbumIconViewToolTip()
{
    delete d;
}

void AlbumIconViewToolTip::setIconItem(AlbumIconItem* iconItem)
{
    d->iconItem = iconItem;

    if (!d->iconItem ||
        !AlbumSettings::instance()->showToolTipsIsValid())
    {
        hide();
    }
    else
    {
        updateToolTip();
        reposition();
        if (isHidden())
            show();
    }
}

QRect AlbumIconViewToolTip::repositionRect()
{
    if (!d->iconItem) return QRect();

    QRect rect = d->iconItem->clickToOpenRect();
    rect.moveTopLeft(d->view->contentsToViewport(rect.topLeft()));
    rect.moveTopLeft(d->view->viewport()->mapToGlobal(rect.topLeft()));
    return rect;
}

QString AlbumIconViewToolTip::tipContents()
{
    if (!d->iconItem) return QString();
    ImageInfo info = d->iconItem->imageInfo();
    return fillTipContents(info);
}

QString AlbumIconViewToolTip::fillTipContents(const ImageInfo& info)
{
    QString            str;
    DToolTipStyleSheet cnt;

    AlbumSettings* settings          = AlbumSettings::instance();
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
            tip += cnt.cellBeg + i18n("Name:") + cnt.cellMid;
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
            str = i18n("%1 (%2)", KIO::convertSize(commonInfo.fileSize),
                                  KGlobal::locale()->formatNumber(commonInfo.fileSize, 0));
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
                str = i18n("Unknown");
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
                if (str.length() > cnt.maxStringLenght) str = str.left(cnt.maxStringLenght-3) + "...";
                metaStr += cnt.cellBeg + i18n("Make/Model:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
            }

            if (settings->getToolTipsShowPhotoDate())
            {
                if (commonInfo.creationDate.isValid())
                {
                    str = KGlobal::locale()->formatDateTime(commonInfo.creationDate, KLocale::ShortDate, true);
                    if (str.length() > cnt.maxStringLenght) str = str.left(cnt.maxStringLenght-3) + "...";
                    metaStr += cnt.cellBeg + i18n("Created:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
                }
                else
                {
                    metaStr += cnt.cellBeg + i18n("Created:") + cnt.cellMid + Qt::escape(cnt.unavailable) + cnt.cellEnd;
                }
            }

            if (settings->getToolTipsShowPhotoFocal())
            {
                str = photoInfo.aperture.isEmpty() ? cnt.unavailable : photoInfo.aperture;

                if (photoInfo.focalLength35.isEmpty())
                    str += QString(" / %1").arg(photoInfo.focalLength.isEmpty() ? cnt.unavailable : photoInfo.focalLength);
                else
                    str += QString(" / %1").arg(i18n("%1 (35mm: %2)",photoInfo.focalLength,photoInfo.focalLength35));

                if (str.length() > cnt.maxStringLenght) str = str.left(cnt.maxStringLenght-3) + "...";
                metaStr += cnt.cellBeg + i18n("Aperture/Focal:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
            }

            if (settings->getToolTipsShowPhotoExpo())
            {
                str = QString("%1 / %2").arg(photoInfo.exposureTime.isEmpty() ? cnt.unavailable : photoInfo.exposureTime)
                                        .arg(photoInfo.sensitivity.isEmpty() ? cnt.unavailable : i18n("%1 ISO",photoInfo.sensitivity));
                if (str.length() > cnt.maxStringLenght) str = str.left(cnt.maxStringLenght-3) + "...";
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
                if (str.length() > cnt.maxStringLenght) str = str.left(cnt.maxStringLenght-3) + "...";
                metaStr += cnt.cellBeg + i18n("Mode/Program:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
            }

            if (settings->getToolTipsShowPhotoFlash())
            {
                str = photoInfo.flashMode.isEmpty() ? cnt.unavailable : photoInfo.flashMode;
                if (str.length() > cnt.maxStringLenght) str = str.left(cnt.maxStringLenght-3) + "...";
                metaStr += cnt.cellBeg + i18n("Flash:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
            }

            if (settings->getToolTipsShowPhotoWB())
            {
                str = photoInfo.whiteBalance.isEmpty() ? cnt.unavailable : photoInfo.whiteBalance;
                if (str.length() > cnt.maxStringLenght) str = str.left(cnt.maxStringLenght-3) + "...";
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
            tip += cnt.cellSpecBeg + i18n("Caption:") + cnt.cellSpecMid + 
                   cnt.breakString(str) + cnt.cellSpecEnd;
        }

        if (settings->getToolTipsShowTags())
        {
            QStringList tagPaths = AlbumManager::instance()->tagPaths(info.tagIds(), false);

            str = tagPaths.join(", ");
            if (str.isEmpty()) str = QString("---");
            if (str.length() > cnt.maxStringLenght) str = str.left(cnt.maxStringLenght-3) + "...";
            tip += cnt.cellSpecBeg + i18n("Tags:") + cnt.cellSpecMid + str + cnt.cellSpecEnd;
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

}  // namespace Digikam
