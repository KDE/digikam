/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-05-31
 * Description : 
 * 
 * Copyright 2005 by Renchi Raju

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

#include <qfont.h>
#include <qfontmetrics.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qstyle.h>

#include "folderview.h"
#include "folderitem.h"

FolderItem::FolderItem(QListView* parent, const QString& text, bool special)
    : QListViewItem(parent, text), m_special(special)
{
}

FolderItem::FolderItem(QListViewItem* parent, const QString& text, bool special)
    : QListViewItem(parent, text), m_special(special)
{
}

FolderItem::~FolderItem()
{
}

void FolderItem::paintCell(QPainter* p, const QColorGroup & cg, int column,
                           int width, int)
{
    FolderView *fv = dynamic_cast<FolderView*>(listView());
    if (!fv)
        return;

    QFontMetrics fm(p->fontMetrics());

    QString t = text(column);

    int margin = fv->itemMargin();
    int r      = margin;
    const QPixmap* icon = pixmap(column);

    if (isSelected())
    {
        p->drawPixmap(0, 0, fv->itemBasePixmapSelected());
        p->setPen(cg.highlightedText());
    }
    else
    {
        p->drawPixmap(0, 0, fv->itemBasePixmapRegular());
        p->setPen(cg.text());
    }

    if (icon)
    {
        int xo = r;
        int yo = (height() - icon->height())/2;

        p->drawPixmap( xo, yo, *icon );

        r += icon->width() + fv->itemMargin();
    }

    if (m_special)
    {
        QFont f(p->font());
        f.setItalic(true);
        p->setFont(f);

        p->setPen(isSelected() ? cg.color(QColorGroup::LinkVisited) :
                  cg.color(QColorGroup::Link));        
    }

    QRect br;
    p->drawText(r, 0, width-margin-r, height(), Qt::AlignLeft|Qt::AlignVCenter,
                t, -1, &br);

    if (m_special)
    {
        p->drawLine(br.right() + 2, height()/2, fv->width(), height()/2);
    }
}

void FolderItem::setup()
{
    widthChanged();

    FolderView *fv = dynamic_cast<FolderView*>(listView());
    int h = fv->itemHeight();
    if (h % 2 > 0)
        h++;

    setHeight(h);
}
