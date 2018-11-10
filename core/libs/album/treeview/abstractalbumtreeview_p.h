/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-25
 * Description : Tree View for album models
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * Copyright (C) 2014      by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_ABSTRACT_ALBUM_TREE_VIEW_P_H
#define DIGIKAM_ABSTRACT_ALBUM_TREE_VIEW_P_H

#include "abstractalbumtreeview.h"

// Qt includes

#include <QStyledItemDelegate>
#include <QMouseEvent>
#include <QScrollBar>
#include <QTimer>
#include <QDrag>
#include <QMenu>
#include <QIcon>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "albumdragdrop.h"
#include "albummanager.h"
#include "albummodeldragdrophandler.h"
#include "applicationsettings.h"
#include "albumthumbnailloader.h"
#include "contextmenuhelper.h"
#include "fileactionmngr.h"
#include "tagdragdrop.h"
#include "tagmodificationhelper.h"
#include "coredb.h"

namespace Digikam
{

template <class A>
static QList<A*> selectedAlbums(QItemSelectionModel* const selModel,
                                AlbumFilterModel* const filterModel)
{
    const QList<QModelIndex> indexes = selModel->selectedIndexes();
    QList<A*> albums;

    foreach (const QModelIndex& index, indexes)
    {
        albums << static_cast<A*>(filterModel->albumForIndex(index));
    }

    return albums;
}

// -------------------------------------------------------------------------------

struct State
{
    State()
        : selected(false),
          expanded(false),
          currentIndex(false)
    {
    }

    bool selected;
    bool expanded;
    bool currentIndex;
};

// -------------------------------------------------------------------------------

class Q_DECL_HIDDEN AlbumTreeViewDelegate : public QStyledItemDelegate
{
public:

    explicit AlbumTreeViewDelegate(AbstractAlbumTreeView* const treeView = 0)
        : QStyledItemDelegate(treeView),
          m_treeView(treeView),
          m_height(0)
    {
        updateHeight();
    }

    virtual QSize sizeHint(const QStyleOptionViewItem& option,
                           const QModelIndex& index) const
    {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        size.setHeight(qMax(size.height(), m_height));
        return size;
    }

    void updateHeight()
    {
        int h = qMax(AlbumThumbnailLoader::instance()->thumbnailSize() + 2,
                     m_treeView->fontMetrics().height());

        if (h % 2 > 0)
        {
            ++h;
        }

        setHeight(h);
    }

    void setHeight(int height)
    {
        if (m_height == height)
        {
            return;
        }

        m_height = height;

        emit sizeHintChanged(QModelIndex());
    }

protected:

    AbstractAlbumTreeView* m_treeView;
    int                    m_height;
};

// -------------------------------------------------------------------------------

class Q_DECL_HIDDEN AbstractAlbumTreeView::Private
{
public:

    explicit Private()
      : delegate(0),
        expandOnSingleClick(false),
        expandNewCurrent(false),
        selectAlbumOnClick(false),
        selectOnContextMenu(true),
        enableContextMenu(false),
        setInAlbumManager(false),
        resizeColumnsTimer(0),
        configSelectionEntry(QLatin1String("Selection")),
        configExpansionEntry(QLatin1String("Expansion")),
        configCurrentIndexEntry(QLatin1String("CurrentIndex")),
        configSortColumnEntry(QLatin1String("SortColumn")),
        configSortOrderEntry(QLatin1String("SortOrder"))
    {
    }

    AlbumTreeViewDelegate*     delegate;

    bool                       expandOnSingleClick;
    bool                       expandNewCurrent;
    bool                       selectAlbumOnClick;
    bool                       selectOnContextMenu;
    bool                       enableContextMenu;
    bool                       setInAlbumManager;

    QMap<int, Digikam::State>  statesByAlbumId;
    QMap<int, Digikam::State>  searchBackup;

    QTimer*                    resizeColumnsTimer;

    AlbumPointer<Album>        lastSelectedAlbum;

    QList<ContextMenuElement*> contextMenuElements;

    QPixmap                    contextMenuIcon;
    QString                    contextMenuTitle;

    const QString              configSelectionEntry;
    const QString              configExpansionEntry;
    const QString              configCurrentIndexEntry;
    const QString              configSortColumnEntry;
    const QString              configSortOrderEntry;
};

} // namespace Digikam

#endif // DIGIKAM_ABSTRACT_ALBUM_TREE_VIEW_P_H
