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

// Local includes.

#include "folderview.h"

namespace Digikam
{

class Album;
class TagFilterViewItem;
class TagFilterViewPrivate;

class TagFilterView : public FolderView
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

    void stateChanged(TagFilterViewItem*);
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

    bool acceptDrop(const QDropEvent *e) const;
    void contentsDropEvent(QDropEvent *e);
    QDragObject* dragObject();

private slots:

    void slotTagAdded(Album* album);
    void slotTagMoved(TAlbum* tag, TAlbum* newParent);
    void slotTagRenamed(Album* album);
    void slotTagDeleted(Album* album);
    void slotClear();
    void slotAlbumIconChanged(Album* album);
    void slotTimeOut();
    void slotContextMenu(QListViewItem*, const QPoint&, int);
    void slotABCContextMenu();
    void slotGotThumbnailFromIcon(Album *album, const QPixmap& thumbnail);
    void slotThumbnailLost(Album *album);
    void slotReloadThumbnails();
    void slotRefresh(const QMap<int, int>&);

private:

    void triggerChange();
    void tagNew(TagFilterViewItem* item, const QString& _title=QString(),
                const QString& _icon=QString());
    void tagEdit(TagFilterViewItem* item);
    void tagDelete(TagFilterViewItem* item);
    void setTagThumbnail(TAlbum *album);
    void toggleChildTags(TagFilterViewItem* tItem, bool b);
    void toggleParentTags(TagFilterViewItem* tItem, bool b);

private:

    TagFilterViewPrivate *d;
};

}  // namespace Digikam

#endif /* TAGFILTERVIEW_H */
