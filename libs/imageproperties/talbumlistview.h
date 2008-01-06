/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-18-12
 * Description : A list view to display digiKam Tags.
 *
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
 
#ifndef TALBUMLISTVIEW_H
#define TALBUMLISTVIEW_H

// Qt includes.

#include <Q3ListView>

// Local includes.

#include "digikam_export.h"
#include "metadatahub.h"

class Q3ListViewItem;
class QDropEvent;
class QMouseEvent;
class Q3DragObject;

namespace Digikam
{
class TAlbum;
class TAlbumListViewPriv;

class DIGIKAM_EXPORT TAlbumCheckListItem : public Q3CheckListItem
{
public:

    TAlbumCheckListItem(Q3ListView* parent, TAlbum* album);

    TAlbumCheckListItem(Q3CheckListItem* parent, TAlbum* album);

    void    setStatus(MetadataHub::TagStatus status);
    void    refresh();
    void    setOpen(bool o);
    TAlbum* album() const;
    int     id() const;
    void    setCount(int count);
    int     count();

private:
    
    void stateChange(bool val);

private:

    int     m_count;
    
    TAlbum *m_album;
};

// ------------------------------------------------------------------------

class DIGIKAM_EXPORT TAlbumListView : public Q3ListView
{
    Q_OBJECT

public:

    TAlbumListView(QWidget* parent);
    ~TAlbumListView();

    void emitSignalItemStateChanged(TAlbumCheckListItem *item);
    void refresh();

signals:

    void signalProgressBarMode(int, const QString&);
    void signalProgressValue(int);
    void signalItemStateChanged(TAlbumCheckListItem *item);

protected:

    void contentsMousePressEvent(QMouseEvent *e);
    void contentsMouseReleaseEvent(QMouseEvent *e);
    void contentsMouseMoveEvent(QMouseEvent *e);

    bool acceptDrop(const QDropEvent *e) const;
    void contentsDropEvent(QDropEvent *e);

    Q3DragObject* dragObject();
    void startDrag();
    TAlbumCheckListItem* dragItem() const;

private slots:

    void slotRefresh(const QMap<int, int>&);

private:

    bool mouseInItemRect(Q3ListViewItem* item, int x) const;

private:

    TAlbumListViewPriv *d;
};

}  // NameSpace Digikam
 
#endif  // TALBUMLISTVIEW_H
