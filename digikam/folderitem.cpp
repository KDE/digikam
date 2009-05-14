/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-31
 * Description : implementation of item folder
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

#include "folderitem.h"

// Qt includes

#include <QFont>
#include <QFontMetrics>
#include <QFrame>
#include <QPainter>
#include <QPixmap>
#include <QStyle>
#include <QStyleOption>

// Local includes

#include "thumbnailsize.h"
#include "albumsettings.h"
#include "folderview.h"

namespace Digikam
{

FolderItem::FolderItem(Q3ListView* parent, const QString& text, bool special)
          : Q3ListViewItem(parent, text)
{
    m_special     = special;
    m_highlighted = false;
}

FolderItem::FolderItem(Q3ListViewItem* parent, const QString& text, bool special)
          : Q3ListViewItem(parent, text)
{
    m_special     = special;
    m_highlighted = false;
}

FolderItem::~FolderItem()
{
}

void FolderItem::setHighlighted(bool b)
{
    m_highlighted = b;
    repaint();
}

bool FolderItem::isHighlighted() const
{
    return m_highlighted;
}

void FolderItem::paintCell(QPainter* p, const QColorGroup& cg, int column, int width, int)
{
    FolderView *fv = dynamic_cast<FolderView*>(listView());
    if (!fv)
        return;

    QFontMetrics fm(p->fontMetrics());

    QString t           = text(column);
    int margin          = fv->itemMargin();
    int r               = margin;
    const QPixmap* icon = pixmap(column);

    if (isSelected())
    {
        p->drawPixmap(0, 0, fv->itemBasePixmapSelected());
        p->setPen(cg.color(QColorGroup::HighlightedText));
    }
    else
    {
        p->drawPixmap(0, 0, fv->itemBasePixmapRegular());
        p->setPen(cg.color(QColorGroup::Text));
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
    p->drawText(QRect(r, 0, width-margin-r, height()), Qt::AlignLeft|Qt::AlignVCenter, t.left(-1), &br);

    if (m_special)
    {
        p->drawLine(br.right() + 2, height()/2, fv->width(), height()/2);
    }

    if (m_highlighted)
    {
        p->setPen(cg.color(QColorGroup::Highlight));
        QRect r = fv->itemRect(this);
        p->drawRect(0, 0, r.width()-1, r.height()-1);
    }
}

void FolderItem::setup()
{
    widthChanged();

    FolderView *fv = dynamic_cast<FolderView*>(listView());
    int h = fv->itemHeight();
    if (h % 2 > 0)
        ++h;

    setHeight(h);
}

int FolderItem::id() const
{
    return 0;
}

void FolderItem::takeItem(Q3ListViewItem *item)
{
    FolderView *fv = dynamic_cast<FolderView*>(listView());
    if (fv)
        fv->notifyTakeItem(item);

    Q3ListViewItem::takeItem(item);
}

// ------------------------------------------------------------------------------------

FolderCheckListItem::FolderCheckListItem(Q3ListView* parent, const QString& text,
                                         Q3CheckListItem::Type tt)
                   : Q3CheckListItem(parent, text, tt)
{
    m_highlighted = false;
}

FolderCheckListItem::FolderCheckListItem(Q3ListViewItem* parent, const QString& text,
                                         Q3CheckListItem::Type tt)
                   : Q3CheckListItem(parent, text, tt)
{
    m_highlighted = false;
}

FolderCheckListItem::~FolderCheckListItem()
{
}

void FolderCheckListItem::setHighlighted(bool b)
{
    m_highlighted = b;
    repaint();
}

bool FolderCheckListItem::isHighlighted() const
{
    return m_highlighted;
}

void FolderCheckListItem::takeItem(Q3ListViewItem *item)
{
    FolderView *fv = dynamic_cast<FolderView*>(listView());
    if (fv)
        fv->notifyTakeItem(item);

    Q3CheckListItem::takeItem(item);
}

void FolderCheckListItem::paintCell(QPainter* p, const QColorGroup& cg, int column, int width, int)
{
    FolderView *fv = dynamic_cast<FolderView*>(listView());
    if (!fv)
        return;

    QFontMetrics fm(p->fontMetrics());

    QString t           = text(column);
    int margin          = fv->itemMargin();
    int r               = margin;
    const QPixmap* icon = pixmap(column);

    QStyle::State styleflags = QStyle::State_None;
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
        int boxsize = fv->style()->pixelMetric(QStyle::PM_CheckListButtonSize, 0, fv);
        int x       = 3;
        int y       = (height() - boxsize)/2 + margin;
        r += boxsize + 4;

        p->fillRect(0, 0, r, height(), cg.color(QColorGroup::Base));

        // NOTE: Inspired form Qt4::Q3listview::paintCell()
        QStyleOptionQ3ListView opt = getStyleOption(fv);
        opt.rect.setRect(x, y, boxsize, height());
        opt.palette = cg;
        opt.state   = styleflags;
        fv->style()->drawPrimitive(QStyle::PE_Q3CheckListIndicator, &opt, p, fv);
    }

    if (isSelected())
    {
        p->drawPixmap(r, 0, fv->itemBasePixmapSelected());
        p->setPen(cg.color(QColorGroup::HighlightedText));
    }
    else
    {
        p->drawPixmap(r, 0, fv->itemBasePixmapRegular());
        p->setPen(cg.color(QColorGroup::Text));
    }

    if (icon)
    {
        int xo = r;
        int yo = (height() - icon->height())/2;

        p->drawPixmap( xo, yo, *icon );

        r += icon->width() + fv->itemMargin();
    }

    p->drawText(r, 0, width-margin-r, height(), Qt::AlignLeft|Qt::AlignVCenter, t);

    p->setBrush(QBrush());
    if (m_highlighted)
    {
        p->setPen(cg.color(QColorGroup::Link));
        QRect r = fv->itemRect(this);
        p->drawRect(0, 0, r.width()-1, r.height()-1);
    }
}

void FolderCheckListItem::setup()
{
    widthChanged();

    FolderView *fv = dynamic_cast<FolderView*>(listView());
    int h = fv->itemHeight();
    if (h % 2 > 0)
        ++h;

    setHeight(h);
}

// NOTE: Inspired from Qt4::Q3listview::getStyleOption()
QStyleOptionQ3ListView FolderCheckListItem::getStyleOption(const FolderView *fv)
{
    Q3ListViewItem *item  = dynamic_cast<Q3ListViewItem*>(this);
    QStyleOptionQ3ListView opt;
    opt.init(fv);
    opt.subControls       = QStyle::SC_None;
    opt.activeSubControls = QStyle::SC_None;
    QWidget *vp           = fv->viewport();
    opt.viewportPalette   = vp->palette();
    opt.viewportBGRole    = vp->backgroundRole();
    opt.itemMargin        = fv->itemMargin();
    opt.sortColumn        = 0;
    opt.treeStepSize      = fv->treeStepSize();
    opt.rootIsDecorated   = fv->rootIsDecorated();
    bool firstItem        = true;
    while (item)
    {
        QStyleOptionQ3ListViewItem lvi;
        lvi.height      = item->height();
        lvi.totalHeight = item->totalHeight();
        lvi.itemY       = item->itemPos();
        lvi.childCount  = item->childCount();
        lvi.features    = QStyleOptionQ3ListViewItem::None;
        lvi.state       = QStyle::State_None;
        if (item->isEnabled())
            lvi.state |= QStyle::State_Enabled;
        if (item->isOpen())
            lvi.state |= QStyle::State_Open;
        if (item->isExpandable())
            lvi.features |= QStyleOptionQ3ListViewItem::Expandable;
        if (item->multiLinesEnabled())
            lvi.features |= QStyleOptionQ3ListViewItem::MultiLine;
        if (item->isVisible())
            lvi.features |= QStyleOptionQ3ListViewItem::Visible;
        if (item->parent() && item->parent()->rtti() == 1
            && static_cast<Q3CheckListItem *>(item->parent())->type() == Q3CheckListItem::Controller)
            lvi.features |= QStyleOptionQ3ListViewItem::ParentControl;
        opt.items.append(lvi);
        if (!firstItem)
        {
            item = item->nextSibling();
        }
        else
        {
            firstItem = false;
            item = item->firstChild();
        }
    }
    return opt;
}

}  // namespace Digikam
