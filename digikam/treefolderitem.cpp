/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-26-02
 * Description : Tree folder item.
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QTreeWidget>
#include <QFont>

// Local includes.

#include "album.h"
#include "treefolderview.h"
#include "treefolderitem.h"

namespace Digikam
{

TreeFolderItem::TreeFolderItem(QTreeWidget *parent, const QString& text, bool special)
              : QTreeWidgetItem(parent, QStringList() << text)
{
    setFocus(false);

    if (special)
    {
        QFont f = font(0);
        f.setItalic(true);
        setFont(0, f);
        setForeground(0, treeWidget()->palette().link());
    }
}

TreeFolderItem::TreeFolderItem(QTreeWidgetItem *parent, const QString& text, bool special)
              : QTreeWidgetItem(parent, QStringList() << text)
{
    setFocus(false);

    if (special)
    {
        QFont f = font(0);
        f.setItalic(true);
        setFont(0, f);
        setForeground(0, treeWidget()->palette().link());
    }
}

TreeFolderItem::~TreeFolderItem()
{
}

void TreeFolderItem::setFocus(bool b)
{
    m_focus = b;
    setForeground(0, m_focus ? treeWidget()->palette().link() 
                             : treeWidget()->palette().text());
    QColor hb = treeWidget()->palette().highlight().color();
    hb.setAlpha(127);
    setBackground(0, m_focus ? QBrush(hb)
                             : treeWidget()->palette().base());
}

bool TreeFolderItem::focus() const
{
    return m_focus;
}

int TreeFolderItem::id() const
{
    return 0;
}

/*
void TreeFolderItem::paintCell(QPainter* p, const QColorGroup & cg, int column,
                           int width, int align)
{
    TreeFolderView *fv = dynamic_cast<TreeFolderView*>(listView());
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

void TreeFolderItem::setup()
{
    widthChanged();

    TreeFolderView *fv = dynamic_cast<TreeFolderView*>(listView());
    int h = fv->itemHeight();
    if (h % 2 > 0)
        h++;

    setHeight(h);
}
*/

// ------------------------------------------------------------------------------------

TreeFolderCheckListItem::TreeFolderCheckListItem(QTreeWidget *parent, const QString& text)
                       : QTreeWidgetItem(parent, QStringList() << text)
{
    setCheckState(0, Qt::Unchecked);
}

TreeFolderCheckListItem::TreeFolderCheckListItem(QTreeWidgetItem *parent, const QString& text)
                       : QTreeWidgetItem(parent, QStringList() << text)
{
    setCheckState(0, Qt::Unchecked);
}

TreeFolderCheckListItem::~TreeFolderCheckListItem()
{
}

/*
void TreeFolderCheckListItem::paintCell(QPainter* p, const QColorGroup & cg,
                                        int column, int width, int)
{
    TreeFolderView *fv = dynamic_cast<TreeFolderView*>(listView());
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

        p->fillRect(0, 0, r, height(), cg.base());

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

void TreeFolderCheckListItem::setup()
{
    widthChanged();

    TreeFolderView *fv = dynamic_cast<TreeFolderView*>(listView());
    int h = fv->itemHeight();
    if (h % 2 > 0)
        h++;

    setHeight(h);
}

// NOTE: Inspired from Qt4::Q3listview::getStyleOption()
QStyleOptionQ3ListView TreeFolderCheckListItem::getStyleOption(const TreeFolderView *fv)
{
    Q3ListViewItem *item = dynamic_cast<Q3ListViewItem*>(this);
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
*/

// ------------------------------------------------------------------------------------

TreeAlbumItem::TreeAlbumItem(QTreeWidget* parent, Album* album)
             : TreeFolderItem(parent, album ? album->title() : QString())
{
    m_album = album;
    m_album->setExtraData(treeWidget(), this);
}

TreeAlbumItem::TreeAlbumItem(QTreeWidgetItem* parent, Album* album)
             : TreeFolderItem(parent, album ? album->title() : QString())
{
    m_album = album;
    m_album->setExtraData(treeWidget(), this);
}

TreeAlbumItem::~TreeAlbumItem()
{
    m_album->removeExtraData(treeWidget());
}

Album* TreeAlbumItem::album() const
{
    return m_album;
}

int TreeAlbumItem::id() const
{
    return album() ? album()->id() : 0;
}

// ------------------------------------------------------------------------------------

TreeAlbumCheckListItem::TreeAlbumCheckListItem(QTreeWidget* parent, Album* album)
                      : TreeAlbumItem(parent, album)
{
    setCheckState(0, Qt::Unchecked);
}

TreeAlbumCheckListItem::TreeAlbumCheckListItem(QTreeWidgetItem* parent, Album* album)
                      : TreeAlbumItem(parent, album)
{
    setCheckState(0, Qt::Unchecked);
}

TreeAlbumCheckListItem::~TreeAlbumCheckListItem()
{
}

}  // namespace Digikam
