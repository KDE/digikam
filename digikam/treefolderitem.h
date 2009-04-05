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

#ifndef TREEFOLDERITEM_H
#define TREEFOLDERITEM_H

// Qt includes

#include <QTreeWidgetItem>

// Local includes

#include "digikam_export.h"

class QTreeWidget;

namespace Digikam
{

class Album;
class TreeFolderView;

class TreeFolderItem : public QTreeWidgetItem
{

public:

    TreeFolderItem(QTreeWidget* parent, const QString& text, bool special=false);
    TreeFolderItem(QTreeWidgetItem* parent, const QString& text, bool special=false);
    virtual ~TreeFolderItem();

    virtual int id() const;

    void setFocus(bool b);
    bool focus() const;

protected:

    QBrush foregroundBrush() const;
    void   setForegroundBrush(const QBrush& brush);

private:

    bool   m_focus;
    bool   m_special;

    QBrush m_foregroundBrush;
};

// ------------------------------------------------------------------------------------

class TreeFolderCheckListItem : public QTreeWidgetItem
{

public:

    TreeFolderCheckListItem(QTreeWidget* parent, const QString& text);
    TreeFolderCheckListItem(QTreeWidgetItem* parent, const QString& text);
    virtual ~TreeFolderCheckListItem();
};

// ------------------------------------------------------------------------------------

class TreeAlbumItem : public TreeFolderItem
{

public:

    TreeAlbumItem(QTreeWidget* parent, Album* album);
    TreeAlbumItem(QTreeWidgetItem* parent, Album* album);
    virtual ~TreeAlbumItem();

    Album* album() const;
    int id() const;

private:

    Album *m_album;
};

// ------------------------------------------------------------------------------------

class TreeAlbumCheckListItem : public TreeAlbumItem
{

public:

    TreeAlbumCheckListItem(QTreeWidget* parent, Album* album);
    TreeAlbumCheckListItem(QTreeWidgetItem* parent, Album* album);
    virtual ~TreeAlbumCheckListItem();

    bool isOn() const;
    void setOn(bool b);
};

}  // namespace Digikam

#endif /* TREEFOLDERITEM_H */
