/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-19
 * Description : Album item file tip adapted from kfiletip 
 *               (konqueror - konq_iconviewwidget.cc)
 *
 * Copyright (C) 1998-1999 by Torben Weis <weis@kde.org>
 * Copyright (C) 2000-2002 by David Faure <david@mandrakesoft.com>
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com> 
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

// Qt includes.

#include <qtooltip.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qdatetime.h>
#include <qstylesheet.h>
#include <qpainter.h>
#include <qapplication.h>

// KDE includes.

#include <klocale.h>
#include <kfileitem.h>
#include <kmimetype.h>
#include <kglobalsettings.h>
#include <kglobal.h>
#include <kdeversion.h>

// LibKDcraw includes.

#include <libkdcraw/version.h>
#include <libkdcraw/kdcraw.h>

#if KDCRAW_VERSION < 0x000106
#include <libkdcraw/dcrawbinary.h>
#endif

// Local includes.

#include "ddebug.h"
#include "dmetadata.h"
#include "albumiconview.h"
#include "albumiconitem.h"
#include "albumsettings.h"
#include "album.h"
#include "albumfiletip.h"

namespace Digikam
{

class AlbumFileTipPriv
{
public:

    AlbumFileTipPriv() : 
        maxStringLen(30), tipBorder(5)
    {
        corner   = 0;
        label    = 0;
        view     = 0;
        iconItem = 0;
    }

    const uint     maxStringLen;
    const uint     tipBorder;

    int            corner;

    QLabel        *label;

    QPixmap        corners[4];

    AlbumIconView *view;

    AlbumIconItem *iconItem;
};

AlbumFileTip::AlbumFileTip(AlbumIconView* view)
            : QFrame(0, 0, WStyle_Customize | WStyle_NoBorder | WStyle_Tool |
                     WStyle_StaysOnTop | WX11BypassWM)
{
    d = new AlbumFileTipPriv;
    d->view = view;
    hide();

    setPalette(QToolTip::palette());
    setFrameStyle(QFrame::Plain | QFrame::Box);
    setLineWidth(1);

    QVBoxLayout *layout = new QVBoxLayout(this, d->tipBorder+1, 0);

    d->label = new QLabel(this);
    d->label->setMargin(0);
    d->label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    layout->addWidget(d->label);
    layout->setResizeMode(QLayout::Fixed);

    renderArrows();
}

AlbumFileTip::~AlbumFileTip()
{
    delete d;
}

void AlbumFileTip::setIconItem(AlbumIconItem* iconItem)
{
    d->iconItem = iconItem;

    if (!d->iconItem || 
        !AlbumSettings::instance()->showToolTipsIsValid())
    {
        hide();
    }
    else
    {
        updateText();
        reposition();
        if (isHidden())
            show();
    }
}

void AlbumFileTip::reposition()
{
    if (!d->iconItem)
        return;

    QRect rect = d->iconItem->clickToOpenRect();
    rect.moveTopLeft(d->view->contentsToViewport(rect.topLeft()));
    rect.moveTopLeft(d->view->viewport()->mapToGlobal(rect.topLeft()));

    QPoint pos = rect.center();
    // d->corner:
    // 0: upperleft
    // 1: upperright
    // 2: lowerleft
    // 3: lowerright

    d->corner = 0;
    // should the tooltip be shown to the left or to the right of the ivi ?

#if KDE_IS_VERSION(3,2,0)
    QRect desk = KGlobalSettings::desktopGeometry(rect.center());
#else
    QRect desk = QApplication::desktop()->screenGeometry(
                               QApplication::desktop()->screenNumber(rect.center()) );
#endif

    if (rect.center().x() + width() > desk.right())
    {
        // to the left
        if (pos.x() - width() < 0) 
        {
            pos.setX(0);
            d->corner = 4;
        } 
        else 
        {
            pos.setX( pos.x() - width() );
            d->corner = 1;
        }
    }

    // should the tooltip be shown above or below the ivi ?
    if (rect.bottom() + height() > desk.bottom())
    {
        // above
        pos.setY( rect.top() - height() - 5);
        d->corner += 2;
    }
    else
    {
        pos.setY( rect.bottom() + 5 );
    }

    move( pos );
}

void AlbumFileTip::renderArrows()
{
    int w = d->tipBorder;

    // -- left top arrow -------------------------------------

    QPixmap& pix0 = d->corners[0];
    pix0.resize(w, w);
    pix0.fill(colorGroup().background());

    QPainter p0(&pix0);
    p0.setPen(QPen(Qt::black, 1));

    for (int j=0; j<w; j++)
        p0.drawLine(0, j, w-j-1, j);

    p0.end();

    // -- right top arrow ------------------------------------

    QPixmap& pix1 = d->corners[1];
    pix1.resize(w, w);
    pix1.fill(colorGroup().background());

    QPainter p1(&pix1);
    p1.setPen(QPen(Qt::black, 1));

    for (int j=0; j<w; j++)
        p1.drawLine(j, j, w-1, j);

    p1.end();

    // -- left bottom arrow ----------------------------------

    QPixmap& pix2 = d->corners[2];
    pix2.resize(w, w);
    pix2.fill(colorGroup().background());

    QPainter p2(&pix2);
    p2.setPen(QPen(Qt::black, 1));

    for (int j=0; j<w; j++)
        p2.drawLine(0, j, j, j);

    p2.end();

    // -- right bottom arrow ---------------------------------

    QPixmap& pix3 = d->corners[3];
    pix3.resize(w, w);
    pix3.fill(colorGroup().background());

    QPainter p3(&pix3);
    p3.setPen(QPen(Qt::black, 1));

    for (int j=0; j<w; j++)
        p3.drawLine(w-j-1, j, w-1, j);

    p3.end();
}

bool AlbumFileTip::event(QEvent *e)
{
    switch ( e->type() )
    {
        case QEvent::Leave:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::FocusIn:
        case QEvent::FocusOut:
        case QEvent::Wheel:
            hide();
        default:
            break;
    }

    return QFrame::event(e);
}

void AlbumFileTip::resizeEvent(QResizeEvent* e)
{
    QFrame::resizeEvent(e);
    reposition();
}

void AlbumFileTip::drawContents(QPainter *p)
{
    if (d->corner >= 4)
    {
        QFrame::drawContents( p );
        return;
    }

    QPixmap &pix = d->corners[d->corner];

    switch ( d->corner )
    {
        case 0:
            p->drawPixmap( 3, 3, pix );
            break;
        case 1:
            p->drawPixmap( width() - pix.width() - 3, 3, pix );
            break;
        case 2:
            p->drawPixmap( 3, height() - pix.height() - 3, pix );
            break;
        case 3:
            p->drawPixmap( width() - pix.width() - 3, height() - pix.height() - 3, pix );
            break;
    }

    QFrame::drawContents(p);
}

void AlbumFileTip::updateText()
{
    QString tip, str;
    QString unavailable(i18n("unavailable"));

    QString headBeg("<tr bgcolor=\"orange\"><td colspan=\"2\">"
                    "<nobr><font size=\"-1\" color=\"black\"><b>");
    QString headEnd("</b></font></nobr></td></tr>");

    QString cellBeg("<tr><td><nobr><font size=\"-1\" color=\"black\">");
    QString cellMid("</font></nobr></td>"
                    "<td><nobr><font size=\"-1\" color=\"black\">");
    QString cellEnd("</font></nobr></td></tr>");

    QString cellSpecBeg("<tr><td><nobr><font size=\"-1\" color=\"black\">");
    QString cellSpecMid("</font></nobr></td>"
                        "<td><nobr><font size=\"-1\" color=\"steelblue\"><i>");
    QString cellSpecEnd("</i></font></nobr></td></tr>");

    tip = "<table cellspacing=\"0\" cellpadding=\"0\" width=\"250\" border=\"0\">";

    AlbumSettings* settings = AlbumSettings::instance();
    const ImageInfo* info   = d->iconItem->imageInfo();
    QFileInfo fileInfo(info->kurl().path());
    KFileItem fi(KFileItem::Unknown, KFileItem::Unknown, info->kurl());
    DMetadata metaData(info->kurl().path());

    // -- File properties ----------------------------------------------

    if (settings->getToolTipsShowFileName()  ||
        settings->getToolTipsShowFileDate()  ||
        settings->getToolTipsShowFileSize()  ||
        settings->getToolTipsShowImageType() ||
        settings->getToolTipsShowImageDim())
    {
        tip += headBeg + i18n("File Properties") + headEnd;

        if (settings->getToolTipsShowFileName())
        {
            tip += cellBeg + i18n("Name:") + cellMid;
            tip += info->kurl().fileName() + cellEnd;
        }

        if (settings->getToolTipsShowFileDate())
        {
            QDateTime modifiedDate = fileInfo.lastModified();
            str = KGlobal::locale()->formatDateTime(modifiedDate, true, true);
            tip += cellBeg + i18n("Modified:") + cellMid + str + cellEnd;
        }

        if (settings->getToolTipsShowFileSize())
        {
            tip += cellBeg + i18n("Size:") + cellMid;
            str = i18n("%1 (%2)").arg(KIO::convertSize(fi.size()))
                                .arg(KGlobal::locale()->formatNumber(fi.size(), 0));
            tip += str + cellEnd;
        }

        QSize   dims;
#if KDCRAW_VERSION < 0x000106
        QString rawFilesExt(KDcrawIface::DcrawBinary::instance()->rawFiles());
#else
        QString rawFilesExt(KDcrawIface::KDcraw::rawFiles());
#endif
        QString ext = fileInfo.extension(false).upper();

        if (!ext.isEmpty() && rawFilesExt.upper().contains(ext))
        {
            str = i18n("RAW Image");
            dims = metaData.getImageDimensions();
        }
        else
        {
            str = fi.mimeComment();

            KFileMetaInfo meta = fi.metaInfo();
            if (meta.isValid())
            {
                if (meta.containsGroup("Jpeg EXIF Data"))
                    dims = meta.group("Jpeg EXIF Data").item("Dimensions").value().toSize();
                else if (meta.containsGroup("General"))
                    dims = meta.group("General").item("Dimensions").value().toSize();
                else if (meta.containsGroup("Technical"))
                    dims = meta.group("Technical").item("Dimensions").value().toSize();
            }
        }

        if (settings->getToolTipsShowImageType())
        {
            tip += cellBeg + i18n("Type:") + cellMid + str + cellEnd;
        }

        if (settings->getToolTipsShowImageDim())
        {
            QString mpixels;
            mpixels.setNum(dims.width()*dims.height()/1000000.0, 'f', 2);
            str = (!dims.isValid()) ? i18n("Unknown") : i18n("%1x%2 (%3Mpx)")
                .arg(dims.width()).arg(dims.height()).arg(mpixels);
            tip += cellBeg + i18n("Dimensions:") + cellMid + str + cellEnd;
        }
    }

    // -- Photograph Info ----------------------------------------------------
    // NOTE: If something is changed here, please updated imageproperties section too.

    if (settings->getToolTipsShowPhotoMake()  ||
        settings->getToolTipsShowPhotoDate()  ||
        settings->getToolTipsShowPhotoFocal() ||
        settings->getToolTipsShowPhotoExpo()  ||
        settings->getToolTipsShowPhotoMode()  ||
        settings->getToolTipsShowPhotoFlash() ||
        settings->getToolTipsShowPhotoWB())
    {
        PhotoInfoContainer photoInfo = metaData.getPhotographInformations();

        if (!photoInfo.isEmpty())
        {
            QString metaStr;
            tip += headBeg + i18n("Photograph Properties") + headEnd;

            if (settings->getToolTipsShowPhotoMake())
            {
                str = QString("%1 / %2").arg(photoInfo.make.isEmpty() ? unavailable : photoInfo.make)
                                        .arg(photoInfo.model.isEmpty() ? unavailable : photoInfo.model);
                if (str.length() > d->maxStringLen) str = str.left(d->maxStringLen-3) + "...";
                metaStr += cellBeg + i18n("Make/Model:") + cellMid + QStyleSheet::escape( str ) + cellEnd;
            }

            if (settings->getToolTipsShowPhotoDate())
            {
                if (photoInfo.dateTime.isValid())
                {
                    str = KGlobal::locale()->formatDateTime(photoInfo.dateTime, true, true);
                    if (str.length() > d->maxStringLen) str = str.left(d->maxStringLen-3) + "...";
                    metaStr += cellBeg + i18n("Created:") + cellMid + QStyleSheet::escape( str ) + cellEnd;
                }
                else
                    metaStr += cellBeg + i18n("Created:") + cellMid + QStyleSheet::escape( unavailable ) + cellEnd;
            }

            if (settings->getToolTipsShowPhotoFocal())
            {
                str = photoInfo.aperture.isEmpty() ? unavailable : photoInfo.aperture;

                if (photoInfo.focalLength35mm.isEmpty())
                    str += QString(" / %1").arg(photoInfo.focalLength.isEmpty() ? unavailable : photoInfo.focalLength);
                else 
                    str += QString(" / %1").arg(i18n("%1 (35mm: %2)").arg(photoInfo.focalLength).arg(photoInfo.focalLength35mm));

                if (str.length() > d->maxStringLen) str = str.left(d->maxStringLen-3) + "...";
                metaStr += cellBeg + i18n("Aperture/Focal:") + cellMid + QStyleSheet::escape( str ) + cellEnd;
            }

            if (settings->getToolTipsShowPhotoExpo())
            {
                str = QString("%1 / %2").arg(photoInfo.exposureTime.isEmpty() ? unavailable : photoInfo.exposureTime)
                                        .arg(photoInfo.sensitivity.isEmpty() ? unavailable : i18n("%1 ISO").arg(photoInfo.sensitivity));
                if (str.length() > d->maxStringLen) str = str.left(d->maxStringLen-3) + "...";
                metaStr += cellBeg + i18n("Exposure/Sensitivity:") + cellMid + QStyleSheet::escape( str ) + cellEnd;
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
                if (str.length() > d->maxStringLen) str = str.left(d->maxStringLen-3) + "...";
                metaStr += cellBeg + i18n("Mode/Program:") + cellMid + QStyleSheet::escape( str ) + cellEnd;
            }

            if (settings->getToolTipsShowPhotoFlash())
            {
                str = photoInfo.flash.isEmpty() ? unavailable : photoInfo.flash;
                if (str.length() > d->maxStringLen) str = str.left(d->maxStringLen-3) + "...";
                metaStr += cellBeg + i18n("Flash:") + cellMid + QStyleSheet::escape( str ) + cellEnd;
            }

            if (settings->getToolTipsShowPhotoWB())
            {
                str = photoInfo.whiteBalance.isEmpty() ? unavailable : photoInfo.whiteBalance;
                if (str.length() > d->maxStringLen) str = str.left(d->maxStringLen-3) + "...";
                metaStr += cellBeg + i18n("White Balance:") + cellMid + QStyleSheet::escape( str ) + cellEnd;
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
        tip += headBeg + i18n("digiKam Properties") + headEnd;

        if (settings->getToolTipsShowAlbumName())
        {
            PAlbum* album = info->album();
            if (album)
                tip += cellSpecBeg + i18n("Album:") + cellSpecMid + album->url().remove(0, 1) + cellSpecEnd;
        }

        if (settings->getToolTipsShowComments())
        {
            str = info->caption();
            if (str.isEmpty()) str = QString("---");
            tip += cellSpecBeg + i18n("Caption:") + cellSpecMid + breakString(str) + cellSpecEnd;
        }

        if (settings->getToolTipsShowTags())
        {
            QStringList tagPaths = info->tagPaths(false);

            str = tagPaths.join(", ");
            if (str.isEmpty()) str = QString("---");
            if (str.length() > d->maxStringLen) str = str.left(d->maxStringLen-3) + "...";
            tip += cellSpecBeg + i18n("Tags:") + cellSpecMid + str + cellSpecEnd;
        }

        if (settings->getToolTipsShowRating())
        {
            str.fill( 'X', info->rating() );
            if (str.isEmpty()) str = QString("---");
            tip += cellSpecBeg + i18n("Rating:") + cellSpecMid + str + cellSpecEnd;
        }
    }

    tip += "</table>";

    d->label->setText(tip);
}

QString AlbumFileTip::breakString(const QString& input)
{
    QString str = input.simplifyWhiteSpace();
    str = QStyleSheet::escape(str);
    uint maxLen = d->maxStringLen;

    if (str.length() <= maxLen)
        return str;

    QString br;

    uint i = 0;
    uint count = 0;

    while (i < str.length())
    {
        if (count >= maxLen && str[i].isSpace())
        {
            count = 0;
            br.append("<br>");
        }
        else
        {
            br.append(str[i]);
        }

        i++;
        count++;
    }

    return br;
}

}  // namespace Digikam
