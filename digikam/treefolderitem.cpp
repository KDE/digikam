/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-26-02
 * Description : Album folder item to use with QTreeWidget.
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

#include "treefolderitem.h"

// Qt includes

#include <QTreeWidget>
#include <QFont>

// Local includes

#include "album.h"

namespace Digikam
{

TreeFolderItem::TreeFolderItem(QTreeWidget *parent, const QString& text, bool special)
              : QTreeWidgetItem(parent, QStringList() << text)
{
    setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled);
    setForegroundBrush(treeWidget()->palette().text());

    if (special)
    {
        for (int i=0 ; i < columnCount() ; ++i)
        {
            QFont f = font(i);
            f.setItalic(true);
            setFont(i, f);
            setForeground(i, treeWidget()->palette().link());
        }
        setForegroundBrush(treeWidget()->palette().link());
    }

    setFocus(false);
}

TreeFolderItem::TreeFolderItem(QTreeWidgetItem *parent, const QString& text, bool special)
              : QTreeWidgetItem(parent, QStringList() << text)
{
    setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled);
    setForegroundBrush(treeWidget()->palette().text());

    if (special)
    {
        for (int i=0 ; i < columnCount() ; ++i)
        {
            QFont f = font(i);
            f.setItalic(true);
            setFont(i, f);
            setForeground(i, treeWidget()->palette().link());
        }
        setForegroundBrush(treeWidget()->palette().link());
    }

    setFocus(false);
}

TreeFolderItem::~TreeFolderItem()
{
}

void TreeFolderItem::setFocus(bool b)
{
    m_focus = b;
    for (int i=0 ; i < columnCount() ; ++i)
    {
        setForeground(i, m_focus ? treeWidget()->palette().link()
                                 : foregroundBrush());
        QColor hb = treeWidget()->palette().highlight().color();
        hb.setAlpha(127);
        setBackground(i, m_focus ? QBrush(hb)
                                 : treeWidget()->palette().base());
    }
}

bool TreeFolderItem::focus() const
{
    return m_focus;
}

int TreeFolderItem::id() const
{
    return 0;
}

QBrush TreeFolderItem::foregroundBrush() const
{
    return m_foregroundBrush;
}

void TreeFolderItem::setForegroundBrush(const QBrush& brush)
{
    m_foregroundBrush = brush;
}

// ------------------------------------------------------------------------------------

TreeFolderCheckListItem::TreeFolderCheckListItem(QTreeWidget *parent, const QString& text)
                       : QTreeWidgetItem(parent, QStringList() << text)
{
    setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    setCheckState(0, Qt::Unchecked);
}

TreeFolderCheckListItem::TreeFolderCheckListItem(QTreeWidgetItem *parent, const QString& text)
                       : QTreeWidgetItem(parent, QStringList() << text)
{
    setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    setFlags(flags() | Qt::ItemIsUserCheckable);
    setCheckState(0, Qt::Unchecked);
}

TreeFolderCheckListItem::~TreeFolderCheckListItem()
{
}

// ------------------------------------------------------------------------------------

TreeAlbumItem::TreeAlbumItem(QTreeWidget* parent, Album* album)
             : TreeFolderItem(parent, album ? album->title() : QString())
{
    m_album = album;
    if (m_album)
        m_album->setExtraData(treeWidget(), this);
}

TreeAlbumItem::TreeAlbumItem(QTreeWidgetItem* parent, Album* album)
             : TreeFolderItem(parent, album ? album->title() : QString())
{
    m_album = album;
    if (m_album)
        m_album->setExtraData(treeWidget(), this);
}

TreeAlbumItem::~TreeAlbumItem()
{
    if (m_album)
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
    setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    setCheckState(0, Qt::Unchecked);
}

TreeAlbumCheckListItem::TreeAlbumCheckListItem(QTreeWidgetItem* parent, Album* album)
                      : TreeAlbumItem(parent, album)
{
    setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    setCheckState(0, Qt::Unchecked);
}

TreeAlbumCheckListItem::~TreeAlbumCheckListItem()
{
}

bool TreeAlbumCheckListItem::isOn() const
{
    return (checkState(0) == Qt::Checked ? true : false);
}

void TreeAlbumCheckListItem::setOn(bool b)
{
    setCheckState(0, b ? Qt::Checked : Qt::Unchecked);
}

}  // namespace Digikam
