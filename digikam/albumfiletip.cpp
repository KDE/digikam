/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2004-08-19
 * Description : Album item file tip adapted from kfiletip 
 *               (konqueror - konq_iconviewwidget.cc)
 *
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 * Copyright (C) 2000, 2001, 2002 David Faure <david@mandrakesoft.com>
 * Copyright (C) 2004-2005 by Renchi Raju 
 * Copyright (C) 2006 by Gilles Caulier 
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
#include <qlabel.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qdatetime.h>
#include <qstylesheet.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qstylesheet.h>

// KDE includes.

#include <klocale.h>
#include <kfileitem.h>
#include <kfilemetainfo.h>
#include <kmimetype.h>
#include <kglobalsettings.h>
#include <kglobal.h>
#include <kdeversion.h>

// Local includes.

#include "albumiconview.h"
#include "albumiconitem.h"
#include "album.h"
#include "albumfiletip.h"

namespace Digikam
{

class AlbumFileTipPriv
{
public:

    AlbumFileTipPriv()
    {
        corner   = 0;
        label    = 0;
        view     = 0;
        iconItem = 0;
    }

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

    setPalette( QToolTip::palette() );
    setFrameStyle( QFrame::Plain | QFrame::Box );
    setLineWidth( 1 );

    d->label = new QLabel(this);
    d->label->setMargin(0);
    d->label->setAlignment(Qt::AlignAuto | Qt::AlignVCenter);

    QVBoxLayout *layout = new QVBoxLayout(this, 10, 0);
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

    if (!d->iconItem)
        hide();
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

    QRect rect = d->iconItem->rect();
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
        if (pos.x() - width() < 0) {
            pos.setX(0);
            d->corner = 4;
        } else {
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
    int w = 10;

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

    QFrame::drawContents( p );
}

void AlbumFileTip::updateText()
{
    QString tip;

    QString headBeg("<tr bgcolor=\"orange\"><td colspan=2>"
                    "<nobr><font size=-2 color=\"black\"><i>");
    QString headEnd("</i></font></nobr></td></nobr</tr>");

    QString cellBeg("<tr><td><nobr><font size=-1 color=\"black\">");
    QString cellMid("</font></nobr></td>"
                    "<td><nobr><font size=-1 color=\"black\">");
    QString cellEnd("</font></nobr></td></tr>");

    QString cellSpecBeg("<tr><td><nobr><font size=-1 color=\"black\">");
    QString cellSpecMid("</font></nobr></td>"
                        "<td><nobr><font size=-1 color=\"steelblue\"><i>");
    QString cellSpecEnd("</i></font></nobr></td></tr>");

    tip = "<table cellspacing=0 cellpadding=0>";

    const ImageInfo* info = d->iconItem->imageInfo();

    // -- File properties ----------------------------------------------

    tip += headBeg + i18n("File Properties") + headEnd;

    tip += cellBeg + i18n("Name:") + cellMid;
    tip += info->kurl().fileName() + cellEnd;
    
    tip += cellBeg + i18n("Type:") + cellMid;
    tip += KMimeType::findByURL(info->kurl())->comment() + cellEnd;

    QDateTime date = info->dateTime();
    tip += cellBeg + i18n("Date:") + cellMid +
           KGlobal::locale()->formatDateTime(date, true, true)
           + cellEnd;

    tip += cellBeg + i18n("Size:") + cellMid;
    tip += i18n("%1 (%2)")
           .arg(KIO::convertSize(info->fileSize()))
           .arg(KGlobal::locale()->formatNumber(info->fileSize(), 0))
           + cellEnd;

    // -- Digikam properties  ------------------------------------------

    tip += headBeg + i18n("digiKam Properties") + headEnd;

    PAlbum* album = info->album();
    if (album)
    {
        tip += cellBeg + i18n("Album:") + cellMid +
               album->url().remove(0,1) + cellEnd;
    }

    tip += cellSpecBeg + i18n("Comments:") + cellSpecMid +
           breakString( info->caption() ) + cellSpecEnd;

    QStringList tagPaths = info->tagPaths();
    for (QStringList::iterator it = tagPaths.begin(); it != tagPaths.end(); ++it)
    {
        (*it).remove(0,1);
    }
    tip += cellBeg + i18n("Tags:") + cellMid +
           tagPaths.join(",<br>") + cellEnd;

    // -- Meta Info ----------------------------------------------------

    QString metaStr;
    KFileMetaInfo metainfo(info->kurl().path());

    if (metainfo.isValid() && !metainfo.isEmpty() )
    {
        QStringList keys = metainfo.preferredKeys();
        int maxCount = 5;
        int count = 0;

        for (QStringList::iterator it = keys.begin();
             count < maxCount && it!=keys.end() ; ++it)
        {
            KFileMetaInfoItem item = metainfo.item( *it );
            if ( item.isValid() )
            {
                QString s = item.string();
                if (s.length() > 50) s = s.left(47) + "...";
                if ( !s.isEmpty() )
                {
                    count++;
                    metaStr += cellBeg +
                               QStyleSheet::escape( item.translatedKey() ) +
                               ":" + cellMid +
                               QStyleSheet::escape( s ) + cellEnd;
                }
            }
        }
    }

    if (!metaStr.isEmpty())
    {
        tip += headBeg + i18n("Meta Information") + headEnd;
        tip += metaStr;
    }

    tip += "</table>";

    d->label->setText(tip);
}

QString AlbumFileTip::breakString(const QString& input)
{
    QString str = input.simplifyWhiteSpace();

    str = QStyleSheet::escape(str);
                 
    uint maxLen = 30;

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
