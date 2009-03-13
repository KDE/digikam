/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-18-12
 * Description : A list view to display digiKam Tags.
 *
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

// Local includes.

#include "digikam_export.h"
#include "metadatahub.h"
#include "folderitem.h"
#include "folderview.h"

class QDropEvent;
class QMouseEvent;
class QDrag;

namespace Digikam
{
class TAlbum;

class TAlbumCheckListItem : public FolderCheckListItem
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
    int     compare(Q3ListViewItem *i, int col, bool ascending) const;

private :

    void    stateChange(bool val);

private:

    int     m_count;

    TAlbum *m_album;
};

// ------------------------------------------------------------------------

class TAlbumListView : public FolderView
{
    Q_OBJECT

public:

    TAlbumListView(QWidget* parent);
    ~TAlbumListView();

    void stateChanged(TAlbumCheckListItem *item);
    void refresh();
    void loadViewState();

signals:

    void signalProgressBarMode(int, const QString&);
    void signalProgressValue(int);
    void signalItemStateChanged(TAlbumCheckListItem *item);

protected:

    bool acceptDrop(const QDropEvent *e) const;
    void contentsDropEvent(QDropEvent *e);

    QDrag* makeDragObject();

private slots:

    void slotRefresh(const QMap<int, int>&);
    void slotAssignTags(int tagId, const QList<int> &imageIDs);

signals: // private

    void assignTags(int tagId, const QList<int> &imageIDs);

private:

    void saveViewState();
};

}  // namespace Digikam

#endif  // TALBUMLISTVIEW_H
