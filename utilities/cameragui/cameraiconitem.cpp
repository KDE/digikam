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

// Qt includes.

#include <qpainter.h>
#include <qpixmap.h>

// Local includes.

#include "iconview.h"
#include "gpiteminfo.h"
#include "cameraiconitem.h"

namespace Digikam
{

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

CameraIconViewItem::CameraIconViewItem(IconGroupItem* parent,
                                       const GPItemInfo& itemInfo,
                                       const QPixmap& pix,
                                       const QString& downloadName)
    : IconItem(parent)
{
    m_itemInfo     = new GPItemInfo(itemInfo);
    m_downloadName = downloadName;
    m_pixmap       = pix;

    calcRect();
}

CameraIconViewItem::~CameraIconViewItem()
{
    delete m_itemInfo;
}

void CameraIconViewItem::setPixmap(const QPixmap& pixmap)
{
    m_pixmap = pixmap;
}

void CameraIconViewItem::paintItem()
{
    IconView* view = iconView();
    QColorGroup cg = view->colorGroup();
    QFont fn(view->font());

    QRect r(rect());

    QPixmap pix(r.width(), r.height());
    pix.fill(isSelected() ? view->colorGroup().highlight() :
             view->colorGroup().base());

    QPainter p(&pix);

    p.drawPixmap(m_pixRect.x() + (m_pixRect.width() - m_pixmap.width())/2,
                 m_pixRect.y() + (m_pixRect.height() - m_pixmap.height())/2,
                 m_pixmap);

    if (isSelected())
    {
        QPen pen;
        pen.setColor(cg.highlight());
        p.setPen(pen);
        p.drawRect(0, 0, pix.width(), pix.height());
        p.fillRect(0, m_textRect.y(), pix.width(),
                   m_textRect.height(), cg.highlight() );
        p.setPen( QPen( cg.highlightedText() ) );
    }
    else
    {
        QPen pen;
        pen.setColor(cg.button());
        p.setPen(pen);
        p.drawRect(0, 0, pix.width(), pix.height());
        p.fillRect(0, m_textRect.y(), pix.width(),
                   m_textRect.height(), cg.button() );
        p.setPen( cg.text() );
    }
    

    p.drawText(m_textRect, Qt::WordBreak|Qt::BreakAnywhere|
               Qt::AlignHCenter|Qt::AlignTop, m_itemInfo->name);

    if (!m_downloadName.isEmpty())
    {
        if (fn.pointSize() > 0)
        {
            fn.setPointSize(QMAX(fn.pointSize()-2, 6));
        }

        p.setFont(fn);
        if (!isSelected())
            p.setPen(QPen("steelblue"));
        p.drawText(m_extraRect, Qt::WordBreak|
                   Qt::BreakAnywhere|Qt::AlignHCenter|
                   Qt::AlignTop,m_downloadName);
    }

    if (this == iconView()->currentItem())
    {
        p.setPen(QPen(isSelected() ? Qt::white : Qt::black,
                      1, Qt::DotLine));
        p.drawRect(0, 0, r.width(), r.height());
    }
    
    if (m_itemInfo->downloaded == 0)
    {
        int x = rect().width() - m_newEmblem->width() - 5;
        int y = 5;
        p.drawPixmap(x, y, *m_newEmblem);
    }
    
    p.end();

    r = QRect(view->contentsToViewport(QPoint(r.x(), r.y())),
              QSize(r.width(), r.height()));

    bitBlt(view->viewport(), r.x(), r.y(), &pix);
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

void CameraIconViewItem::calcRect()
{
    const int thumbSize = 128;
    
    m_pixRect       = QRect(0,0,0,0);
    m_textRect      = QRect(0,0,0,0);
    m_extraRect     = QRect(0,0,0,0);
    QRect itemRect  = rect();
    itemRect.moveTopLeft(QPoint(0,0));

    m_pixRect.setWidth(thumbSize);
    m_pixRect.setHeight(thumbSize);

    QFontMetrics fm(iconView()->font());
    QRect r = QRect(fm.boundingRect(0, 0, thumbSize, 0xFFFFFFFF,
                                    Qt::AlignHCenter | Qt::AlignTop |
                                    Qt::WordBreak | Qt::BreakAnywhere,
                                    m_itemInfo->name));
    m_textRect.setWidth(r.width());
    m_textRect.setHeight(r.height());

    if (!m_downloadName.isEmpty())
    {
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
        m_extraRect.setWidth(r.width());
        m_extraRect.setHeight(r.height());

        m_textRect.setWidth(QMAX(m_textRect.width(), m_extraRect.width()));
        m_textRect.setHeight(m_textRect.height() + m_extraRect.height());
    }
    
    int w = QMAX(m_textRect.width(), m_pixRect.width() );
    int h = m_textRect.height() + m_pixRect.height() ;

    itemRect.setWidth(w+4);
    itemRect.setHeight(h+4);

    // Center the pix and text rect
    m_pixRect = QRect(2, 2, m_pixRect.width(), m_pixRect.height());
    m_textRect = QRect((itemRect.width() - m_textRect.width())/2,
                       itemRect.height() - m_textRect.height(),
                       m_textRect.width(), m_textRect.height());

    if (!m_extraRect.isEmpty())
    {
        m_extraRect = QRect((itemRect.width() - m_extraRect.width())/2,
                            itemRect.height() - m_extraRect.height(),
                            m_extraRect.width(), m_extraRect.height());
    }
}

QRect CameraIconViewItem::clickToOpenRect()
{
    QRect  r(rect());
    
    if (m_pixmap.isNull())
    {
        QRect  pixRect(m_pixRect);
        pixRect.moveBy(r.x(), r.y());
        return pixRect;
    }


    QRect pixRect(m_pixRect.x() + (m_pixRect.width() - m_pixmap.width())/2,
                  m_pixRect.y() + (m_pixRect.height() - m_pixmap.height())/2,
                  m_pixmap.width(),
                  m_pixmap.height());
    pixRect.moveBy(r.x(), r.y());
    return pixRect;
}

}  // namespace Digikam

