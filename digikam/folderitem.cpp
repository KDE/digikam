/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-31
 * Description : implementation of item folder 
 * 
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>

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

#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPixmap>
#include <QStyle>

// Local includes.

#include "thumbnailsize.h"
#include "albumsettings.h"
#include "folderview.h"
#include "folderitem.h"

namespace Digikam
{

FolderItem::FolderItem(Q3ListView* parent, const QString& text, bool special)
          : Q3ListViewItem(parent, text)
{
    m_special = special;
    m_focus = false;
}

FolderItem::FolderItem(Q3ListViewItem* parent, const QString& text, bool special)
          : Q3ListViewItem(parent, text)
{
    m_special = special;
    m_focus = false;    
}

FolderItem::~FolderItem()
{
}

void FolderItem::setFocus(bool b)
{
    m_focus = b;
}

bool FolderItem::focus() const
{
    return m_focus;
}

void FolderItem::paintCell(QPainter* p, const QColorGroup & cg, int column,
                           int width, int align)
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

        r += icon->width() + 5 + fv->itemMargin();
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
    
    if (m_focus)
    {
        p->setPen(cg.link());
        QRect r = fv->itemRect(this);
        p->drawRect(0, 0, r.width(), r.height());
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

int FolderItem::id() const
{
    return 0;
}

// ------------------------------------------------------------------------------------

FolderCheckListItem::FolderCheckListItem(Q3ListView* parent, const QString& text,
                                         Q3CheckListItem::Type tt)
                   : Q3CheckListItem(parent, text, tt)
{
}

FolderCheckListItem::FolderCheckListItem(Q3ListViewItem* parent, const QString& text,
                                         Q3CheckListItem::Type tt)
                   : Q3CheckListItem(parent, text, tt)
{
}

FolderCheckListItem::~FolderCheckListItem()
{
}

void FolderCheckListItem::paintCell(QPainter* p, const QColorGroup & cg,
                                    int column, int width, int)
{
    FolderView *fv = dynamic_cast<FolderView*>(listView());
    if (!fv)
        return;
    
    QFontMetrics fm(p->fontMetrics());

    QString t = text(column);

    int margin = fv->itemMargin();
    int r      = margin;
    const QPixmap* icon = pixmap(column);

    int styleflags = QStyle::State_None;
    switch (state())
    {
        case(Q3CheckListItem::Off):
            styleflags |= QStyle::State_Off;
            break;
        case(Q3CheckListItem::NoChange):
            styleflags |= QStyle::State_NoChange;
            break;
        case(Q3CheckListItem::On):
            styleflags |= QStyle::State_On;
            break;
    }

    if (isSelected())
        styleflags |= QStyle::State_Selected;

    if (isEnabled() && fv->isEnabled())
        styleflags |= QStyle::State_Enabled;

    if ((type() == Q3CheckListItem::CheckBox) ||
        (type() == Q3CheckListItem::CheckBoxController))
    {
        int boxsize = fv->style().pixelMetric(QStyle::PM_CheckListButtonSize, fv); 
        int x = 3;
        int y = (height() - boxsize)/2 + margin;
        r += boxsize + 4;

        p->fillRect(0, 0, r, height(), cg.base());
        
        fv->style().drawPrimitive(QStyle::PE_CheckListIndicator, p,
                                  QRect(x, y, boxsize, height()),
                                  cg, styleflags, QStyleOption(this));
    }
    
    if (isSelected())
    {
        p->drawPixmap(r, 0, fv->itemBasePixmapSelected());
        p->setPen(cg.highlightedText());
    }
    else
    {
        p->drawPixmap(r, 0, fv->itemBasePixmapRegular());
        p->setPen(cg.text());
    }
    
    if (icon)
    {
        int xo = r;
        int yo = (height() - icon->height())/2;

        p->drawPixmap( xo, yo, *icon );

        r += icon->width() + fv->itemMargin();
    }

    p->drawText(r, 0, width-margin-r, height(), Qt::AlignLeft|Qt::AlignVCenter, t);
}

void FolderCheckListItem::setup()
{
    widthChanged();

    FolderView *fv = dynamic_cast<FolderView*>(listView());
    int h = fv->itemHeight();
    if (h % 2 > 0)
        h++;

    setHeight(h);
}

}  // namespace Digikam
