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

    QString tip, str;
    QString unavailable(i18n("unavailable"));

    tip = "<table cellspacing=\"0\" cellpadding=\"0\" width=\"250\" border=\"0\">";

    AlbumSettings* settings          = AlbumSettings::instance();
    const ImageInfo info             = d->iconItem->imageInfo();
    ImageCommonContainer commonInfo  = info.imageCommonContainer();
    ImageMetadataContainer photoInfo = info.imageMetadataContainer();

    // -- File properties ----------------------------------------------

    if (settings->getToolTipsShowFileName()  ||
        settings->getToolTipsShowFileDate()  ||
        settings->getToolTipsShowFileSize()  ||
        settings->getToolTipsShowImageType() ||
        settings->getToolTipsShowImageDim())
    {
        tip += m_headBeg + i18n("File Properties") + m_headEnd;

        if (settings->getToolTipsShowFileName())
        {
            tip += m_cellBeg + i18n("Name:") + m_cellMid;
            tip += commonInfo.fileName + m_cellEnd;
        }

        if (settings->getToolTipsShowFileDate())
        {
            QDateTime modifiedDate = commonInfo.fileModificationDate;
            str = KGlobal::locale()->formatDateTime(modifiedDate, KLocale::ShortDate, true);
            tip += m_cellBeg + i18n("Modified:") + m_cellMid + str + m_cellEnd;
        }

        if (settings->getToolTipsShowFileSize())
        {
            tip += m_cellBeg + i18n("Size:") + m_cellMid;
            str = i18n("%1 (%2)", KIO::convertSize(commonInfo.fileSize),
                                  KGlobal::locale()->formatNumber(commonInfo.fileSize, 0));
            tip += str + m_cellEnd;
        }

        QSize dims;

        if (settings->getToolTipsShowImageType())
        {
            tip += m_cellBeg + i18n("Type:") + m_cellMid + commonInfo.format + m_cellEnd;
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
            tip += m_cellBeg + i18n("Dimensions:") + m_cellMid + str + m_cellEnd;
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
            tip += m_headBeg + i18n("Photograph Properties") + m_headEnd;

            if (settings->getToolTipsShowPhotoMake())
            {
                str = QString("%1 / %2").arg(photoInfo.make.isEmpty() ? unavailable : photoInfo.make)
                                        .arg(photoInfo.model.isEmpty() ? unavailable : photoInfo.model);
                if (str.length() > m_maxStringLen) str = str.left(m_maxStringLen-3) + "...";
                metaStr += m_cellBeg + i18n("Make/Model:") + m_cellMid + Qt::escape( str ) + m_cellEnd;
            }

            if (settings->getToolTipsShowPhotoDate())
            {
                if (commonInfo.creationDate.isValid())
                {
                    str = KGlobal::locale()->formatDateTime(commonInfo.creationDate, KLocale::ShortDate, true);
                    if (str.length() > m_maxStringLen) str = str.left(m_maxStringLen-3) + "...";
                    metaStr += m_cellBeg + i18n("Created:") + m_cellMid + Qt::escape( str ) + m_cellEnd;
                }
                else
                {
                    metaStr += m_cellBeg + i18n("Created:") + m_cellMid + Qt::escape( unavailable ) + m_cellEnd;
                }
            }

            if (settings->getToolTipsShowPhotoFocal())
            {
                str = photoInfo.aperture.isEmpty() ? unavailable : photoInfo.aperture;

                if (photoInfo.focalLength35.isEmpty())
                    str += QString(" / %1").arg(photoInfo.focalLength.isEmpty() ? unavailable : photoInfo.focalLength);
                else
                    str += QString(" / %1").arg(i18n("%1 (35mm: %2)",photoInfo.focalLength,photoInfo.focalLength35));

                if (str.length() > m_maxStringLen) str = str.left(m_maxStringLen-3) + "...";
                metaStr += m_cellBeg + i18n("Aperture/Focal:") + m_cellMid + Qt::escape( str ) + m_cellEnd;
            }

            if (settings->getToolTipsShowPhotoExpo())
            {
                str = QString("%1 / %2").arg(photoInfo.exposureTime.isEmpty() ? unavailable : photoInfo.exposureTime)
                                        .arg(photoInfo.sensitivity.isEmpty() ? unavailable : i18n("%1 ISO",photoInfo.sensitivity));
                if (str.length() > m_maxStringLen) str = str.left(m_maxStringLen-3) + "...";
                metaStr += m_cellBeg + i18n("Exposure/Sensitivity:") + m_cellMid + Qt::escape( str ) + m_cellEnd;
            }

            if (settings->getToolTipsShowPhotoMode())
            {

                if (photoInfo.exposureMode.isEmpty() && photoInfo.exposureProgram.isEmpty())
                    str = unavailable;
                else if (!photoInfo.exposureMode.isEmpty() && photoInfo.exposureProgram.isEmpty())
                    str = photoInfo.exposureMode;
                else if (photoInfo.exposureMode.isEmpty() && !photoInfo.exposureProgram.isEmpty())
                    str = photoInfo.exposureProgram;
                else
                    str = QString("%1 / %2").arg(photoInfo.exposureMode).arg(photoInfo.exposureProgram);
                if (str.length() > m_maxStringLen) str = str.left(m_maxStringLen-3) + "...";
                metaStr += m_cellBeg + i18n("Mode/Program:") + m_cellMid + Qt::escape( str ) + m_cellEnd;
            }

            if (settings->getToolTipsShowPhotoFlash())
            {
                str = photoInfo.flashMode.isEmpty() ? unavailable : photoInfo.flashMode;
                if (str.length() > m_maxStringLen) str = str.left(m_maxStringLen-3) + "...";
                metaStr += m_cellBeg + i18n("Flash:") + m_cellMid + Qt::escape( str ) + m_cellEnd;
            }

            if (settings->getToolTipsShowPhotoWB())
            {
                str = photoInfo.whiteBalance.isEmpty() ? unavailable : photoInfo.whiteBalance;
                if (str.length() > m_maxStringLen) str = str.left(m_maxStringLen-3) + "...";
                metaStr += m_cellBeg + i18n("White Balance:") + m_cellMid + Qt::escape( str ) + m_cellEnd;
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
        tip += m_headBeg + i18n("digiKam Properties") + m_headEnd;

        if (settings->getToolTipsShowAlbumName())
        {
            PAlbum* album = AlbumManager::instance()->findPAlbum(info.albumId());
            if (album)
                tip += m_cellSpecBeg + i18n("Album:") + m_cellSpecMid + album->albumPath().remove(0, 1) + m_cellSpecEnd;
        }

        if (settings->getToolTipsShowComments())
        {
            str = info.comment();
            if (str.isEmpty()) str = QString("---");
            tip += m_cellSpecBeg + i18n("Caption:") + m_cellSpecMid + breakString(str) + m_cellSpecEnd;
        }

        if (settings->getToolTipsShowTags())
        {
            QStringList tagPaths = AlbumManager::instance()->tagPaths(info.tagIds(), false);

            str = tagPaths.join(", ");
            if (str.isEmpty()) str = QString("---");
            if (str.length() > m_maxStringLen) str = str.left(m_maxStringLen-3) + "...";
            tip += m_cellSpecBeg + i18n("Tags:") + m_cellSpecMid + str + m_cellSpecEnd;
        }

        if (settings->getToolTipsShowRating())
        {
            int rating = info.rating();
            if (rating <= 0)
                str = QString("---");
            else
                str.fill( 'X', info.rating() );
            tip += m_cellSpecBeg + i18n("Rating:") + m_cellSpecMid + str + m_cellSpecEnd;
        }
    }

    tip += "</table>";

    return tip;
}

}  // namespace Digikam
