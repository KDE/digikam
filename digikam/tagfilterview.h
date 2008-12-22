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

#include "searchtextbar.h"
#include "folderview.h"

class QDrag;

namespace Digikam
{

class Album;
class TAlbum;
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

    void slotTextTagFilterChanged(const SearchTextSettings&);

    /** Reset all active tag filters */
    void slotResetTagFilters();

protected:


    bool acceptDrop(const QDropEvent *e) const;
    void contentsDropEvent(QDropEvent *e);

private slots:

    void slotTagAdded(Album* album);
    void slotTagMoved(TAlbum* tag, TAlbum* newParent);
    void slotTagRenamed(Album* album);
    void slotTagDeleted(Album* album);
    void slotClear();
    void slotAlbumIconChanged(Album* album);
    void slotTimeOut();
    void slotContextMenu(Q3ListViewItem*, const QPoint&, int);
    void slotABCContextMenu();
    void slotGotThumbnailFromIcon(Album *album, const QPixmap& thumbnail);
    void slotThumbnailLost(Album *album);
    void slotReloadThumbnails();
    void slotRefresh(const QMap<int, int>&);
    void slotAssignTags(int tagId, const QList<int> &imageIDs);

signals: // private

    void assignTags(int tagId, const QList<int> &imageIDs);

private:

    void triggerChange();
    void tagNew(TagFilterViewItem* item, const QString& _title=QString(),
                const QString& _icon=QString());
    void tagEdit(TagFilterViewItem* item);
    void tagDelete(TagFilterViewItem* item);
    void setTagThumbnail(TAlbum *album);
    void toggleChildTags(TagFilterViewItem* tItem, bool b);
    void toggleParentTags(TagFilterViewItem* tItem, bool b);
    QDrag* makeDragObject();

private:

    TagFilterViewPrivate* const d;
};

}  // namespace Digikam

#endif /* TAGFILTERVIEW_H */
