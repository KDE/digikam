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

#include <QToolTip>
#include <QLabel>
#include <QPixmap>
#include <QDateTime>
#include <QPainter>
#include <QApplication>
#include <QVBoxLayout>
#include <QTextDocument>
#include <QFileInfo>

// KDE includes.

#include <kdebug.h>
#include <klocale.h>
#include <kfileitem.h>
#include <kmimetype.h>
#include <kglobalsettings.h>
#include <kglobal.h>
#include <kdeversion.h>

// LibKDcraw includes.

#include <libkdcraw/kdcraw.h>
#include <libkdcraw/version.h>

#if KDCRAW_VERSION < 0x000400
#include <libkdcraw/dcrawbinary.h>
#endif

// Local includes.

#include "thumbbar.h"
#include "themeengine.h"
#include "dmetadata.h"

namespace Digikam
{

class ThumbBarToolTipPriv
{
public:

    ThumbBarToolTipPriv() :
        maxStringLen(30), tipBorder(5)
    {
        corner = 0;
        label  = 0;
        view   = 0;
        item   = 0;
    }

    const  int     maxStringLen;
    const uint     tipBorder;

    int            corner;

    QLabel        *label;

    QPixmap        corners[4];

    ThumbBarView  *view;

    ThumbBarItem  *item;
};

ThumbBarToolTip::ThumbBarToolTip(ThumbBarView* view)
            : QFrame(0), m_maxStringLen(30), d (new ThumbBarToolTipPriv)
{
    m_headBeg = QString("<tr bgcolor=\"#73CAE6\"><td colspan=\"2\">"
                        "<nobr><font size=\"-1\" color=\"black\"><b>");
    m_headEnd = QString("</b></font></nobr></td></tr>");

    m_cellBeg = QString("<tr><td><nobr><font size=\"-1\" color=\"black\">");
    m_cellMid = QString("</font></nobr></td>"
                        "<td><nobr><font size=\"-1\" color=\"black\">");
    m_cellEnd = QString("</font></nobr></td></tr>");

    m_cellSpecBeg = QString("<tr><td><nobr><font size=\"-1\" color=\"black\">");
    m_cellSpecMid = QString("</font></nobr></td>"
                            "<td><nobr><font size=\"-1\" color=\"steelblue\"><i>");
    m_cellSpecEnd = QString("</i></font></nobr></td></tr>");

    d->view = view;
    hide();

    setPalette(QToolTip::palette());
    setFrameStyle(QFrame::Plain | QFrame::Box);
    setLineWidth(1);
    setWindowFlags(Qt::ToolTip);

    QVBoxLayout *layout = new QVBoxLayout(this);

    d->label = new QLabel(this);
    d->label->setMargin(0);
    d->label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    layout->addWidget(d->label);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    layout->setMargin(d->tipBorder+1);
    layout->setSpacing(0);

    renderArrows();
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
        d->label->setText(tipContents());
        reposition();
        if (isHidden())
            show();
    }
}

ThumbBarItem* ThumbBarToolTip::item() const
{
    return d->item;
}

void ThumbBarToolTip::reposition()
{
    if (!d->item)
        return;

    QRect rect = d->item->rect();
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

    QRect desk = KGlobalSettings::desktopGeometry(rect.center());

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

void ThumbBarToolTip::renderArrows()
{
    int w = d->tipBorder;

    // -- left top arrow -------------------------------------

    QPixmap& pix0 = d->corners[0];
    pix0          = QPixmap(w, w);
    pix0.fill(palette().color(QPalette::Background));

    QPainter p0(&pix0);
    p0.setPen(QPen(Qt::black, 1));

    for (int j=0; j<w; j++)
        p0.drawLine(0, j, w-j-1, j);

    p0.end();

    // -- right top arrow ------------------------------------

    QPixmap& pix1 = d->corners[1];
    pix1          = QPixmap(w, w);
    pix1.fill(palette().color(QPalette::Background));

    QPainter p1(&pix1);
    p1.setPen(QPen(Qt::black, 1));

    for (int j=0; j<w; j++)
        p1.drawLine(j, j, w-1, j);

    p1.end();

    // -- left bottom arrow ----------------------------------

    QPixmap& pix2 = d->corners[2];
    pix2          = QPixmap(w, w);
    pix2.fill(palette().color(QPalette::Background));

    QPainter p2(&pix2);
    p2.setPen(QPen(Qt::black, 1));

    for (int j=0; j<w; j++)
        p2.drawLine(0, j, j, j);

    p2.end();

    // -- right bottom arrow ---------------------------------

    QPixmap& pix3 = d->corners[3];
    pix3          = QPixmap(w, w);
    pix3.fill(palette().color(QPalette::Background));

    QPainter p3(&pix3);
    p3.setPen(QPen(Qt::black, 1));

    for (int j=0; j<w; j++)
        p3.drawLine(w-j-1, j, w-1, j);

    p3.end();
}

bool ThumbBarToolTip::event(QEvent *e)
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

void ThumbBarToolTip::resizeEvent(QResizeEvent* e)
{
    QFrame::resizeEvent(e);
    reposition();
}

void ThumbBarToolTip::paintEvent(QPaintEvent *e)
{
    QFrame::paintEvent(e);

    if (d->corner >= 4)
        return;

    QPainter p(this);
    QPixmap &pix = d->corners[d->corner];

    switch (d->corner)
    {
        case 0:
            p.drawPixmap( 3, 3, pix );
            break;
        case 1:
            p.drawPixmap( width() - pix.width() - 3, 3, pix );
            break;
        case 2:
            p.drawPixmap( 3, height() - pix.height() - 3, pix );
            break;
        case 3:
            p.drawPixmap( width() - pix.width() - 3, height() - pix.height() - 3, pix );
            break;
    }
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

QString ThumbBarToolTip::breakString(const QString& input)
{
    QString str = input.simplified();
    str         = Qt::escape(str);
    int maxLen  = d->maxStringLen;

    if (str.length() <= maxLen)
        return str;

    QString br;

    int i     = 0;
    int count = 0;

    while (i < str.length())
    {
        if (count >= maxLen && str[i].isSpace())
        {
            count = 0;
            br.append("<br/>");
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
