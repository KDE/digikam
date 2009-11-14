/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-25
 * Description : Tree View for album models
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef ALBUMTREEVIEW_H
#define ALBUMTREEVIEW_H

// Qt includes

#include <QTreeView>

// Local includes

#include "albummodel.h"
#include "albumfiltermodel.h"

namespace Digikam
{

class AbstractAlbumTreeView : public QTreeView
{
    Q_OBJECT

public:

    explicit AbstractAlbumTreeView(AbstractSpecificAlbumModel *model, QWidget *parent = 0);

    AbstractSpecificAlbumModel *albumModel() const;
    AlbumFilterModel *albumFilterModel() const;

    /// Enable expanding of tree items on single click on the item (default: on)
    void setSelectOnSingleClick(bool doThat);

    /** This is a combination of indexAt() checked with visualRect().
     *  p must be in the viewport currently. Decoration will not be included.
     *  Suitable for mouse click positions.
     */
    QModelIndex indexVisuallyAt(const QPoint& p);

public Q_SLOTS:

    void setSearchTextSettings(const SearchTextSettings& settings);

Q_SIGNALS:

    /// Emitted when the currently selected album changes
    void currentAlbumChanged(Album *currentAlbum);
    /// Emitted when the current selection changes. Use currentChanged unless in multi-selection mode.
    void selectedAlbumsChanged(QList<Album*> selectedAlbums);
    /// Emitted when search text settings have been changed
    void filteringDone(bool atLeastOneMatch);

protected Q_SLOTS:

    // override if implemented behavior is not as intended
    virtual void slotRootAlbumAvailable();

    void slotFilterChanged();
    void slotCurrentChanged();
    void slotSelectionChanged();

protected:

    bool checkExpandedState(const QModelIndex& index);
    void mousePressEvent(QMouseEvent *e);

    void dragEnterEvent(QDragEnterEvent *e);
    void dragMoveEvent(QDragMoveEvent *e);
    void dragLeaveEvent(QDragLeaveEvent * e);
    void dropEvent(QDropEvent *e);

    virtual void middleButtonPressed(Album *a);

    AbstractSpecificAlbumModel *m_albumModel;
    AlbumFilterModel           *m_albumFilterModel;

    bool     m_checkOnMiddleClick;

private:

    bool     m_expandOnSingleClick;
};

class AbstractCountingAlbumTreeView : public AbstractAlbumTreeView
{
    Q_OBJECT

public:

    explicit AbstractCountingAlbumTreeView(AbstractCountingAlbumModel *model, QWidget *parent = 0);

private Q_SLOTS:

    void slotCollapsed(const QModelIndex& index);
    void slotExpanded(const QModelIndex& index);
    void slotSetShowCount();
    void slotRowsInserted(const QModelIndex& parent, int start, int end);
    void updateShowCountState(const QModelIndex& index, bool recurse);
};

class AbstractCheckableAlbumTreeView : public AbstractCountingAlbumTreeView
{

public:

    /// Models of these view _can_ be checkable, they need _not_. You need to enable it on the model.

    explicit AbstractCheckableAlbumTreeView(AbstractCheckableAlbumModel *model, QWidget *parent = 0);

    /// Manage check state through the model directly
    AbstractCheckableAlbumModel *checkableModel() const;

    /// Enable checking on middle mouse button click (default: on)
    void setCheckOnMiddleClick(bool doThat);

protected:

    virtual void middleButtonPressed(Album *a);
};

class AlbumTreeView : public AbstractCheckableAlbumTreeView
{
public:

    AlbumTreeView(QWidget *parent = 0);
    AlbumModel *albumModel() const;
};

class TagTreeView : public AbstractCheckableAlbumTreeView
{
public:

    TagTreeView(QWidget *parent = 0);
    TagModel *albumModel() const;
};

class SearchTreeView : public AbstractAlbumTreeView
{
public:

    SearchTreeView(QWidget *parent = 0);
    SearchModel *albumModel() const;
};

class DateAlbumTreeView : public AbstractCountingAlbumTreeView
{
public:

    DateAlbumTreeView(QWidget *parent = 0);
    DateAlbumModel *albumModel() const;
};



}

#endif

