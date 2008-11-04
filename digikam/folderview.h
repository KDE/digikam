/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-28
 * Description : implementation of folder view.
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

/** @file foldeview.h */

#ifndef _FOLDERVIEW_H_
#define _FOLDERVIEW_H_

// Qt includes.

#include <qlistview.h>

// Local includes.

#include "digikam_export.h"
#include "album.h"

namespace Digikam
{

class FolderViewPriv;
class FolderItem;

/**
 * \class FolderView
 * \brief Base class for a tree view
 */


class DIGIKAM_EXPORT FolderView : public QListView
{
    Q_OBJECT

public:

    enum CollapseMode
    {
        /*
         * Collapse the folder view and re-open the current viewed album (default mode)
         * In this mode, all root items are collapsed, then the one containing
         * the currently selected album is expand again.
         * This mode will make sure that the selected album is visible in the folder tree.
         */
        RestoreCurrentAlbum,
        /*
         * Collapse the folder view but omit the root item.
         * In this mode all items in the folder view are collapsed,
         * and the first root item is expanded again (My Tags / My Albums etc)
         */
        OmitRoot
    };

    FolderView(QWidget *parent, const char *name = "FolderView");
    virtual ~FolderView();

    void setActive(bool val);
    bool active() const;

    int      itemHeight() const;
    QRect    itemRect(QListViewItem *item) const;
    QPixmap  itemBasePixmapRegular() const;
    QPixmap  itemBasePixmapSelected() const;

    virtual void collapseView(CollapseMode mode = RestoreCurrentAlbum);

protected:

    void contentsMousePressEvent(QMouseEvent *e);
    void contentsMouseReleaseEvent(QMouseEvent *e);
    void contentsMouseMoveEvent(QMouseEvent *e);
    void contentsDragEnterEvent(QDragEnterEvent *e);
    void contentsDragMoveEvent(QDragMoveEvent *e);
    void contentsDragLeaveEvent(QDragLeaveEvent * e);
    void contentsDropEvent(QDropEvent *e);

    virtual bool acceptDrop(const QDropEvent *e) const;

    void startDrag();
    QListViewItem* dragItem() const;

    void resizeEvent(QResizeEvent* e);
    void fontChange(const QFont& oldFont);

    virtual void selectItem(int id);

    /**
     * load the last state from the view from disk
     */
    virtual void loadViewState();

    /**
     * writes the views state to disk
     */
    virtual void saveViewState();

protected slots:

    virtual void slotSelectionChanged();
    virtual void slotAllAlbumsLoaded();

private slots:

    void slotThemeChanged();
    void slotIconSizeChanged();

private:

    bool mouseInItemRect(QListViewItem* item, int x) const;

    FolderViewPriv *d;
};

}  // namespace Digikam

#endif // _FOLDERVIEW_H
