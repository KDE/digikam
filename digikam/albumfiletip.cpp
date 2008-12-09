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
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "albumfiletip.h"

// Qt includes.

#include <QToolTip>
#include <QLabel>
#include <QPixmap>
#include <QDateTime>
#include <QPainter>
#include <QApplication>
#include <QVBoxLayout>
#include <QTextDocument>

// KDE includes.

#include <kdebug.h>
#include <klocale.h>
#include <kfileitem.h>
#include <kmimetype.h>
#include <kglobalsettings.h>
#include <kglobal.h>
#include <kdeversion.h>

// Local includes.

#include "themeengine.h"
#include "dmetadata.h"
#include "albumiconview.h"
#include "albumiconitem.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "album.h"

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

    const  int     maxStringLen;
    const uint     tipBorder;

    int            corner;

    QLabel        *label;

    QPixmap        corners[4];

    AlbumIconView *view;

    AlbumIconItem *iconItem;
};

AlbumFileTip::AlbumFileTip(AlbumIconView* view)
            : QFrame(0), d(new AlbumFileTipPriv)
{
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

void AlbumFileTip::renderArrows()
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

void AlbumFileTip::paintEvent(QPaintEvent *e)
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

void AlbumFileTip::updateText()
{
    QString tip, str;
    QString unavailable(i18n("unavailable"));

    QString headBeg = QString("<tr bgcolor=\"%1\"><td colspan=\"2\">"
                              "<nobr><font size=\"-1\" color=\"%2\"><b>")
                              .arg(ThemeEngine::instance()->baseColor().name())
                              .arg(ThemeEngine::instance()->textRegColor().name());
    QString headEnd("</b></font></nobr></td></tr>");

    QString cellBeg = QString("<tr><td><nobr><font size=\"-1\" color=\"%1\">")
                              .arg(ThemeEngine::instance()->textRegColor().name());
    QString cellMid = QString("</font></nobr></td><td><nobr><font size=\"-1\" color=\"%1\">")
                              .arg(ThemeEngine::instance()->textRegColor().name());
    QString cellEnd("</font></nobr></td></tr>");

    QString cellSpecBeg = QString("<tr><td><nobr><font size=\"-1\" color=\"%1\">")
                                  .arg(ThemeEngine::instance()->textRegColor().name());
    QString cellSpecMid = QString("</font></nobr></td><td><nobr><font size=\"-1\" color=\"%1\"><i>")
                                  .arg(ThemeEngine::instance()->textSpecialRegColor().name());
    QString cellSpecEnd("</i></font></nobr></td></tr>");

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
        tip += headBeg + i18n("File Properties") + headEnd;

        if (settings->getToolTipsShowFileName())
        {
            tip += cellBeg + i18n("Name:") + cellMid;
            tip += commonInfo.fileName + cellEnd;
        }

        if (settings->getToolTipsShowFileDate())
        {
            QDateTime modifiedDate = commonInfo.fileModificationDate;
            str = KGlobal::locale()->formatDateTime(modifiedDate, KLocale::ShortDate, true);
            tip += cellBeg + i18n("Modified:") + cellMid + str + cellEnd;
        }

        if (settings->getToolTipsShowFileSize())
        {
            tip += cellBeg + i18n("Size:") + cellMid;
            str = i18n("%1 (%2)", KIO::convertSize(commonInfo.fileSize),
                                  KGlobal::locale()->formatNumber(commonInfo.fileSize, 0));
            tip += str + cellEnd;
        }

        QSize   dims;

        if (settings->getToolTipsShowImageType())
        {
            tip += cellBeg + i18n("Type:") + cellMid + commonInfo.format + cellEnd;
        }

        if (settings->getToolTipsShowImageDim())
        {
            if (commonInfo.width == 0 || commonInfo.height == 0)
                str = i18n("Unknown");
            else
            {
                QString mpixels;
                mpixels.setNum(commonInfo.width*commonInfo.height/1000000.0, 'f', 2);
                str = i18nc("width x height (megapixels Mpx)", "%1x%2 (%3Mpx)",
                            commonInfo.width, commonInfo.height, mpixels);
            }
            tip += cellBeg + i18n("Dimensions:") + cellMid + str + cellEnd;
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
            tip += headBeg + i18n("Photograph Properties") + headEnd;

            if (settings->getToolTipsShowPhotoMake())
            {
                str = QString("%1 / %2").arg(photoInfo.make.isEmpty() ? unavailable : photoInfo.make)
                                        .arg(photoInfo.model.isEmpty() ? unavailable : photoInfo.model);
                if (str.length() > d->maxStringLen) str = str.left(d->maxStringLen-3) + "...";
                metaStr += cellBeg + i18n("Make/Model:") + cellMid + Qt::escape( str ) + cellEnd;
            }

            if (settings->getToolTipsShowPhotoDate())
            {
                if (commonInfo.creationDate.isValid())
                {
                    str = KGlobal::locale()->formatDateTime(commonInfo.creationDate, KLocale::ShortDate, true);
                    if (str.length() > d->maxStringLen) str = str.left(d->maxStringLen-3) + "...";
                    metaStr += cellBeg + i18n("Created:") + cellMid + Qt::escape( str ) + cellEnd;
                }
                else
                    metaStr += cellBeg + i18n("Created:") + cellMid + Qt::escape( unavailable ) + cellEnd;
            }

            if (settings->getToolTipsShowPhotoFocal())
            {
                str = photoInfo.aperture.isEmpty() ? unavailable : photoInfo.aperture;

                if (photoInfo.focalLength35.isEmpty())
                    str += QString(" / %1").arg(photoInfo.focalLength.isEmpty() ? unavailable : photoInfo.focalLength);
                else
                    str += QString(" / %1").arg(i18n("%1 (35mm: %2)",photoInfo.focalLength,photoInfo.focalLength35));

                if (str.length() > d->maxStringLen) str = str.left(d->maxStringLen-3) + "...";
                metaStr += cellBeg + i18n("Aperture/Focal:") + cellMid + Qt::escape( str ) + cellEnd;
            }

            if (settings->getToolTipsShowPhotoExpo())
            {
                str = QString("%1 / %2").arg(photoInfo.exposureTime.isEmpty() ? unavailable : photoInfo.exposureTime)
                                        .arg(photoInfo.sensitivity.isEmpty() ? unavailable : i18n("%1 ISO",photoInfo.sensitivity));
                if (str.length() > d->maxStringLen) str = str.left(d->maxStringLen-3) + "...";
                metaStr += cellBeg + i18n("Exposure/Sensitivity:") + cellMid + Qt::escape( str ) + cellEnd;
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
                metaStr += cellBeg + i18n("Mode/Program:") + cellMid + Qt::escape( str ) + cellEnd;
            }

            if (settings->getToolTipsShowPhotoFlash())
            {
                str = photoInfo.flashMode.isEmpty() ? unavailable : photoInfo.flashMode;
                if (str.length() > d->maxStringLen) str = str.left(d->maxStringLen-3) + "...";
                metaStr += cellBeg + i18n("Flash:") + cellMid + Qt::escape( str ) + cellEnd;
            }

            if (settings->getToolTipsShowPhotoWB())
            {
                str = photoInfo.whiteBalance.isEmpty() ? unavailable : photoInfo.whiteBalance;
                if (str.length() > d->maxStringLen) str = str.left(d->maxStringLen-3) + "...";
                metaStr += cellBeg + i18n("White Balance:") + cellMid + Qt::escape( str ) + cellEnd;
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
            PAlbum* album = AlbumManager::instance()->findPAlbum(info.albumId());
            if (album)
                tip += cellSpecBeg + i18n("Album:") + cellSpecMid + album->albumPath().remove(0, 1) + cellSpecEnd;
        }

        if (settings->getToolTipsShowComments())
        {
            str = info.comment();
            if (str.isEmpty()) str = QString("---");
            tip += cellSpecBeg + i18n("Caption:") + cellSpecMid + breakString(str) + cellSpecEnd;
        }

        if (settings->getToolTipsShowTags())
        {
            QStringList tagPaths = AlbumManager::instance()->tagPaths(info.tagIds(), false);

            str = tagPaths.join(", ");
            if (str.isEmpty()) str = QString("---");
            if (str.length() > d->maxStringLen) str = str.left(d->maxStringLen-3) + "...";
            tip += cellSpecBeg + i18n("Tags:") + cellSpecMid + str + cellSpecEnd;
        }

        if (settings->getToolTipsShowRating())
        {
            int rating = info.rating();
            if (rating <= 0)
                str = QString("---");
            else
                str.fill( 'X', info.rating() );
            tip += cellSpecBeg + i18n("Rating:") + cellSpecMid + str + cellSpecEnd;
        }
    }

    tip += "</table>";

    d->label->setText(tip);
}

QString AlbumFileTip::breakString(const QString& input)
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
