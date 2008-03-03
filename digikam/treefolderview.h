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

#include <QTreeWidget>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QDragEnterEvent>

// Local includes

#include "digikam_export.h"
#include "albummanager.h"

namespace Digikam
{
class TreeFolderItem;
class TreeFolderViewPriv;

class DIGIKAM_EXPORT TreeFolderView : public QTreeWidget
{
    Q_OBJECT

public:

    TreeFolderView(QWidget *parent, const char *name="TreeFolderView");
    virtual ~TreeFolderView();

    void setActive(bool val);
    bool active() const;

protected:

    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void dragEnterEvent(QDragEnterEvent*);
    void dragMoveEvent(QDragMoveEvent*);
    void dragLeaveEvent(QDragLeaveEvent*);
    void dropEvent(QDropEvent*);
    QStringList mimeTypes() const;

    virtual bool acceptDrop(const QDropEvent *e) const;

    TreeFolderItem* dragItem() const;
    virtual void makeDragObject(){};

    virtual void selectItem(int){};

    /** load the last state from the view from disk */
    virtual void loadViewState();

    /** writes the views state to disk */
    virtual void saveViewState();

protected slots:

    virtual void slotSelectionChanged(){};
    virtual void slotAllAlbumsLoaded();

private slots:

    void slotThemeChanged();
    void slotIconSizeChanged();

private:

    TreeFolderViewPriv *d;
};

}  // namespace Digikam

#endif // TREEFOLDERVIEW_H
