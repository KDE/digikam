/* ============================================================
 * File  : albumfiletip.cpp
 * Date  : 2004-08-19
 * Description : 
 * 
 * Adapted from kfiletip (konqueror - konq_iconviewwidget.cc)
 *
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 * Copyright (C) 2000, 2001, 2002 David Faure <david@mandrakesoft.com>  
 * Copyright (C) 2004 by Renchi Raju<renchi@pooh.tam.uiuc.edu>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#include <qtooltip.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qdatetime.h>
#include <qstylesheet.h>

#include <klocale.h>
#include <kfileitem.h>
#include <kfilemetainfo.h>
#include <kglobalsettings.h>

#include "albumiconview.h"
#include "albumiconitem.h"
#include "albumlister.h"
#include "album.h"

#include "albumfiletip.h"

AlbumFileTip::AlbumFileTip(AlbumIconView* view)
    : QFrame( 0, 0, WStyle_Customize | WStyle_NoBorder | WStyle_Tool |
              WStyle_StaysOnTop | WX11BypassWM )
{
    m_view     = view;
    m_iconItem = 0;
    hide();

    setPalette( QToolTip::palette() );
    setFrameStyle( QFrame::Plain | QFrame::Box );
    setLineWidth( 1 );

    m_label = new QLabel(this);
    m_label->setMargin(0);
    m_label->setAlignment(Qt::AlignAuto | Qt::AlignVCenter);

    QVBoxLayout *layout = new QVBoxLayout(this, 5, 0);
    layout->addWidget(m_label);
    layout->setResizeMode(QLayout::Fixed);
}

AlbumFileTip::~AlbumFileTip()
{
}

void AlbumFileTip::setIconItem(AlbumIconItem* iconItem)
{
    m_iconItem = iconItem;

    if (!m_iconItem)
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
    QRect rect = m_iconItem->rect();
    QPoint off = m_view->mapToGlobal( m_view->contentsToViewport( QPoint( 0, 0 ) ) );
    rect.moveBy( off.x(), off.y() );

    QPoint pos = rect.center();

    // should the tooltip be shown to the left or to the right of the ivi ?
    QRect desk = KGlobalSettings::desktopGeometry(rect.center());
    if (rect.center().x() + width() > desk.right())
    {
        // to the left
        if (pos.x() - width() < 0) {
            pos.setX(0);
        } else {
            pos.setX( pos.x() - width() );
        }
    }
    // should the tooltip be shown above or below the ivi ?
    if (rect.bottom() + height() > desk.bottom())
    {
        // above
        pos.setY( rect.top() - height() );
    }
    else pos.setY( rect.bottom() );

    move( pos );
    
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

    const KFileItem* fi = m_iconItem->fileItem();
    
    // File properties ----------------------------------------------
    
    tip += headBeg + QString("File Properties") + headEnd;

    tip += cellBeg + i18n("Name:") + cellMid;
    tip += fi->url().fileName() + cellEnd;

    QDateTime date;
    date.setTime_t(fi->time(KIO::UDS_MODIFICATION_TIME));
    tip += cellBeg + i18n("Date:") + cellMid +
           dateToString(date) + cellEnd;
           
    tip += cellBeg + i18n("Size:") + cellMid;
    tip += QString::number(fi->size()) + QString(" bytes") + cellEnd;

    tip += cellBeg + i18n("Owner:") + cellMid +
           fi->user() + QString(" - ") + fi->group() + cellEnd;

    tip += cellBeg + i18n("Permissions:") + cellMid +
           fi->permissionsString() + cellEnd;

    // Digikam properties  ------------------------------------------

    tip += headBeg + QString("DigiKam properties") + headEnd;

    PAlbum* album = m_view->albumLister()->findParentAlbum(fi);
    if (album)
    {
        tip += cellBeg + i18n("Album:") + cellMid +
               album->getURL().remove(0,1) + cellEnd;
    }
    
    tip += cellSpecBeg + i18n("Comments:") + cellSpecMid +
           m_view->itemComments(m_iconItem) + cellSpecEnd;

    QStringList tagPaths(m_view->itemTagPaths(m_iconItem));
    for (QStringList::iterator it = tagPaths.begin(); it != tagPaths.end(); ++it)
    {
        (*it).remove(0,1);
    }
    tip += cellBeg + i18n("Tags:") + cellMid +
           tagPaths.join(",<br>") + cellEnd;

    // Meta Info ----------------------------------------------------

    QString metaStr;
    KFileMetaInfo info(fi->metaInfo());

    if (info.isValid() && !info.isEmpty() )
    {
        QStringList keys = info.preferredKeys();
        int maxCount = 6;
        int count = 0;
        
        for (QStringList::iterator it = keys.begin();
             count < maxCount && it!=keys.end() ; ++it)
        {
            KFileMetaInfoItem item = info.item( *it );
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
        tip += headBeg + QString("Meta Information") + headEnd;
        tip += metaStr;
    }
    
    tip += "</table>";

    
    m_label->setText(tip);
}

QString AlbumFileTip::dateToString(const QDateTime& datetime) const
{
    QString str;
    
    int hr = datetime.time().hour();
    hr = (hr > 12) ? (hr - 12) : hr;

    str.sprintf("%4d/%2.2d/%2.2d, %2.2d:%2.2d%s",
                datetime.date().year(),
                datetime.date().month(),
                datetime.date().day(),
                hr,
                datetime.time().minute(),
                datetime.time() > QTime(12,0) ? "PM" : "AM");

    return str;
}
