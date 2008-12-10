/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-09
 * Description : thumbbar tool tip
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

#include "thumbbartooltip.h"

// Qt includes.

#include <QPixmap>
#include <QPainter>
#include <QTextDocument>
#include <QFileInfo>

// KDE includes.

#include <kdebug.h>
#include <klocale.h>
#include <kfileitem.h>
#include <kmimetype.h>
#include <kdeversion.h>

// LibKDcraw includes.

#include <libkdcraw/kdcraw.h>
#include <libkdcraw/version.h>

#if KDCRAW_VERSION < 0x000400
#include <libkdcraw/dcrawbinary.h>
#endif

// Local includes.

#include "thumbbar.h"
#include "dmetadata.h"

namespace Digikam
{

class ThumbBarToolTipPriv
{
public:

    ThumbBarToolTipPriv()
    {
        view   = 0;
        item   = 0;
    }

    ThumbBarView  *view;

    ThumbBarItem  *item;
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
        if (isHidden())
            show();
    }
}

ThumbBarItem* ThumbBarToolTip::item() const
{
    return d->item;
}

QRect ThumbBarToolTip::repositionRect()
{
    if (!item()) return QRect();

    QRect rect = item()->rect();
    rect.moveTopLeft(d->view->contentsToViewport(rect.topLeft()));
    rect.moveTopLeft(d->view->viewport()->mapToGlobal(rect.topLeft()));
    return rect;
}

QString ThumbBarToolTip::tipContents()
{
    if (!item()) return QString();

    ThumbBarToolTipSettings settings = toolTipSettings();

    QString tipText, str;
    QString unavailable(i18n("unavailable"));

    tipText = "<table cellspacing=\"0\" cellpadding=\"0\" width=\"250\" border=\"0\">";

    QFileInfo fileInfo(item()->url().path());
    KFileItem fi(KFileItem::Unknown, KFileItem::Unknown, item()->url());
    DMetadata metaData(item()->url().path());

    // -- File properties ----------------------------------------------

    if (settings.showFileName  ||
        settings.showFileDate  ||
        settings.showFileSize  ||
        settings.showImageType ||
        settings.showImageDim)
    {
        tipText += m_headBeg + i18n("File Properties") + m_headEnd;

        if (settings.showFileName)
        {
            tipText += m_cellBeg + i18n("Name:") + m_cellMid;
            tipText += item()->url().fileName() + m_cellEnd;
        }

        if (settings.showFileDate)
        {
            QDateTime modifiedDate = fileInfo.lastModified();
            str = KGlobal::locale()->formatDateTime(modifiedDate, KLocale::ShortDate, true);
            tipText += m_cellBeg + i18n("Modified:") + m_cellMid + str + m_cellEnd;
        }

        if (settings.showFileSize)
        {
            tipText += m_cellBeg + i18n("Size:") + m_cellMid;
            str = i18n("%1 (%2)", KIO::convertSize(fi.size()),
                                  KGlobal::locale()->formatNumber(fi.size(),
                                  0));
            tipText += str + m_cellEnd;
        }

        QSize   dims;

#if KDCRAW_VERSION < 0x000400
        QString rawFilesExt(KDcrawIface::DcrawBinary::instance()->rawFiles());
#else
        QString rawFilesExt(KDcrawIface::KDcraw::rawFiles());
#endif
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
            tipText += m_cellBeg + i18n("Type:") + m_cellMid + str + m_cellEnd;
        }

        if (settings.showImageDim)
        {
            QString mpixels;
            mpixels.setNum(dims.width()*dims.height()/1000000.0, 'f', 2);
            str = (!dims.isValid()) ? i18n("Unknown") : i18n("%1x%2 (%3Mpx)",
                    dims.width(), dims.height(), mpixels);
            tipText += m_cellBeg + i18n("Dimensions:") + m_cellMid + str + m_cellEnd;
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
        PhotoInfoContainer photoInfo = metaData.getPhotographInformations();

        if (!photoInfo.isEmpty())
        {
            QString metaStr;
            tipText += m_headBeg + i18n("Photograph Properties") + m_headEnd;

            if (settings.showPhotoMake)
            {
                str = QString("%1 / %2").arg(photoInfo.make.isEmpty() ? unavailable : photoInfo.make)
                                        .arg(photoInfo.model.isEmpty() ? unavailable : photoInfo.model);
                if (str.length() > m_maxStringLen) str = str.left(m_maxStringLen-3) + "...";
                metaStr += m_cellBeg + i18n("Make/Model:") + m_cellMid + Qt::escape( str ) + m_cellEnd;
            }

            if (settings.showPhotoDate)
            {
                if (photoInfo.dateTime.isValid())
                {
                    str = KGlobal::locale()->formatDateTime(photoInfo.dateTime, KLocale::ShortDate, true);
                    if (str.length() > m_maxStringLen) str = str.left(m_maxStringLen-3) + "...";
                    metaStr += m_cellBeg + i18n("Created:") + m_cellMid + Qt::escape( str ) + m_cellEnd;
                }
                else
                    metaStr += m_cellBeg + i18n("Created:") + m_cellMid + Qt::escape( unavailable ) + m_cellEnd;
            }

            if (settings.showPhotoFocal)
            {
                str = photoInfo.aperture.isEmpty() ? unavailable : photoInfo.aperture;

                if (photoInfo.focalLength35mm.isEmpty())
                    str += QString(" / %1").arg(photoInfo.focalLength.isEmpty() ? unavailable : photoInfo.focalLength);
                else
                    str += QString(" / %1").arg(i18n("%1 (35mm: %2)",
                           photoInfo.focalLength, photoInfo.focalLength35mm));

                if (str.length() > m_maxStringLen) str = str.left(m_maxStringLen-3) + "...";
                metaStr += m_cellBeg + i18n("Aperture/Focal:") + m_cellMid + Qt::escape( str ) + m_cellEnd;
            }

            if (settings.showPhotoExpo)
            {
                str = QString("%1 / %2").arg(photoInfo.exposureTime.isEmpty() ? unavailable :
                                             photoInfo.exposureTime)
                                        .arg(photoInfo.sensitivity.isEmpty() ? unavailable :
                                             i18n("%1 ISO", photoInfo.sensitivity));
                if (str.length() > m_maxStringLen) str = str.left(m_maxStringLen-3) + "...";
                metaStr += m_cellBeg + i18n("Exposure/Sensitivity:") + m_cellMid + Qt::escape( str ) + m_cellEnd;
            }

            if (settings.showPhotoMode)
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

            if (settings.showPhotoFlash)
            {
                str = photoInfo.flash.isEmpty() ? unavailable : photoInfo.flash;
                if (str.length() > m_maxStringLen) str = str.left(m_maxStringLen-3) + "...";
                metaStr += m_cellBeg + i18n("Flash:") + m_cellMid + Qt::escape( str ) + m_cellEnd;
            }

            if (settings.showPhotoWB)
            {
                str = photoInfo.whiteBalance.isEmpty() ? unavailable : photoInfo.whiteBalance;
                if (str.length() > m_maxStringLen) str = str.left(m_maxStringLen-3) + "...";
                metaStr += m_cellBeg + i18n("White Balance:") + m_cellMid + Qt::escape( str ) + m_cellEnd;
            }

            tipText += metaStr;
        }
    }

    return tipText;
}

}  // namespace Digikam
