/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-06
 * Description : Albums folder view.
 *
 * Copyright (C) 2005-2006 by Joern Ahrens <joern.ahrens@kdemail.net>
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ALBUMFOLDERVIEW_H
#define ALBUMFOLDERVIEW_H

// QT includes
#include <qtreeview.h>

// Local includes
#include "albummodel.h"
#include "albumtreeview.h"
#include "albummodificationhelper.h"

namespace Digikam {

class AlbumFolderViewNewPriv;

/**
 * Basic album view based on a QTreeWidget.
 *
 * @author jwienke
 */
class AlbumFolderViewNew: public AlbumTreeView
{
Q_OBJECT
public:
    AlbumFolderViewNew(QWidget *parent, AlbumModificationHelper *albumModificationHelper);
    ~AlbumFolderViewNew();

    /**
     * Returns the album on that the last context menu was triggered.
     *
     * @return album for which the last context menu was triggered or null if it
     *         wasn't triggered on a real album.
     */
    PAlbum *lastContextMenuAlbum() const;

Q_SIGNALS:

    /**
     * Emitted if a find duplicates search shall be invoked on the given album.
     *
     * @param album the album to find duplicates in
     */
    void signalFindDuplicatesInAlbum(Album *album);

public Q_SLOTS:

    /**
     * Selects the given album.
     *
     * @param album album to select
     */
    void slotSelectAlbum(Album *album);

private:

    /**
     * Creates the context menu.
     *
     * @param event event that requested the menu
     */
    void contextMenuEvent(QContextMenuEvent *event);

    /**
     * Re-implemented to handle custom tool tips.
     *
     * @param event event to process.
     */
    bool viewportEvent(QEvent *event);

private Q_SLOTS:
    void slotAlbumSelected(const QModelIndex &index);

private:
    AlbumFolderViewNewPriv *d;

};

}

// TODO jwienke: old code without model view

// KDE includes

#include <kio/job.h>

// Local includes

#include "searchtextbar.h"
#include "folderview.h"
#include "folderitem.h"

class QDropEvent;
class QPixmap;
class QDrag;

class KUrl;

namespace Digikam
{

class Album;
class PAlbum;
class AlbumFolderViewPriv;

/*class AlbumFolderViewItem : public FolderItem
{
public:

    AlbumFolderViewItem(Q3ListView *parent, PAlbum *album);
    AlbumFolderViewItem(Q3ListViewItem *parent, PAlbum *album);

    // special group item (collection/dates)
    AlbumFolderViewItem(Q3ListViewItem* parent, const QString& name,
                        int year, int month);

    PAlbum* album() const;
    int     id() const;
    bool    isGroupItem() const;
    int     compare(Q3ListViewItem *i, int col, bool ascending) const;
    void    refresh();
    void    setOpen(bool o);
    void    setCount(int count);
    int     count();
    int     countRecursive();

private:

    bool    m_groupItem;

    int     m_year;
    int     m_month;
    int     m_count;
    int     m_countRecursive;

    PAlbum *m_album;
};

class AlbumFolderView : public FolderView
{
    Q_OBJECT

public:

    AlbumFolderView(QWidget *parent);
    ~AlbumFolderView();

    void resort();
    void refresh();

    void albumNew();
    void albumDelete();
    void albumEdit();
    void albumRename();

    void setAlbumThumbnail(PAlbum *album);

    void setCurrentAlbum(Album *album);

Q_SIGNALS:

    void signalAlbumModified();
    void signalTextFolderFilterMatch(bool);
    void signalFindDuplicatesInAlbum(Album*);

public Q_SLOTS:

    void slotTextFolderFilterChanged(const SearchTextSettings&);

private Q_SLOTS:

    void slotGotThumbnailFromIcon(Album *album, const QPixmap& thumbnail);
    void slotThumbnailLost(Album *album);
    void slotReloadThumbnails();
    void slotSelectionChanged();

    void slotAlbumAdded(Album *);
    void slotAlbumDeleted(Album *album);
    void slotAlbumsCleared();
    void slotAlbumIconChanged(Album* album);
    void slotAlbumRenamed(Album *album);

    void slotContextMenu(Q3ListViewItem*, const QPoint&, int);

    void slotDIOResult(KJob* job);
    void slotRefresh(const QMap<int, int>&);

protected:

    void contentsDropEvent(QDropEvent *e);
    bool acceptDrop(const QDropEvent *e) const;

    void selectItem(int id);

private:

    void albumNew(AlbumFolderViewItem *item);
    void albumEdit(AlbumFolderViewItem *item);
    void albumRename(AlbumFolderViewItem *item);
    void albumDelete(AlbumFolderViewItem *item);

    void addAlbumChildrenToList(KUrl::List& list, Album *album);

    AlbumFolderViewItem* findParent(PAlbum* album, bool& failed);
    AlbumFolderViewItem* findParentByFolder(PAlbum* album, bool& failed);
    AlbumFolderViewItem* findParentByCategory(PAlbum* album, bool& failed);
    AlbumFolderViewItem* findParentByDate(PAlbum* album, bool& failed);

    void reparentItem(AlbumFolderViewItem* folderItem);
    void clearEmptyGroupItems();
    QDrag* makeDragObject();

private:

    AlbumFolderViewPriv* const d;
};*/

}  // namespace Digikam

#endif // ALBUMFOLDERVIEW_H
