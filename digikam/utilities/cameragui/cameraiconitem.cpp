/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-09-21
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#include <qpainter.h>
#include <qpixmap.h>
#include <thumbview.h>

#include "gpiteminfo.h"
#include "cameraiconitem.h"

const char* CameraIconViewItem::new_xpm[] =
{
    "13 13 8 1",
    "       c None",
    ".      c #232300",
    "+      c #F6F611",
    "@      c #000000",
    "#      c #DBDA4D",
    "$      c #FFFF00",
    "%      c #AAA538",
    "&      c #E8E540",
    "      .      ",
    "  .  .+.  .  ",
    " @#@ .$. .#. ",
    "  @$@#$#@$.  ",
    "   @$%&%$@   ",
    " ..#%&&&%#.. ",
    ".+$$&&&&&$$+@",
    " ..#%&&&%#@@ ",
    "   @$%&%$@   ",
    "  .$@#$#@$.  ",
    " @#. @$@ @#. ",
    "  .  @+@  .  ",
    "      @      "
};

QPixmap* CameraIconViewItem::m_newEmblem = 0;

CameraIconViewItem::CameraIconViewItem(ThumbView* parent,
                                       const GPItemInfo& itemInfo,
                                       const QPixmap& pix,
                                       const QString& downloadName)
    : ThumbItem(parent, itemInfo.name, pix)
{
    m_itemInfo     = new GPItemInfo(itemInfo);
    m_downloadName = downloadName;
    calcRect();
}

CameraIconViewItem::~CameraIconViewItem()
{
    delete m_itemInfo;
}

void CameraIconViewItem::calcRect()
{
    const int thumbSize = 128;
    
    QRect itemPixRect       = QRect(0,0,0,0);
    QRect itemTextRect      = QRect(0,0,0,0);
    QRect itemExtraTextRect = QRect(0,0,0,0);
    QRect itemRect          = rect();

    itemPixRect.setWidth(thumbSize);
    itemPixRect.setHeight(thumbSize);

    QFontMetrics fm(iconView()->font());
    QRect r = QRect(fm.boundingRect(0, 0, thumbSize, 0xFFFFFFFF,
                                    Qt::AlignHCenter | Qt::AlignTop |
                                    Qt::WordBreak | Qt::BreakAnywhere,
                                    m_itemInfo->name));
    itemTextRect.setWidth(r.width());
    itemTextRect.setHeight(r.height());

    if (!m_downloadName.isEmpty()) {

        QFont fn(iconView()->font());
        if (fn.pointSize() > 0)
        {
            fn.setPointSize(QMAX(fn.pointSize()-2, 6));
        }

        fm = QFontMetrics(fn);
        r = QRect(fm.boundingRect(0, 0, thumbSize, 0xFFFFFFFF,
                                  Qt::AlignHCenter | Qt::WordBreak |
                                  Qt::BreakAnywhere | Qt::AlignTop,
                                  m_downloadName));
        m_itemExtraRect.setWidth(r.width());
        m_itemExtraRect.setHeight(r.height());

        itemTextRect.setWidth(QMAX(itemTextRect.width(), m_itemExtraRect.width()));
        itemTextRect.setHeight(itemTextRect.height() + m_itemExtraRect.height());
    }
    
    int w = QMAX(itemTextRect.width(), itemPixRect.width() );
    int h = itemTextRect.height() + itemPixRect.height() ;

    itemRect.setWidth(w+4);
    itemRect.setHeight(h+4);

    // Center the pix and text rect
    itemPixRect = QRect(2, 2, itemPixRect.width(), itemPixRect.height());
    itemTextRect = QRect((itemRect.width() - itemTextRect.width())/2,
                         itemRect.height() - itemTextRect.height(),
                         itemTextRect.width(), itemTextRect.height());
    if (!m_itemExtraRect.isEmpty()) {
        m_itemExtraRect = QRect((itemRect.width() - m_itemExtraRect.width())/2,
                                itemRect.height() - m_itemExtraRect.height(),
                                m_itemExtraRect.width(), m_itemExtraRect.height());
    }

    // Update rects
    if ( itemPixRect != pixmapRect() )
        setPixmapRect( itemPixRect );
    if ( itemTextRect != textRect() )
        setTextRect( itemTextRect );
     setRect( itemRect );
}

void CameraIconViewItem::paintItem(QPainter *, const QColorGroup& cg)
{
    QRect pRect=pixmapRect(true);
    QRect tRect=textRect(true);
    QFont fn(iconView()->font());

    QPixmap pix(rect().width(), rect().height());
    pix.fill(cg.base());

    QPainter p(&pix);
    p.drawPixmap(pRect.x() + (pRect.width()-pixmap()->width())/2,
                 pRect.y() + (pRect.height()-pixmap()->height())/2,
                 *pixmap() );

    if (isSelected())
    {
        QPen pen;
        pen.setColor(cg.highlight());
        p.setPen(pen);
        p.drawRect(0, 0, pix.width(), pix.height());
        p.fillRect(0, tRect.y(), pix.width(),
                   tRect.height(), cg.highlight() );
        p.setPen( QPen( cg.highlightedText() ) );
    }
    else
    {
        QPen pen;
        pen.setColor(cg.button());
        p.setPen(pen);
        p.drawRect(0, 0, pix.width(), pix.height());
        p.fillRect(0, tRect.y(), pix.width(),
                   tRect.height(), cg.button() );
        p.setPen( cg.text() );
    }

    p.drawText(tRect, Qt::WordBreak|Qt::BreakAnywhere|
               Qt::AlignHCenter|Qt::AlignTop, m_itemInfo->name);

    if (!m_downloadName.isEmpty()) {

        if (fn.pointSize() > 0)
        {
            fn.setPointSize(QMAX(fn.pointSize()-2, 6));
        }

        p.setFont(fn);
        if (!isSelected())
            p.setPen(QPen("steelblue"));
        p.drawText(m_itemExtraRect, Qt::WordBreak|
                   Qt::BreakAnywhere|Qt::AlignHCenter|
                   Qt::AlignTop,m_downloadName);
    }

    if (m_itemInfo->downloaded == 0)
    {
        int x = rect().width() - m_newEmblem->width() - 5;
        int y = 5;
        p.drawPixmap(x, y, *m_newEmblem);
    }
    
    p.end();

    QRect r(rect());
    r = QRect(iconView()->contentsToViewport(QPoint(r.x(), r.y())),
              QSize(r.width(), r.height()));

    bitBlt(iconView()->viewport(), r.x(), r.y(), &pix,
           0, 0, r.width()+4, r.height()+4);
}

void CameraIconViewItem::setDownloadName(const QString& downloadName)
{
    m_downloadName = downloadName;
    calcRect();
    repaint();
}

QString CameraIconViewItem::getDownloadName() const
{
    return m_downloadName;
}

void CameraIconViewItem::setDownloaded()
{
    if (m_itemInfo->downloaded != 1)
    {
        m_itemInfo->downloaded = 1;
        repaint();
    }
}
