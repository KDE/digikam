/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2005-05-05
 * Description : tags filter view
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

#ifndef TAGFILTERVIEW_H
#define TAGFILTERVIEW_H

// Qt includes.

#include <QPixmap>
#include <QDropEvent>
#include <QMouseEvent>

// Local includes.

#include "treefolderview.h"

class QTreeWidgetItem;

namespace Digikam
{

class Album;
class TagFilterViewItem;
class TagFilterViewPrivate;

class TagFilterView : public TreeFolderView
{
    Q_OBJECT

public:

    enum ToggleAutoTags
    {
        NoToggleAuto = 0,
        Children,
        Parents,
        ChildrenAndParents
    };

public:

    TagFilterView(QWidget* parent);
    ~TagFilterView();

    void refresh();

signals:

    void signalProgressBarMode(int, const QString&);
    void signalProgressValue(int);
    void signalTextTagFilterMatch(bool);

public slots:

    void slotTextTagFilterChanged(const QString&);

    /** Reset all active tag filters */
    void slotResetTagFilters();

protected:

    void contextMenuEvent(QContextMenuEvent*);
    bool acceptDrop(const QDropEvent *e) const;
    void dropEvent(QDropEvent *e);

private slots:

    void slotAlbumAdded(Album *album);
    void slotAlbumMoved(TAlbum *tag, TAlbum *newParent);
    void slotAlbumRenamed(Album *album);
    void slotAlbumDeleted(Album *album);
    void slotAlbumsCleared();
    void slotAlbumIconChanged(Album *album);
    void slotTimeOut();
    void slotABCContextMenu();
    void slotGotThumbnailFromIcon(Album *album, const QPixmap& thumbnail);
    void slotThumbnailLost(Album *album);
    void slotReloadThumbnails();
    void slotRefresh(const QMap<int, int>&);
    void slotItemExpanded(QTreeWidgetItem*);
    void slotItemChanged(QTreeWidgetItem*);

private:

    void triggerChange();
    void tagNew(TagFilterViewItem* item, const QString& _title=QString(),
                const QString& _icon=QString());
    void tagEdit(TagFilterViewItem* item);
    void tagDelete(TagFilterViewItem* item);
    void setTagThumbnail(TAlbum *album);
    void toggleChildTags(TagFilterViewItem* tItem, bool b);
    void toggleParentTags(TagFilterViewItem* tItem, bool b);
    void makeDragObject();

private:

    TagFilterViewPrivate *d;
};

}  // namespace Digikam

#endif /* TAGFILTERVIEW_H */
