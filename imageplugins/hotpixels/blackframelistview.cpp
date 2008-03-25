/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-05
 * Description : a ListView to display black frames
 * 
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2005-2006 by Unai Garro <ugarro at users dot sourceforge dot net>
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

#define THUMB_WIDTH 150

// Qt includes.

#include <qpainter.h>
#include <qtooltip.h>

// Local includes.

#include "blackframelistview.h"
#include "blackframelistview.moc"

namespace DigikamHotPixelsImagesPlugin
{

BlackFrameListView::BlackFrameListView(QWidget* parent)
                  : QListView(parent)
{
    addColumn(i18n("Preview"));
    addColumn(i18n("Size"));
    addColumn(i18n("This is a column which will contain the amount of HotPixels "
                   "found in the black frame file", "HP"));
    setAllColumnsShowFocus(true);
    setResizeMode(QListView::LastColumn);
    setSelectionMode(QListView::Single);
}

// --------------------------------------------------------------------------

BlackFrameListViewItem::BlackFrameListViewItem(BlackFrameListView* parent, const KURL &url)
                      : QObject(parent), QListViewItem(parent)
{
    m_parent        = parent;
    m_blackFrameURL = url;
    m_parser        = new BlackFrameParser(parent);
    m_parser->parseBlackFrame(url);

    connect(m_parser, SIGNAL(parsed(QValueList<HotPixel>)),
            this, SLOT(slotParsed(QValueList<HotPixel>)));

    connect(this, SIGNAL(parsed(QValueList<HotPixel>, const KURL&)),
            parent, SLOT(slotParsed(QValueList<HotPixel>, const KURL&)));

    connect(m_parser, SIGNAL(signalLoadingProgress(float)),
            this, SIGNAL(signalLoadingProgress(float)));

    connect(m_parser, SIGNAL(signalLoadingComplete()),
            this, SIGNAL(signalLoadingComplete()));
}

void BlackFrameListViewItem::activate()
{
    QToolTip::add( m_parent, m_blackFrameDesc);
    emit parsed(m_hotPixels, m_blackFrameURL);
}

QString BlackFrameListViewItem::text(int column)const
{
    switch (column)
    {
        case 0:
        {
            // First column includes the pixmap
            break;
        }
        case 1:
        {
            // The image size.
            if (!m_imageSize.isEmpty())
                return (QString("%1x%2").arg(m_imageSize.width()).arg(m_imageSize.height())); 
            break;
        }
        case 2:
        {
            // The amount of hot pixels found in the black frame.
            return (QString::number(m_hotPixels.count())); 
            break;
        }
    }

    return QString();
}

void BlackFrameListViewItem::paintCell(QPainter* p, const QColorGroup& cg, int column, int width, int align)
{
    //Let the normal listview item draw it all for now
    QListViewItem::paintCell(p, cg, column, width, align);
}

void BlackFrameListViewItem::slotParsed(QValueList<HotPixel> hotPixels)
{
    m_hotPixels = hotPixels;
    m_image     = m_parser->image();
    m_imageSize = m_image.size();
    m_thumb     = thumb(QSize(THUMB_WIDTH, THUMB_WIDTH/3*2));
    setPixmap(0, m_thumb);

    m_blackFrameDesc = QString("<p><b>" + m_blackFrameURL.fileName() + "</b>:<p>");
    QValueList <HotPixel>::Iterator end(m_hotPixels.end());
    for (QValueList <HotPixel>::Iterator it = m_hotPixels.begin() ; it != end ; ++it)
        m_blackFrameDesc.append( QString("[%1,%2] ").arg((*it).x()).arg((*it).y()) );

    emit parsed(m_hotPixels, m_blackFrameURL);
}

QPixmap BlackFrameListViewItem::thumb(const QSize& size)
{
    QPixmap thumb;

    //First scale it down to the size
    thumb = m_image.smoothScale(size, QImage::ScaleMin);

    //And draw the hot pixel positions on the thumb
    QPainter p(&thumb);

    //Take scaling into account
    float xRatio, yRatio;
    float hpThumbX, hpThumbY;
    QRect hpRect;

    xRatio = (float)size.width()/(float)m_image.width();
    yRatio = (float)size.height()/(float)m_image.height();

    //Draw hot pixels one by one
    QValueList <HotPixel>::Iterator it;    
    QValueList <HotPixel>::Iterator end(m_hotPixels.end()); 
    for (it=m_hotPixels.begin() ; it!=end ; ++it)
    {
        hpRect   = (*it).rect;
        hpThumbX = (hpRect.x()+hpRect.width()/2)*xRatio;
        hpThumbY = (hpRect.y()+hpRect.height()/2)*yRatio;

        p.setPen(QPen(Qt::black));
        p.drawLine((int)hpThumbX, (int)hpThumbY-1, (int)hpThumbX, (int)hpThumbY+1);
        p.drawLine((int)hpThumbX-1, (int)hpThumbY, (int)hpThumbX+1, (int)hpThumbY);
        p.setPen(QPen(Qt::white));
        p.drawPoint((int)hpThumbX-1, (int)hpThumbY-1);
        p.drawPoint((int)hpThumbX+1, (int)hpThumbY+1);
        p.drawPoint((int)hpThumbX-1, (int)hpThumbY+1);
        p.drawPoint((int)hpThumbX+1, (int)hpThumbY-1);
    }

    return thumb;
}

int BlackFrameListViewItem::width(const QFontMetrics& fm,const QListView* lv,int c)const
{
    if (c==0) return THUMB_WIDTH;
    else return QListViewItem::width(fm,lv,c);
}

}  // NameSpace DigikamHotPixelsImagesPlugin
