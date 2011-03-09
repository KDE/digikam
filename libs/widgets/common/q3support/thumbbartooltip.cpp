/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-09
 * Description : thumbbar tool tip
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

#include "thumbbartooltip.h"

// Qt includes

#include <QPixmap>
#include <QPainter>
#include <QTextDocument>
#include <QFileInfo>

// KDE includes


#include <klocale.h>
#include <kfileitem.h>
#include <kmimetype.h>
#include <kdeversion.h>

// LibKDcraw includes

#include <libkdcraw/kdcraw.h>
#include <libkdcraw/version.h>

// Local includes

#include "thumbbar.h"
#include "dmetadata.h"

namespace Digikam
{

class ThumbBarToolTipPriv
{
public:

    ThumbBarToolTipPriv() :
        view(0),
        item(0)
    {
    }

    ThumbBarView* view;

    ThumbBarItem* item;
};

ThumbBarToolTip::ThumbBarToolTip(ThumbBarView* view)
    : DItemToolTip(), d(new ThumbBarToolTipPriv)
{
    d->view = view;
}

ThumbBarToolTip::~ThumbBarToolTip()
{
    delete d;
}

ThumbBarToolTipSettings& ThumbBarToolTip::toolTipSettings() const
{
    return d->view->getToolTipSettings();
}

void ThumbBarToolTip::setItem(ThumbBarItem* item)
{
    d->item = item;

    if (!d->item)
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

ThumbBarItem* ThumbBarToolTip::item() const
{
    return d->item;
}

QRect ThumbBarToolTip::repositionRect()
{
    if (!item())
    {
        return QRect();
    }

    QRect rect = item()->rect();
    rect.moveTopLeft(d->view->contentsToViewport(rect.topLeft()));
    rect.moveTopLeft(d->view->viewport()->mapToGlobal(rect.topLeft()));
    return rect;
}

QString ThumbBarToolTip::tipContents()
{
    if (!item())
    {
        return QString();
    }

    QString                 str;
    ThumbBarToolTipSettings settings = toolTipSettings();
    DToolTipStyleSheet      cnt(settings.font);

    QFileInfo fileInfo(item()->url().toLocalFile());
    KFileItem fi(KFileItem::Unknown, KFileItem::Unknown, item()->url());
    DMetadata metaData(item()->url().toLocalFile());
    QString tip = cnt.tipHeader;

    // -- File properties ----------------------------------------------

    if (settings.showFileName  ||
        settings.showFileDate  ||
        settings.showFileSize  ||
        settings.showImageType ||
        settings.showImageDim)
    {
        tip += cnt.headBeg + i18n("File Properties") + cnt.headEnd;

        if (settings.showFileName)
        {
            tip += cnt.cellBeg + i18n("Name:") + cnt.cellMid;
            tip += item()->url().fileName() + cnt.cellEnd;
        }

        if (settings.showFileDate)
        {
            QDateTime modifiedDate = fileInfo.lastModified();
            str = KGlobal::locale()->formatDateTime(modifiedDate, KLocale::ShortDate, true);
            tip += cnt.cellBeg + i18n("Date:") + cnt.cellMid + str + cnt.cellEnd;
        }

        if (settings.showFileSize)
        {
            tip += cnt.cellBeg + i18n("Size:") + cnt.cellMid;
            str = i18n("%1 (%2)", KIO::convertSize(fi.size()),
                       KGlobal::locale()->formatNumber(fi.size(),
                               0));
            tip += str + cnt.cellEnd;
        }

        QSize   dims;

        QString rawFilesExt(KDcrawIface::KDcraw::rawFiles());
        QString ext = fileInfo.suffix().toUpper();

        if (!ext.isEmpty() && rawFilesExt.toUpper().contains(ext))
        {
            str = i18n("RAW Image");
            dims = metaData.getImageDimensions();
        }
        else
        {
            str = fi.mimeComment();

            KFileMetaInfo meta = fi.metaInfo();

            /*          TODO: KDE4PORT: KFileMetaInfo API as Changed.
                                        Check if new method to search "Dimensions" information is enough.

                        if (meta.isValid())
                        {
                            if (meta.containsGroup("Jpeg EXIF Data"))
                                dims = meta.group("Jpeg EXIF Data").item("Dimensions").value().toSize();
                            else if (meta.containsGroup("General"))
                                dims = meta.group("General").item("Dimensions").value().toSize();
                            else if (meta.containsGroup("Technical"))
                                dims = meta.group("Technical").item("Dimensions").value().toSize();
                        }*/

            if (meta.isValid() && meta.item("Dimensions").isValid())
            {
                dims = meta.item("Dimensions").value().toSize();
            }
        }

        if (settings.showImageType)
        {
            tip += cnt.cellBeg + i18n("Type:") + cnt.cellMid + str + cnt.cellEnd;
        }

        if (settings.showImageDim)
        {
            QString mpixels;
            mpixels.setNum(dims.width()*dims.height()/1000000.0, 'f', 2);
            str = (!dims.isValid()) ? i18n("Unknown") : i18n("%1x%2 (%3Mpx)",
                    dims.width(), dims.height(), mpixels);
            tip += cnt.cellBeg + i18n("Dimensions:") + cnt.cellMid + str + cnt.cellEnd;
        }
    }

    // -- Photograph Info ----------------------------------------------------

    if (settings.showPhotoMake  ||
        settings.showPhotoDate  ||
        settings.showPhotoFocal ||
        settings.showPhotoExpo  ||
        settings.showPhotoMode  ||
        settings.showPhotoFlash ||
        settings.showPhotoWB)
    {
        PhotoInfoContainer photoInfo = metaData.getPhotographInformation();

        if (!photoInfo.isEmpty())
        {
            QString metaStr;
            tip += cnt.headBeg + i18n("Photograph Properties") + cnt.headEnd;

            if (settings.showPhotoMake)
            {
                str = QString("%1 / %2").arg(photoInfo.make.isEmpty() ? cnt.unavailable : photoInfo.make)
                      .arg(photoInfo.model.isEmpty() ? cnt.unavailable : photoInfo.model);

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + "...";
                }

                metaStr += cnt.cellBeg + i18n("Make/Model:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
            }

            if (settings.showPhotoDate)
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

            if (settings.showPhotoFocal)
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

            if (settings.showPhotoExpo)
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

            if (settings.showPhotoMode)
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

            if (settings.showPhotoFlash)
            {
                str = photoInfo.flash.isEmpty() ? cnt.unavailable : photoInfo.flash;

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + "...";
                }

                metaStr += cnt.cellBeg + i18n("Flash:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
            }

            if (settings.showPhotoWB)
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
