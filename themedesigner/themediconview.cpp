/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-15
 * Description : themed icon view 
 * 
 * Copyright (C) 2005 by Renchi Raju <renchi at pooh.tam.uiuc.edu>
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

// Local includes.

#include "themeengine.h"
#include "themedicongroupitem.h"
#include "themediconitem.h"
#include "themediconview.h"
#include "themediconview.moc"

namespace Digikam
{

class ThemedIconViewPriv
{
public:
    
    int   thumbSize;
    
    QRect itemRect;
    QRect itemDateRect;
    QRect itemPixmapRect;
    QRect itemNameRect;
    QRect itemCommentsRect;
    QRect itemResolutionRect;
    QRect itemSizeRect;
    QRect itemTagRect;
    QRect bannerRect;

    QPixmap itemRegPixmap;
    QPixmap itemSelPixmap;
    QPixmap bannerPixmap;

    QFont fnReg;
    QFont fnCom;
    QFont fnXtra;
};

ThemedIconView::ThemedIconView(QWidget* parent)
              : IconView(parent)
{
    d = new ThemedIconViewPriv;
    d->thumbSize = 128;


    ThemedIconGroupItem* groupItem = new ThemedIconGroupItem(this);
    for (int i=0; i<10; i++)
    {
        ThemedIconItem* item = new ThemedIconItem(groupItem);
        if (i > 0 && i < 3)
            item->setSelected(true, false);
    }
    
    updateBannerRectPixmap();
    updateItemRectsPixmap();

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));
}

ThemedIconView::~ThemedIconView()
{    
    delete d;
}

QRect ThemedIconView::itemRect() const
{
    return d->itemRect;
}

QRect ThemedIconView::itemDateRect() const
{
    return d->itemDateRect;
}

QRect ThemedIconView::itemPixmapRect() const
{
    return d->itemPixmapRect;
}

QRect ThemedIconView::itemNameRect() const
{
    return d->itemNameRect;
}

QRect ThemedIconView::itemCommentsRect() const
{
    return d->itemCommentsRect;
}

QRect ThemedIconView::itemResolutionRect() const
{
    return d->itemResolutionRect;
}

QRect ThemedIconView::itemTagRect() const
{
    return d->itemTagRect;
}

QRect ThemedIconView::itemSizeRect() const
{
    return d->itemSizeRect;
}

QRect ThemedIconView::bannerRect() const
{
    return d->bannerRect;
}

QPixmap* ThemedIconView::itemBaseRegPixmap() const
{
    return &d->itemRegPixmap;
}

QPixmap* ThemedIconView::itemBaseSelPixmap() const
{
    return &d->itemSelPixmap;
}

QPixmap ThemedIconView::bannerPixmap() const
{
    return d->bannerPixmap;
}

QFont ThemedIconView::itemFontReg() const
{
    return d->fnReg;
}

QFont ThemedIconView::itemFontCom() const
{
    return d->fnCom;
}

QFont ThemedIconView::itemFontXtra() const
{
    return d->fnXtra;
}

void ThemedIconView::slotThemeChanged()
{
    updateBannerRectPixmap();
    updateItemRectsPixmap();

    viewport()->update();
}

void ThemedIconView::resizeEvent(QResizeEvent* e)
{
    IconView::resizeEvent(e);

    if (d->bannerRect.width() != frameRect().width())
        updateBannerRectPixmap();
}

void ThemedIconView::updateBannerRectPixmap()
{
    d->bannerRect = QRect(0, 0, 0, 0);

    // Title --------------------------------------------------------
    QFont fn(font());
    int fnSize = fn.pointSize();
    bool usePointSize;
    if (fnSize > 0)
    {
        fn.setPointSize(fnSize+2);
        usePointSize = true;
    }
    else
    {
        fnSize = fn.pixelSize();
        fn.setPixelSize(fnSize+2);
        usePointSize = false;
    }

    fn.setBold(true);
    QFontMetrics fm(fn);
    QRect tr = fm.boundingRect(0, 0, frameRect().width(),
                               0xFFFFFFFF, Qt::AlignLeft | Qt::AlignVCenter,
                               "XXX");
    d->bannerRect.setHeight(tr.height());

    if (usePointSize)
        fn.setPointSize(font().pointSize());
    else
        fn.setPixelSize(font().pixelSize());

    fn.setBold(false);
    fm = QFontMetrics(fn);

    tr = fm.boundingRect(0, 0, frameRect().width(),
                         0xFFFFFFFF, Qt::AlignLeft | Qt::AlignVCenter,
                         "XXX");

    d->bannerRect.setHeight(d->bannerRect.height() + tr.height() + 10);
    d->bannerRect.setWidth(frameRect().width());

    d->bannerPixmap = ThemeEngine::instance()->bannerPixmap(d->bannerRect.width(),
                                                            d->bannerRect.height());
}

void ThemedIconView::updateItemRectsPixmap()
{
    d->itemRect = QRect(0,0,0,0);
    d->itemDateRect = QRect(0,0,0,0);
    d->itemPixmapRect = QRect(0,0,0,0);
    d->itemNameRect = QRect(0,0,0,0);
    d->itemCommentsRect = QRect(0,0,0,0);
    d->itemResolutionRect = QRect(0,0,0,0);
    d->itemSizeRect = QRect(0,0,0,0);
    d->itemTagRect = QRect(0,0,0,0);

    d->fnReg  = font();
    d->fnCom  = font();
    d->fnXtra = font();
    d->fnCom.setItalic(true);

    int fnSz = d->fnReg.pointSize();
    if (fnSz > 0)
    {
        d->fnCom.setPointSize(fnSz-1);
        d->fnXtra.setPointSize(fnSz-2);
    }
    else
    {
        fnSz = d->fnReg.pixelSize();
        d->fnCom.setPixelSize(fnSz-1);
        d->fnXtra.setPixelSize(fnSz-2);
    }

    int margin  = 5;
    int w = d->thumbSize + 2*margin;

    QFontMetrics fm(d->fnReg);
    QRect oneRowRegRect = fm.boundingRect(0, 0, w, 0xFFFFFFFF,
                                          Qt::AlignTop | Qt::AlignHCenter,
                                          "XXXXXXXXX");
    fm = QFontMetrics(d->fnCom);
    QRect oneRowComRect = fm.boundingRect(0, 0, w, 0xFFFFFFFF,
                                          Qt::AlignTop | Qt::AlignHCenter,
                                          "XXXXXXXXX");
    fm = QFontMetrics(d->fnXtra);
    QRect oneRowXtraRect = fm.boundingRect(0, 0, w, 0xFFFFFFFF,
                                           Qt::AlignTop | Qt::AlignHCenter,
                                           "XXXXXXXXX");

    int y = margin;

    d->itemPixmapRect = QRect(margin, y, w, d->thumbSize+margin);
    y = d->itemPixmapRect.bottom();

    {
        d->itemNameRect = QRect(margin, y, w, oneRowRegRect.height());
        y = d->itemNameRect.bottom();
    }

    {
        d->itemCommentsRect = QRect(margin, y, w, oneRowComRect.height());
        y = d->itemCommentsRect.bottom();
    }

    {
        d->itemDateRect = QRect(margin, y, w, oneRowXtraRect.height());
        y = d->itemDateRect.bottom();
    }

    {
        d->itemTagRect = QRect(margin, y, w, oneRowComRect.height());
        y = d->itemTagRect.bottom();
    }

    d->itemRect = QRect(0, 0, w+2*margin, y+margin);

    d->itemRegPixmap = ThemeEngine::instance()->thumbRegPixmap(d->itemRect.width(),
                                                               d->itemRect.height());

    d->itemSelPixmap = ThemeEngine::instance()->thumbSelPixmap(d->itemRect.width(),
                                                               d->itemRect.height());
}

}  // NameSpace Digikam
