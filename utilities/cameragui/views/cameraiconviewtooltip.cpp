/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-17
 * Description : camera icon view tool tip
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

#include "cameraiconviewtooltip.h"

// Qt includes

#include <QPixmap>
#include <QPainter>
#include <QTextDocument>
#include <QDateTime>

// KDE includes


#include <klocale.h>
#include <kfileitem.h>
#include <kglobal.h>
#include <kdeversion.h>
#include <kmimetype.h>

// Local includes

#include "cameraiconview.h"
#include "cameraiconitem.h"
#include "albumsettings.h"

namespace Digikam
{

class CameraIconViewToolTipPriv
{
public:

    CameraIconViewToolTipPriv() :
        view(0),
        iconItem(0)
    {
    }

    CameraIconView* view;
    CameraIconItem* iconItem;
};

CameraIconViewToolTip::CameraIconViewToolTip(CameraIconView* view)
    : DItemToolTip(), d(new CameraIconViewToolTipPriv)
{
    d->view = view;
}

CameraIconViewToolTip::~CameraIconViewToolTip()
{
    delete d;
}

void CameraIconViewToolTip::setIconItem(CameraIconItem* iconItem)
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

        if (isHidden() && !toolTipIsEmpty())
        {
            show();
        }
    }
}

QRect CameraIconViewToolTip::repositionRect()
{
    if (!d->iconItem)
    {
        return QRect();
    }

    QRect rect = d->iconItem->clickToOpenRect();
    rect.moveTopLeft(d->view->contentsToViewport(rect.topLeft()));
    rect.moveTopLeft(d->view->viewport()->mapToGlobal(rect.topLeft()));
    return rect;
}

QString CameraIconViewToolTip::tipContents()
{
    if (!d->iconItem)
    {
        return QString();
    }

    GPItemInfo* info = d->iconItem->itemInfo();
    return fillTipContents(info);
}

QString CameraIconViewToolTip::fillTipContents(GPItemInfo* info)
{
    QString            str;
    AlbumSettings*     settings = AlbumSettings::instance();
    DToolTipStyleSheet cnt(settings->getToolTipsFont());

    QString tip                  = cnt.tipHeader;
    PhotoInfoContainer photoInfo = info->photoInfo;

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
            tip += info->name + cnt.cellEnd;
        }

        if (settings->getToolTipsShowFileDate())
        {
            QDateTime creatededDate = info->mtime;
            str = KGlobal::locale()->formatDateTime(creatededDate, KLocale::ShortDate, true);
            tip += cnt.cellBeg + i18n("Date:") + cnt.cellMid + str + cnt.cellEnd;
        }

        if (settings->getToolTipsShowFileSize())
        {
            tip += cnt.cellBeg + i18n("Size:") + cnt.cellMid;
            str = i18n("%1 (%2)", KIO::convertSize(info->size),
                       KGlobal::locale()->formatNumber(info->size, 0));
            tip += str + cnt.cellEnd;
        }

        if (settings->getToolTipsShowImageType())
        {
            KMimeType::Ptr mt = KMimeType::mimeType(info->mime);

            if (mt)
            {
                tip += cnt.cellBeg + i18n("Type:") + cnt.cellMid + mt->comment() + cnt.cellEnd;
            }
        }

        if (settings->getToolTipsShowImageDim())
        {
            if (info->width == -1 || info->height == -1)
            {
                str = i18n("Unknown");
            }
            else
            {
                QString mpixels;
                mpixels.setNum(info->width*info->height/1000000.0, 'f', 2);
                str = i18nc("width x height (megapixels Mpx)", "%1x%2 (%3Mpx)",
                            info->width, info->height, mpixels);
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
        if (!photoInfo.isEmpty())
        {
            QString metaStr;
            tip += cnt.headBeg + i18n("Photograph Properties") + cnt.headEnd;

            if (settings->getToolTipsShowPhotoMake())
            {
                str = QString("%1 / %2").arg(photoInfo.make.isEmpty() ? cnt.unavailable : photoInfo.make)
                      .arg(photoInfo.model.isEmpty() ? cnt.unavailable : photoInfo.model);

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + "...";
                }

                metaStr += cnt.cellBeg + i18n("Make/Model:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
            }

            if (settings->getToolTipsShowPhotoDate())
            {
                if (photoInfo.dateTime.isValid())
                {
                    str = KGlobal::locale()->formatDateTime(photoInfo.dateTime, KLocale::ShortDate, true);

                    if (str.length() > cnt.maxStringLength)
                    {
                        str = str.left(cnt.maxStringLength-3) + "...";
                    }

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

                if (photoInfo.focalLength35mm.isEmpty())
                {
                    str += QString(" / %1").arg(photoInfo.focalLength.isEmpty() ? cnt.unavailable : photoInfo.focalLength);
                }
                else
                    str += QString(" / %1").arg(i18n("%1 (35mm: %2)",
                                                     photoInfo.focalLength, photoInfo.focalLength35mm));

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + "...";
                }

                metaStr += cnt.cellBeg + i18n("Aperture/Focal:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
            }

            if (settings->getToolTipsShowPhotoExpo())
            {
                str = QString("%1 / %2").arg(photoInfo.exposureTime.isEmpty() ? cnt.unavailable :
                                             photoInfo.exposureTime)
                      .arg(photoInfo.sensitivity.isEmpty() ? cnt.unavailable :
                           i18n("%1 ISO", photoInfo.sensitivity));

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + "...";
                }

                metaStr += cnt.cellBeg + i18n("Exposure/Sensitivity:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
            }

            if (settings->getToolTipsShowPhotoMode())
            {
                if (photoInfo.exposureMode.isEmpty() && photoInfo.exposureProgram.isEmpty())
                {
                    str = cnt.unavailable;
                }
                else if (!photoInfo.exposureMode.isEmpty() && photoInfo.exposureProgram.isEmpty())
                {
                    str = photoInfo.exposureMode;
                }
                else if (photoInfo.exposureMode.isEmpty() && !photoInfo.exposureProgram.isEmpty())
                {
                    str = photoInfo.exposureProgram;
                }
                else
                {
                    str = QString("%1 / %2").arg(photoInfo.exposureMode).arg(photoInfo.exposureProgram);
                }

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + "...";
                }

                metaStr += cnt.cellBeg + i18n("Mode/Program:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
            }

            if (settings->getToolTipsShowPhotoFlash())
            {
                str = photoInfo.flash.isEmpty() ? cnt.unavailable : photoInfo.flash;

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + "...";
                }

                metaStr += cnt.cellBeg + i18n("Flash:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
            }

            if (settings->getToolTipsShowPhotoWB())
            {
                str = photoInfo.whiteBalance.isEmpty() ? cnt.unavailable : photoInfo.whiteBalance;

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + "...";
                }

                metaStr += cnt.cellBeg + i18n("White Balance:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
            }

            tip += metaStr;
        }
    }

    tip += cnt.tipFooter;

    return tip;
}

}  // namespace Digikam
