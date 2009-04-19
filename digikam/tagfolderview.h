/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-22
 * Description : tags folder view.
 *
 * Copyright (C) 2005-2006 by Joern Ahrens <joern.ahrens@kdemail.net>
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

#ifndef TAGFOLDERVIEW_H
#define TAGFOLDERVIEW_H

// Qt includes

#include <QDropEvent>
#include <QPixmap>

// Local includes

#include "searchtextbar.h"
#include "folderview.h"

class QDropEvent;
class QDrag;

namespace Digikam
{

class Album;
class TAlbum;
class TagFolderViewItem;
class TagFolderViewPriv;

class TagFolderView : public FolderView
{
    Q_OBJECT

public:

    TagFolderView(QWidget *parent);
    ~TagFolderView();

    void tagNew();
    void tagEdit();
    void tagDelete();

    void selectItem(int id);

    void refresh();

Q_SIGNALS:

    void signalProgressBarMode(int, const QString&);
    void signalProgressValue(int);
    void signalTextTagFilterMatch(bool);

public Q_SLOTS:

    void slotTextTagFilterChanged(const SearchTextSettings&);

protected:

    void contentsDropEvent(QDropEvent *e);
    bool acceptDrop(const QDropEvent *e) const;

private Q_SLOTS:

    void slotAlbumAdded(Album*);
    void slotSelectionChanged();
    void slotAlbumDeleted(Album*);
    void slotAlbumRenamed(Album*);
    void slotAlbumsCleared();
    void slotAlbumIconChanged(Album* album);
    void slotAlbumMoved(TAlbum* tag, TAlbum* newParent);
    void slotContextMenu(Q3ListViewItem*, const QPoint&, int);
    void slotGotThumbnailFromIcon(Album *album, const QPixmap& thumbnail);
    void slotThumbnailLost(Album *album);
    void slotReloadThumbnails();
    void slotRefresh(const QMap<int, int>&);
    void slotAssignTags(int tagId, const QList<int> &imageIDs);
    void slotTagNewFromABCMenu(const QString&);

Q_SIGNALS: // private

    void assignTags(int tagId, const QList<int> &imageIDs);

private:

    void tagNew(TagFolderViewItem *item, const QString& _title=QString(),
                const QString& _icon=QString() );
    void tagEdit(TagFolderViewItem *item);
    void tagDelete(TagFolderViewItem *item);
    void setTagThumbnail(TAlbum *album);
    QDrag* makeDragObject();

private:

    TagFolderViewPriv* const d;
};

}  // namespace Digikam

#endif // TAGFOLDERVIEW_H
