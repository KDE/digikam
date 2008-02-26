/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-26-02
 * Description : Tree folder view.
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

#ifndef TREEFOLDERVIEW_H
#define TREEFOLDERVIEW_H

// Qt includes.

#include <Q3ListView>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QPixmap>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class TreeFolderViewPriv;
class TreeFolderItem;

class DIGIKAM_EXPORT TreeFolderView : public Q3ListView
{
    Q_OBJECT

public:

    TreeFolderView(QWidget *parent, const char *name = "TreeFolderView");
    virtual ~TreeFolderView();

    void setActive(bool val);
    bool active() const;

    int      itemHeight() const;
    QRect    itemRect(Q3ListViewItem *item) const;
    QPixmap  itemBasePixmapRegular() const;
    QPixmap  itemBasePixmapSelected() const;

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
    TreeFolderItem* dragItem() const;

    void resizeEvent(QResizeEvent* e);
    void fontChange(const QFont& oldFont);

    virtual void selectItem(int id);

    /** load the last state from the view from disk */
    virtual void loadViewState();

    /** writes the views state to disk */
    virtual void saveViewState();

    bool mouseInItemRect(Q3ListViewItem* item, int x) const;

protected slots:

    virtual void slotSelectionChanged();
    virtual void slotAllAlbumsLoaded();

private slots:

    void slotThemeChanged();
    void slotIconSizeChanged();

private:

    TreeFolderViewPriv *d;
};

}  // namespace Digikam

#endif // TREEFOLDERVIEW_H
